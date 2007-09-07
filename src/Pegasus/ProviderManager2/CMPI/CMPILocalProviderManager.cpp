//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CMPI_Version.h"

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/Common/MessageLoader.h>

#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProviderModule.h>
#include <Pegasus/ProviderManager2/CMPI/CMPILocalProviderManager.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

#undef IDLE_LIMIT
#define IDLE_LIMIT 50

/* Thread deletion specific */
Semaphore CMPILocalProviderManager::_pollingSem(0);
AtomicInt CMPILocalProviderManager::_stopPolling(0);
Thread *CMPILocalProviderManager::_reaperThread = 0;
List<CMPILocalProviderManager::cleanupThreadRecord,Mutex> 
    CMPILocalProviderManager::_finishedThreadList;
Mutex CMPILocalProviderManager::_reaperMutex;

CMPILocalProviderManager::CMPILocalProviderManager ():
_idle_timeout (IDLE_LIMIT)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::CMPILocalProviderManager()");
    PEG_METHOD_EXIT();
}

CMPILocalProviderManager::~CMPILocalProviderManager ()
{
    Uint32 ccode;
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "MPILocalProviderManager::~CMPILocalProviderManager()");

    _provider_ctrl (UNLOAD_ALL_PROVIDERS, this, &ccode);
    // Since all of the providers are deleted we can delete the 
    //  modules too.
    for (ModuleTable::Iterator j = _modules.start (); j != 0; j++)
    {
        CMPIProviderModule *module = j.value ();
        delete module;
    }

    if (_reaperThread)
    {
        AutoMutex lock(_reaperMutex);
        _stopPolling++;
        _pollingSem.signal();
        // Wait until it finishes itself.
        _reaperThread->join(); 
        delete _reaperThread; 
        _reaperThread = 0;
    }
    PEGASUS_ASSERT(_finishedThreadList.size() == 0);
    PEG_METHOD_EXIT();
}

