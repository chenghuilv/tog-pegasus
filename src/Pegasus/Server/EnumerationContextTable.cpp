//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%////////////////////////////////////////////////////////////////////////////

#include "EnumerationContextTable.h"

#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/General/Guid.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/Condition.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Server/EnumerationContext.h>
#include <Pegasus/Common/StringConversion.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;
/************************************************************************
//
//      TimerClass - List entry for List of all Enumeration Contexts for
//      which the interoperation timer is active. This uses the list class
//      and inserts entries into the list in order of timeout. Note: There
//      should never be more than one entry for any single context
//      since operations on a context are NOT concurrent.
//      NOTE: Incomplete: Consider converting the timer to a list mechanism
//      as cheaper way to execute timeouts.
//
************************************************************************/

// KS_TODO Remove this
//class Timer : public Linkable
//{
//public:
//    Timer(const Uint32 timeout, EnumerationContext& ec
//          : _timeout(timeout), _ec(ec)) { }
//    ~Timer() { }
//    insertTimer()
//    bool isTimedOut():
//
//    // Test for equality of a context name
//    static bool equal(const Person* person, const void* ecName)
//    {
//        return *((String*)client_data) == ec->getContextName();
//    }
//private:
//    Timer(const Timer& x);
//    Timer& operation=(const Timer& x);
//    Uint64 _timeout;
//    EnumerationContext* -ec;
//};
//
//Typedef List<Timer, NullLock> TimerList;
//TimerList timerList;
//
//// if the head of list timer has expired return true and remove it from
//// the list
//bool Timer::isTimedOut()
//{
//    if (timerList.front(_ec->isTimedOut()))
//    {
//
//    }
//    return timerList.front(_ec->isTimedOut());
//
//}
/************************************************************************
//
//      EnumerationContextTable Class Implementation
//
************************************************************************/
// Thread execution function for timerThread(). This thread executes
// regular timeout tests on active contexts and closes them or marks them
// for close if timed out.  This is required for those cases where a
// pull sequence is terminated without either completing or closing the
// sequence.
#define PEGASUS_USE_PULL_TIMEOUT_THREAD
#ifdef PEGASUS_USE_PULL_TIMEOUT_THREAD
ThreadReturnType PEGASUS_THREAD_CDECL operationContextTimerThread(void* parm)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "EnumerationContextTable::operationContextTimerThread");
    Thread *my_handle = (Thread *)parm;
    EnumerationContextTable* et =
        (EnumerationContextTable *)my_handle->get_parm();
    PEGASUS_ASSERT(et->valid());   // KS_TEMP

    Uint32 loopInterval = 2000; // time milliseconds for the loop itself

    // execute loop at regular intervals until there are no more contexts
    // KS_TODO - The choice of existence of contexts as the loop controller
    // means that we will look through many inactive contexts to find any
    // that are both active and timed out (much search for a very unusual
    // condition). This algorithm not optimum.
    while (et->size() != 0)
    {
        // Test to determine if we are ready for next loop.
        while (!et->isTimedOut())
        {
            Threads::sleep(loopInterval);
        }

        et->removeExpiredContexts();
        et->updateNextTimeout();

    }
    // reset the timeout value to indicate that the thread is quiting
    et->setTimerThreadIdle();

    PEG_METHOD_EXIT();
    return ThreadReturnType(0);
}
#endif

EnumerationContextTable::EnumerationContextTable()
    :
    _timeoutInterval(0),
    _nextTimeout(0),
    _enumContextCounter(1),
    _responseCacheMaximumSize(0),
    _cacheHighWaterMark(0),
    _maxOperationTimeout(0)
{
}
/*  Create the Enumeration table and set the values for
    maximum InteroperationTimeOut and maximum size of the
    response Cache where objects are gathered from the
    providers to be distributed as pull client operations
    are processed.
*/
void EnumerationContextTable::setContextDefaultParameters(
    Uint32 maxInteroperationTimeoutValue,
    Uint32 reponseCacheMaximumSize)
{
    _responseCacheMaximumSize = reponseCacheMaximumSize;
    _maxOperationTimeout = maxInteroperationTimeoutValue;
}