Sint32 CMPILocalProviderManager::_provider_ctrl (
    CTRL code, 
    void *parm, 
    void *ret)
{

    static Uint32 quantum;
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_provider_ctrl()");

    Sint32 ccode = 0;
    CTRL_STRINGS *parms = reinterpret_cast < CTRL_STRINGS * >(parm);

    switch (code)
    {
        case GET_PROVIDER:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl:GET_PROVIDER()");

                String providerName = *(parms->providerName);
                String moduleFileName = *(parms->fileName);
                String location = *(parms->location);

                CMPIProvider *pr = 0;
                CMPIProvider::OpProviderHolder * ph =
                    reinterpret_cast < CMPIProvider::OpProviderHolder * >(ret);

                pr = _lookupProvider (providerName);

                if (pr->getStatus () != CMPIProvider::INITIALIZED)
                {
                    pr->setLocation (location);
                    _initProvider (pr, moduleFileName);

                    if (pr->getStatus () != CMPIProvider::INITIALIZED)
                    {
                        MessageLoaderParms parms(
                            "ProviderManager.CMPI.CMPILocalProviderManager."
                            "CANNOT_INIT_PROVIDER",
                            "Failed to initialize the provider $0.",
                            pr->getName());
                        PEG_METHOD_EXIT ();
                        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
                    }
                }


                PEG_TRACE_STRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL4,
                    "Returning Provider" + providerName);


                ph->SetProvider (pr);
                ph->GetProvider ().update_idle_timer ();
                break;
            }

        case UNLOAD_PROVIDER:
            {

                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl:\
                    UNLOAD_PROVIDER");
                CMPIProvider *pr = 0;
                pr = _lookupProvider (*(parms->providerName));
                if ((pr->getStatus () == CMPIProvider::INITIALIZED))
                {

                    PEG_TRACE_STRING(
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL4,
                        "Unloading CMPIProvider: " + pr->getName());

                    AutoMutex lock (_providerTableMutex);
                    // The provider table must be locked before unloading. 
                    _providers.remove (pr->_name);
                    _unloadProvider (pr);
                    delete pr;

                }
                else
                {
                    // No need to have a memory leak.
                    _providers.remove(pr->_name);
                    delete pr;
                }
                break;
            }

        case LOOKUP_PROVIDER:
            {

                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl:\
                    LOOKUP_PROVIDER");

                AutoMutex lock (_providerTableMutex);

                if (true == _providers.lookup (*(parms->providerName),
                    *(reinterpret_cast <
                    CMPIProvider * *>(ret))))
                {
                    PEG_TRACE_STRING(
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL4,
                        "Found CMPIProvider in cache: " +
                        *(parms->providerName));

                    (*(reinterpret_cast < CMPIProvider * *>(ret)))->
                        update_idle_timer ();
                }
                else
                {

                    PEG_TRACE_STRING(
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL4,
                        "Could not find  CMPIProvider in cache: " +
                        *(parms->providerName));
                    ccode = -1;
                }

                break;
            }

        case LOOKUP_MODULE:
            {

                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl:LOOKUP_MODULE");

                AutoMutex lock (_providerTableMutex);

                if (false == _modules.lookup (*(parms->fileName),
                    *(reinterpret_cast <
                    CMPIProviderModule * *>(ret))))
                {
                    PEG_TRACE_STRING(
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL4,
                        "Could not find  CMPIProvider Module in cache: "
                        + *(parms->fileName));
                    ccode = -1;
                }

                break;
            }

        case INSERT_PROVIDER:
            {

                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl:\
                    INSERT_PROVIDER");

                AutoMutex lock (_providerTableMutex);

                if (false == _providers.insert (*(parms->providerName),
                    *reinterpret_cast <
                    CMPIProvider * *>(parm)))
                    ccode = -1;
                break;
            }
        case INSERT_MODULE:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl:INSERT_MODULE");
                AutoMutex lock (_providerTableMutex);
                if (false == _modules.insert (*(parms->fileName),
                    *reinterpret_cast <
                    CMPIProviderModule * *>(parm)))
                    ccode = -1;
                break;
            }

        case UNLOAD_ALL_PROVIDERS:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl: \
                    UNLOAD_ALL_PROVIDERS");
                CMPILocalProviderManager *myself =
                    reinterpret_cast < CMPILocalProviderManager * >(parm);
                CMPIProvider *provider = 0;
                // Locked provider mutex.
                AutoMutex lock (_providerTableMutex);

                PEG_TRACE((
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL4,
                    "providers in cache = %d", myself->_providers.size ()));
                ProviderTable::Iterator i = myself->_providers.start ();
                try
                {
                    for (; i != 0; i++)
                    {

                        provider = i.value ();
                        PEGASUS_ASSERT (provider != 0);
                        if (provider->getStatus () == 
                            CMPIProvider::UNINITIALIZED)
                        {
                            // Delete the skeleton.
                            delete provider;
                            continue;
                        }
                        else
                        {
                            _unloadProvider (provider);
                            delete provider;
                        }
                    }
                    // All the providers are removed. Clear the hash-table
                    _providers.clear ();
                }
                catch (...)
                {
                    PEG_TRACE_CSTRING(
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL2,
                        "Unexpected Exception in UNLOAD_ALL_PROVIDERS.");
                }
                break;
            }
        case UNLOAD_IDLE_PROVIDERS:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "CMPILocalProviderManager::_provider_ctrl: \
                    UNLOAD_IDLE_PROVIDERS");
                AutoMutex lock (_providerTableMutex);

                quantum++;
                CMPILocalProviderManager *myself =
                    reinterpret_cast < CMPILocalProviderManager * >(parm);
                CMPIProvider *provider = 0;
                Uint32 numProviders = myself->_providers.size ();

                if (numProviders)
                {
                    // Rather than removing the provider immediately while
                    // iterating through the list we remember all candidates
                    // for unload in a simple array.
                    CMPIProvider **unloadProviderArray =
                        new CMPIProvider *[numProviders];
                    Uint32 upaIndex = 0;

                    try
                    {
                        struct timeval now;
                        Time::gettimeofday (&now);
                        ProviderTable::Iterator i = myself->_providers.start();

                        for (; i != 0; i++)
                        {
                            provider = i.value ();
                            PEGASUS_ASSERT (provider != 0);

                            if (provider->getStatus () == 
                                CMPIProvider::UNINITIALIZED)
                            {
                                continue;
                            }

                            if (provider->_quantum == quantum)
                            {
                                continue;
                            }

                            provider->_quantum = quantum;

                            if (provider->_current_operations.get ())
                            {
                                PEG_TRACE_STRING(
                                    TRC_PROVIDERMANAGER,
                                    Tracer::LEVEL4,
                                    "CMPIProvider has pending operations: "
                                    + provider->getName ());

                                continue;
                            }

                            PEG_TRACE_STRING(
                                TRC_PROVIDERMANAGER,
                                Tracer::LEVEL4,
                                "Checking timeout data for CMPIProvider: "
                                + provider->getName ());
                            struct timeval timeout = { 0, 0};
                            provider->get_idle_timer (&timeout);

                            PEG_TRACE_CSTRING(
                                TRC_PROVIDERMANAGER,
                                Tracer::LEVEL4,
                                provider->unload_ok ()? 
                                " provider->unload_ok() returns: true" : 
                                " provider->unload_ok() returns: false");

                            if (provider->unload_ok () == true &&
                                (now.tv_sec - timeout.tv_sec) >
                                ((Sint32) myself->_idle_timeout))
                            {
                                // Remember this provider to be unloaded
                                unloadProviderArray[upaIndex] = provider;
                                upaIndex++;
                            }
                            //else cout<<"--- NOT unloaded: "+ 
                            // provider->getName()<<endl;
                        }

                        // Now finally we unload all providers that we 
                        // identified as candidates above.
                        for (Uint32 index = 0; index < upaIndex; index++)
                        {
                            CMPIProvider *provider = 
                                unloadProviderArray[index];
                            PEG_TRACE_STRING(
                                TRC_PROVIDERMANAGER,
                                Tracer::LEVEL4,
                                "Now trying to unload CMPIProvider " +
                                provider->getName ());
                            {
                                // lock the provider mutex

                                AutoMutex pr_lock (provider->_statusMutex);

                                if (provider->tryTerminate () == false)
                                {
                                    // provider not unloaded -- we are 
                                    // respecting this!
                                    PEG_TRACE_STRING(
                                        TRC_PROVIDERMANAGER,
                                        Tracer::LEVEL4,
                                        "Provider refused to unload: " +
                                        unloadProviderArray[index]->getName());
                                    continue;
                                }

                                PEGASUS_ASSERT (provider->_module != 0);

                                // unload provider module
                                provider->_module->unloadModule ();
                                Logger::put (Logger::STANDARD_LOG, 
                                    System::CIMSERVER,
                                    Logger::TRACE,
                                    "CMPILocalProviderManager::_provider_crtl"
                                    " -  Unload provider $0",
                                    provider->getName ());

                                // Note: The deleting of the cimom handle is 
                                // being moved after the call to 
                                // unloadModule() based on a previous fix for 
                                // bug 3669 and consistency with other 
                                // provider managers. Do not move it back 
                                // before the call to unloadModule().
                                PEG_TRACE_STRING(
                                    TRC_PROVIDERMANAGER,
                                    Tracer::LEVEL4,
                                    "Destroying CMPIProvider's CIMOM Handle "
                                    + provider->getName());
                                delete provider->_cimom_handle;

                                // set provider status to UNINITIALIZED
                                provider->reset ();
                            }
                        }
                    }
                    catch (...)
                    {
                        PEG_TRACE_CSTRING(
                            TRC_PROVIDERMANAGER,
                            Tracer::LEVEL2,
                            "Unexpected Exception in UNLOAD_IDLE_PROVIDERS.");
                    }
                    delete [] unloadProviderArray;
                }                     // if there are any providers
                break;
            }

        default:
            ccode = -1;
            break;
    }
    PEG_METHOD_EXIT ();
    return(ccode);
}



/*
 * The reaper function polls out the threads from the global list 
 * (_finishedThreadList), joins them deletes them, and removes them from the 
 * CMPIProvider specific list.
*/
ThreadReturnType PEGASUS_THREAD_CDECL CMPILocalProviderManager::_reaper(
    void *parm)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_reaper()");
    Thread *myself = reinterpret_cast<Thread *>(parm);
    do
    {
        _pollingSem.wait();
        // All of the threads are finished working. We just need to reap 'em
        cleanupThreadRecord *rec = 0;

        while (_finishedThreadList.size() >0)
        {
            // Pull of the the threads from the global list.
            rec = _finishedThreadList.remove_front();
            PEG_TRACE_STRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL4,
                String("-- Reaping the thread from ") +
                rec->provider->getName());

            rec->thread->join();

            // Remove the thread for the CMPIProvider.
            rec->provider->threadDelete(rec->thread);

            // Delete the thread.
            delete rec->thread;
            delete rec;
        }
    }
    while (_stopPolling.get() == 0);
    myself->exit_self( (ThreadReturnType) 0);
    PEG_METHOD_EXIT();
    return(0);
}
/*
 // Cleanup the thread and upon deletion of it, call the CMPIProvider' 
 // "threadDeleted". to not, all the CMPIProvider '
 // Note that this function is called from the thread that finished with
 // running the providers function, and returns immediately while scheduling 
 // the a cleanup procedure. If you want to wait until the thread is truly 
 // deleted, call 'waitUntilThreadsDone' - but DO NOT do it in the the thread
 // that the Thread owns - you will wait forever.
 //
 // @argument t Thread that is not NULL and finished with running the provider
 // function.
 // @argument p CMPIProvider in which the 't' Thread was running.
 */