/* Remove all contexts and delete them. Only used on system shutdown.
*/
EnumerationContextTable::~EnumerationContextTable()
{

    // remove any existing entries
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "EnumerationContextTable::~EnumerationContextTable");

    removeContextTable();

    PEG_METHOD_EXIT();
}

/*
    Create a new context entry and return it. This includes information
    required to process the pull and close operations for the enumeration
    sequence controlled by this context. The context instance will remain
    active for the life of the enumeration sequence.
*/
EnumerationContext* EnumerationContextTable::createContext(
    const CIMNamespaceName& nameSpace,
    Uint32Arg&  operationTimeoutParam,
    Boolean continueOnError,
    MessageType pullRequestType,
    CIMResponseData::ResponseDataContent contentType)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,"EnumerationContextTable::createContext");

    AutoMutex autoMut(tableLock);
    // set the operation timeout to either the default or current
    // value
    Uint32 operationTimeout =
        (operationTimeoutParam.isNull())?
            _maxOperationTimeout
            :
            operationTimeoutParam.getValue();

    // Create new context, Context name is monolithically increasing counter.
    // The interoperationTimeout is defined by either default or input value.
    EnumerationContext* enumCtxt = new EnumerationContext(nameSpace,
        operationTimeout,
        continueOnError,
        pullRequestType,
        contentType);

    // Set the maximum size for the response Cache from the default
    // value in the table. This is for future where we could adjust the
    // size dynamically for each operation depending on resource utilization.
    enumCtxt->_responseCacheMaximumSize = _responseCacheMaximumSize;

    // Create new context name
    //
    _enumContextCounter++;
    Uint32 size;
    char t[22];
    const char* cxtName = Uint32ToString(t,_enumContextCounter.get(),size);

    // Put the name into the context and return value
    enumCtxt->_enumerationContextName = cxtName;

    // insert new context into the table
    if(!ht.insert(cxtName, enumCtxt))
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "Error Creating Enumeration Context %s. System Failed",
            cxtName ));
        enumCtxt = 0;
        PEGASUS_ASSERT(false);
    }
    // If we do not use the separate thread, test for timeouts each time
    // we create a new context.
#ifndef PEGASUS_USE_PULL_TIMEOUT_THREAD
    if (isTimedOut())
    {
        removeExpiredContexts();
    }
#endif
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "CreateContext ContextId= %s", cxtName));

    PEG_METHOD_EXIT();
    return enumCtxt;
}

void EnumerationContextTable::removeContextTable()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "EnumerationContextTable::removeContextTable");
    Uint32 ctr = 0;
    for (HT::Iterator i = ht.start(); i; i++)
    {
        EnumerationContext* enumeration = i.value();
        //// KS_TODO remove this diagnostic which shows leftover
        //// context entries.  Should be zero.
        cout << "Found at ~EnumerationContextTable "
             << enumeration->_enumerationContextName
             << " started " << (long unsigned int)
            (TimeValue::getCurrentTime().toMilliseconds()
               - enumeration->_startTime)/1000
            << " milliseconds before." << endl;

        PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL4,
            "EnumerationTable Close. Entry Found "
                " name %s, started %llu milliseconds before,",
             (const char *)enumeration->getName().getCString(),
             ((TimeValue::getCurrentTime().toMilliseconds()
               - enumeration->_startTime)/1000) ));

        // KS_TODO _ clean up threads before closing the table

        delete enumeration;
        ht.remove(i.key());
        ctr++;
    }
    if (ctr > 0)
    {
        cout << "EnumerationContextTable shutdown found "
             << ctr << " contexts " << endl;
    }
    PEG_METHOD_EXIT();
}