void CMPILocalProviderManager::cleanupThread(Thread *t, CMPIProvider *p)
{
    PEG_METHOD_ENTER(
         TRC_PROVIDERMANAGER,
         "CMPILocalProviderManager::cleanupThread()");

    PEGASUS_ASSERT( t != 0 && p != 0 );

    PEGASUS_ASSERT ( p->isThreadOwner(t) );
    // The mutex guards against a race condition for _reaperThread creation.
    AutoMutex lock(_reaperMutex);

    // Put the Thread and the CMPIProvider on the global list.
    cleanupThreadRecord *record = new cleanupThreadRecord(t, p);
    _finishedThreadList.insert_back(record);

    if (_reaperThread == 0)
    {
        _reaperThread = new Thread(_reaper, NULL, false);
        ThreadStatus rtn = PEGASUS_THREAD_OK;
        while ((rtn = _reaperThread->run()) != PEGASUS_THREAD_OK)
        {
            if (rtn == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
                Threads::yield();
            else
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "Could not allocate thread to take care of deleting "
                    "user threads. ");
                delete _reaperThread; _reaperThread = 0;
                PEG_METHOD_EXIT();
                return;
            }       
        }
    }
    // Wake up the reaper.
    _pollingSem.signal();
    PEG_METHOD_EXIT();

}

CMPIProvider::OpProviderHolder CMPILocalProviderManager::
    getRemoteProvider (const String & location, const String & providerName)
{
    CMPIProvider::OpProviderHolder ph;
    CTRL_STRINGS
        strings;
    Sint32
        ccode;
    const String
        proxy ("CMPIRProxyProvider");

    String
        rproviderName ("R");
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProvidertManager::getRemoteProvider()");

    rproviderName.append (providerName);

    strings.fileName = &proxy;
    strings.providerName = &rproviderName;
    strings.location = &location;

    try
    {
        ccode = _provider_ctrl (GET_PROVIDER, &strings, &ph);
    }
    catch (const Exception & e)
    {
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            String("--- loading proxy:  ") + e.getMessage () );

        PEG_METHOD_EXIT ();
        throw;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "--- Unexpected exception in loading proxy provider: ---");
        PEG_METHOD_EXIT ();
        throw;
    }

    PEG_METHOD_EXIT ();
    return ph;
}

CMPIProvider::OpProviderHolder CMPILocalProviderManager::
    getProvider (const String & fileName, const String & providerName)
{
    CMPIProvider::OpProviderHolder ph;
    CTRL_STRINGS strings;
    Sint32 ccode;

    String lproviderName ("L");

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "ProviderManager::getProvider()");

    if (fileName.size() == 0)
    {
        throw Exception(MessageLoaderParms(
            "ProviderManager.CMPI.CMPILocalProviderManager.CANNOT_FIND_LIBRARY",
            "For provider $0 the library name was empty. Check provider "
            "registered location.",
            providerName));

    }
    lproviderName.append (providerName);
    strings.fileName = &fileName;
    strings.providerName = &lproviderName;
    strings.location = &String::EMPTY;

    try
    {
        ccode = _provider_ctrl (GET_PROVIDER, &strings, &ph);
    }
    catch (const Exception & e)
    {
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            String("--- loading local provider:  ") + e.getMessage () );
        PEG_METHOD_EXIT ();
        throw;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "--- Unexpected exception in loading local provider ---");
        PEG_METHOD_EXIT ();
        throw;
    }


    PEG_METHOD_EXIT ();
    return(ph);

}

void CMPILocalProviderManager::unloadProvider(
    const String & fileName,
    const String & providerName)
{
    CTRL_STRINGS strings;
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "ProviderManager::unloadProvider()");

    String lproviderName ("L");
    String rproviderName ("R");
    lproviderName.append (providerName);
    rproviderName.append (providerName);

    strings.fileName = &fileName;
    strings.providerName = &lproviderName;
    strings.location = &String::EMPTY;
    _provider_ctrl (UNLOAD_PROVIDER, &strings, (void *) 0);

    strings.providerName = &rproviderName;

    _provider_ctrl (UNLOAD_PROVIDER, &strings, (void *) 0);

    PEG_METHOD_EXIT ();
}

void CMPILocalProviderManager::shutdownAllProviders ()
{
    _provider_ctrl (UNLOAD_ALL_PROVIDERS, (void *) this, (void *) 0);
}


Boolean CMPILocalProviderManager::hasActiveProviders ()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProviderManager::hasActiveProviders()");

    try
    {
        AutoMutex lock (_providerTableMutex);
        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "providers in _providers table = %d", _providers.size ()));

        // Iterate through the _providers table looking for an active provider
        for (ProviderTable::Iterator i = _providers.start (); i != 0; i++)
        {
            if (i.value ()->getStatus () == CMPIProvider::INITIALIZED)
            {
                PEG_METHOD_EXIT ();
                return true;
            }
        }
    }
    catch (...)
    {
        // Unexpected exception; do not assume that no providers are loaded
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Unexpected Exception in hasActiveProviders.");
        PEG_METHOD_EXIT ();
        return true;
    }

    // No active providers were found in the _providers table
    PEG_METHOD_EXIT ();
    return false;
}

void CMPILocalProviderManager::unloadIdleProviders ()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProviderManager::unloadIdleProviders()");

    try
    {
        _provider_ctrl (UNLOAD_IDLE_PROVIDERS, this, (void *) 0);
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Caught unexpected exception from UNLOAD_IDLE_PROVIDERS.");
    }

    PEG_METHOD_EXIT ();
}

Array <CMPIProvider *>CMPILocalProviderManager::
    getIndicationProvidersToEnable()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::getIndicationProvidersToEnable()");

    Array < CMPIProvider * >enableProviders;

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "Number of providers in _providers table = %d",
        _providers.size ()));

    try
    {
        AutoMutex
            lock (_providerTableMutex);

        //
        // Iterate through the _providers table
        //
        for (ProviderTable::Iterator i = _providers.start (); i != 0; i++)
        {
            //
            //  Enable any indication provider with current subscriptions
            //
            CMPIProvider *
                provider = i.value ();
            if (provider->testSubscriptions ())
            {
                enableProviders.append (provider);
            }
        }

    }
    catch (const CIMException & e)
    {
        PEG_TRACE_STRING(
            TRC_DISCARDED_DATA,
            Tracer::LEVEL2,
            "CIMException: " + e.getMessage ());
    }
    catch (const Exception & e)
    {
        PEG_TRACE_STRING(
            TRC_DISCARDED_DATA,
            Tracer::LEVEL2,
            "Exception: " + e.getMessage ());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_DISCARDED_DATA,
            Tracer::LEVEL2,
            "Unexpected error in getIndicationProvidersToEnable");
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "Number of indication providers to enable = %d",
        enableProviders.size ()));

    PEG_METHOD_EXIT ();
    return enableProviders;
}