/*
    Check for valid and in table. Remove from table if in the table
    Delete if the delete = true;
*/
void EnumerationContextTable::removeCxt(
    const String& enumerationContextName,
    Boolean deleteContext)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "EnumerationContextTable::remove");
    AutoMutex autoMut(tableLock);
    EnumerationContext* en = find(enumerationContextName);

    if (en == 0)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,  // KS_TEMP
            "remove Context ERRORERROR en == 0 enName %s",
            (const char *)enumerationContextName.getCString()));

        cout << "EnumTable.remove where en = 0" << endl;
        PEG_METHOD_EXIT();
        return;
    }
    _removeContext(en, deleteContext);
}

// KS_TODO - Clean up fact that we repeat code above and here and have
// 2 parallel functions for deletion (pointer and name)
void EnumerationContextTable::removeContext(EnumerationContext* en)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,"EnumerationContextTable::removeContext");
    PEGASUS_ASSERT(valid());
    AutoMutex autoMut(tableLock);

    PEG_METHOD_EXIT();
    _removeContext(en, true);
}

// Private remove function with no lock protection. The tableLock must
// be set before this function is called to protect the table. This simply
// removes the context from the context table.
Boolean EnumerationContextTable::_removeContext(
    EnumerationContext* en, Boolean deleteContext)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,"EnumerationContextTable::_removeContext");

    PEGASUS_ASSERT(en->valid());            // KS_TEMP
    tableValidate();

    // If it is valid and providers are complete, remove
    // the enumerationContext.  If providers not complete, only
    // completion of provider deliveries can initiate removal of
    // the enumeration context.
    // This function assumes that the sequence is really complete and
    // provider returns complete.  If that is not true, it just generates
    // an error and returns.
    if (en->_clientClosed && en->_providersComplete)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,  // KS_TEMP
            "EnumerationContext Remove. ContextId= %s",
            (const char *)en->getName().getCString() ));

        // test/set the highwater mark for the table
        if (en->_cacheHighWaterMark > _cacheHighWaterMark)
        {
            _cacheHighWaterMark = en->_cacheHighWaterMark;
        }
        // KS_TODO Temporary diagnostic trace of enumerateContext internal info
        en->trace();
        // Remove from EnumerationContextTable.
        ht.remove(en->getName());

        // KS_TODO - Should we clear the cache? Right now, just display
        if (en->responseCacheSize() != 0)
        {
            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,  // KS_TEMP
                "ERROR. Cache != 0 EnumerationContext Remove. ContextId= %s "
                " size = %u",
                (const char *)en->getName().getCString(),
                en->responseCacheSize() ));
        }

        // Delete the enumerationContext object
        if (deleteContext)
        {
            delete en;
        }

        // KS_TODO - Diagnostic Temporary
        tableValidate();

        PEG_METHOD_EXIT();
        return true;
    }
    else
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,  // KS_TEMP TODO
            "_removeContext ERRORERROR %s  _providersComplete=%s"
                " _waitingCacheSizeCondition=%s clientClosed=%s",
            (const char *)en->getName().getCString(),
            boolToString(en->_providersComplete),
            boolToString(en->_clientClosed) ));
        //// KS_TODO remove this.  Test Diagnostic only.
        cout << "remove ignored. "
            << " clientClosed " <<(en->_clientClosed? "true" : "false")
            << " _providersComplete " <<
                 (en->_providersComplete? "true" : "false")
            << endl;
    }
    PEG_METHOD_EXIT();
    return false;
}

Uint32 EnumerationContextTable::size()
{
    AutoMutex autoMut(tableLock);
    return(ht.size());
}

// If context name found, return pointer to that context.  Otherwise
// return 0
EnumerationContext* EnumerationContextTable::find(
    const String& enumerationContextName)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "EnumerationContextTable::find");

    AutoMutex autoMut(tableLock);

    EnumerationContext* en;

    // lookup enumeration. Boolean true if found
    if(ht.lookup(enumerationContextName, en))
    {
        PEGASUS_ASSERT(en != 0);
        PEGASUS_ASSERT(en->valid());
        PEG_METHOD_EXIT();
        return en;
    }
    // Return not found indicator
    PEG_METHOD_EXIT();
    return 0;
}