CMPIProvider *CMPILocalProviderManager::_initProvider(
    CMPIProvider * provider,
    const String & moduleFileName)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_initProvider()");

    CMPIProviderModule *module = 0;
    ProviderVector base;

    {
        // lock the providerTable mutex
        AutoMutex lock (_providerTableMutex);

        // lookup provider module
        module = _lookupModule (moduleFileName);
    }                             // unlock the providerTable mutex

    Boolean moduleLoaded = false;
    Boolean deleteProvider = false;
    String exceptionMsg = moduleFileName;
    {
        // lock the provider status mutex
        AutoMutex lock (provider->_statusMutex);

        if (provider->_status == CMPIProvider::INITIALIZED)
        {
            PEG_METHOD_EXIT();
            // Initialization is already complete
            return provider;
        }

        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Loading/Linking Provider Module " + moduleFileName);

        // load the provider
        try
        {
            base = module->load (provider->_name);
            moduleLoaded = true;
        }
        catch (const Exception &e)
        {
            exceptionMsg = e.getMessage();
            PEG_TRACE_STRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Exception caught Loading/Linking Provider Module " +
                moduleFileName+ " error is: "+exceptionMsg);
            deleteProvider =true;
        }
        catch (...)
        {
            PEG_TRACE_STRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Unknown exception caught Loading/Linking Provider Module " +
                moduleFileName);
            exceptionMsg = moduleFileName;
            deleteProvider = true;
        }

        if (!deleteProvider)
        {
            // initialize the provider
            PEG_TRACE_STRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Initializing Provider " + provider->getName());

            CIMOMHandle *cimomHandle = new CIMOMHandle ();
            provider->set (module, base, cimomHandle);
            provider->_quantum = 0;

            try
            {
                provider->initialize (*(provider->_cimom_handle));
            }
            catch (const Exception &e)
            {
                PEG_TRACE_STRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "Problem initializing: " + e.getMessage());
                deleteProvider = true;
                exceptionMsg = e.getMessage();
            }
            catch (...)
            {
                PEG_TRACE_STRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL2,
                    "Unknown problem initializing " + provider->getName());
                deleteProvider = true;
                exceptionMsg = provider->getName();
            }
        }                             // unlock the provider status mutex
    }
    /* We wait until this moment to delete the provider b/c we need
       be outside the scope for the AutoMutex for the provider. */
    if (deleteProvider)
    {
        // Note: The deleting of the cimom handle is being
        // moved after the call to unloadModule() based on
        // a previous fix for bug 3669 and consistency with
        // other provider managers. Do not move it back before
        // the call to unloadModule().

        // unload provider module
        if (moduleLoaded)
        {
            module->unloadModule();
        }

        // delete the cimom handle
        delete provider->_cimom_handle;
        // set provider status to UNINITIALIZED
        provider->reset ();

        AutoMutex lock (_providerTableMutex);
        _providers.remove (provider->_name);
        delete provider;

        PEG_METHOD_EXIT ();
        throw Exception(exceptionMsg);
    }

    PEG_METHOD_EXIT ();
    return(provider);
}


void CMPILocalProviderManager::_unloadProvider (CMPIProvider * provider)
{
    //
    // NOTE:  It is the caller's responsibility to make sure that
    // the ProviderTable mutex is locked before calling this method.
    //
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_unloadProvider()");

    PEG_TRACE_STRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "Unloading Provider " + provider->getName());

    if (provider->_current_operations.get ())
    {
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Provider cannot be unloaded due to pending operations: "
            + provider->getName());
    }
    else
    {
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Terminating Provider " + provider->getName());


        // lock the provider mutex
        AutoMutex pr_lock (provider->_statusMutex);

        try
        {
            provider->terminate ();
        }
        catch (...)
        {
            PEG_TRACE_STRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Error occured terminating CMPI provider " +
                provider->getName());
        }

        // delete the cimom handle
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Destroying CMPIProvider's CIMOM Handle " +
            provider->getName());

        delete provider->_cimom_handle;
        PEGASUS_ASSERT (provider->_module != 0);

        // unload provider module
        provider->_module->unloadModule ();
        Logger::put (Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPILocalProviderManager::_provider_crtl -  Unload provider $0",
            provider->getName ());

        // set provider status to UNINITIALIZED
        provider->reset ();

        // Do not delete the provider. The function calling this function
        // takes care of that.
    }

    PEG_METHOD_EXIT ();
}

CMPIProvider * CMPILocalProviderManager::_lookupProvider(
    const String & providerName)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_lookupProvider()");
    // lock the providerTable mutex
    AutoMutex lock (_providerTableMutex);
    // look up provider in cache
    CMPIProvider *pr = 0;
    if (true == _providers.lookup (providerName, pr))
    {
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Found Provider " + providerName +
            " in CMPI Provider Manager Cache");
    }
    else
    {
        // create provider
        pr = new CMPIProvider (providerName, 0, 0);
        // insert provider in provider table
        _providers.insert (providerName, pr);

        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Created provider " + pr->getName ());
    }

    PEG_METHOD_EXIT ();
    return(pr);
}


CMPIProviderModule * CMPILocalProviderManager::_lookupModule(
    const String & moduleFileName)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_lookupModule()");

    // look up provider module in cache
    CMPIProviderModule *module = 0;

    if (true == _modules.lookup (moduleFileName, module))
    {
        // found provider module in cache
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Found Provider Module" + moduleFileName +
            " in Provider Manager Cache");

    }
    else
    {
        // provider module not found in cache, create provider module
        PEG_TRACE_STRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Creating CMPI Provider Module " + moduleFileName);

        module = new CMPIProviderModule (moduleFileName);

        // insert provider module in module table
        _modules.insert (moduleFileName, module);
    }

    PEG_METHOD_EXIT ();
    return(module);
}

PEGASUS_NAMESPACE_END
    