/** Test all table entries and remove the ones timed out.

*/
void EnumerationContextTable::removeExpiredContexts()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "EnumerationContextTable::removeExpiredContexts");

    // Lock the EnumerationContextTable so no operations can be accepted
    // during this process
    AutoMutex autoMut(tableLock);

    Uint64 currentTime = TimeValue::getCurrentTime().toMicroseconds();

    // Search enumeration table for entries timed out
    for (HT::Iterator i = ht.start(); i; i++)
    {
        EnumerationContext* en = i.value();
        PEGASUS_ASSERT(en->valid());             // diagnostic. KS_TEMP
        // Lock the context at this point to assure that no client
        // operation interacts with possible timeout and removal.
        en->lockContext();
        // test if entry is active (timer not zero)
        if (en->_interOperationTimer != 0)
        {
            if (en->isTimedOut(currentTime))
            {
//              cout << "Entry timed out " << en->getContextName()
//                  << " " << en->_interOperationTimer <<  endl;
                //// en->_interOperationTimer = 0;
                // Force the client closed so nothing more accepted.
                en->setClientClosed();

                // If providers are complete we can remove the context
                if (en->ifProvidersComplete())
                {
                    _removeContext(en, true);
                    PEG_METHOD_EXIT();
                    return;
                }
            }
//          else
//          {
//              cout << "Entry NOT timed out " << en->getContextName()
//                  << " " << en->_interOperationTimer <<  endl;
//          }
        }
        en->unlockContext();
    }
//  cout << "exit removeExpiredContexts" << endl;
    PEG_METHOD_EXIT();
    return;
}

// Validate every entry in the table.
void EnumerationContextTable::tableValidate()
{
    for (HT::Iterator i = ht.start(); i; i++)
    {
        EnumerationContext* en = i.value();
        if (!en->valid())
        {
            trace();
            PEGASUS_ASSERT(en->valid());             // diagnostic. KS_TEMP
        }
    }
}

void EnumerationContextTable::dispatchTimerThread(Uint32 interval)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
                     "EnumerationContextTable::dispatchTimerThread");

    PEGASUS_ASSERT(valid());   // KS_TEMP

    AutoMutex autoMut(tableLock);
    if (timerThreadIdle())
    {
        // convert second timer to milliseconds
        if (interval < _timeoutInterval)
        {
            _timeoutInterval = interval * 1000;
        }
        Uint64 nextTimeout = interval +
            TimeValue::getCurrentTime().toMilliseconds();
        if (nextTimeout < _nextTimeout)
        {
            _nextTimeout = nextTimeout;
        }

#ifdef PEGASUS_USE_PULL_TIMEOUT_THREAD
        // Start a detached thread that executes the timeout tests.
        // This thread runs until the timer is cleared or there are
        // no more contexts.

        Thread thread(operationContextTimerThread, this, true);
        if (thread.run() != PEGASUS_THREAD_OK)
        {
            // KS_TODO add to msg bundle.
            MessageLoaderParms parms(
                "Server.EnumerationContextTale.THREAD_ERROR",
                "Failed to start thread.");

            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                parms);
            PEGASUS_ASSERT(false);
            PEG_METHOD_EXIT();
            return;
        }
#endif
    }
    PEG_METHOD_EXIT();
}


void EnumerationContextTable::trace()
{
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "Context Table Trace. size = %u", ht.size()));

    AutoMutex autoMut(tableLock);
    for (HT::Iterator i = ht.start(); i; i++)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "key [%s]", (const char*)i.key().getCString() ));
        EnumerationContext* enumeration = i.value();
        enumeration->trace();
    }
}
/**
 * validate the magic object for this context
 *
 * @return Boolean True if valid Enumeration table.
 */
Boolean EnumerationContextTable::valid()
{
    return _magic;
}

PEGASUS_NAMESPACE_END
