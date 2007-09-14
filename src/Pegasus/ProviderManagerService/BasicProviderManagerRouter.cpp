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

#include "BasicProviderManagerRouter.h"

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/ProviderManagerService/ProviderManagerModule.h>
#include <Pegasus/ProviderManager2/ProviderManager.h>

// ProviderManager library names.  Should these be defined elsewhere?
#define LIBRARY_NAME_CMPIPM    "CMPIProviderManager"
#define LIBRARY_NAME_JMPIPM    "JMPIProviderManager"

PEGASUS_NAMESPACE_BEGIN

static const char UNSUPPORTED_INTERFACE_TYPE_KEY [] =
    "ProviderManager.BasicProviderManagerRouter.UNSUPPORTED_INTERFACE_TYPE";
static const char UNSUPPORTED_INTERFACE_TYPE [] =
    "Unsupported InterfaceType \"$0\" in ProviderModule \"$1\".";

class ProviderManagerContainer
{
public:

    ProviderManagerContainer(
        const String& physicalName,
        const String& logicalName,
        const String& interfaceName,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        Boolean subscriptionInitComplete)
    : _manager(0)
    {
#if defined (PEGASUS_OS_VMS)
    String provDir = ConfigManager::getInstance()->
                                    getCurrentValue("providerDir");
    _physicalName = ConfigManager::getHomedPath(provDir) + "/" +
                       FileSystem::buildLibraryFileName(physicalName);
#else
        _physicalName = ConfigManager::getHomedPath(PEGASUS_DEST_LIB_DIR) +
            String("/") + FileSystem::buildLibraryFileName(physicalName);
#endif

        _logicalName = logicalName;
        _interfaceName = interfaceName;

        _module.reset(new ProviderManagerModule(_physicalName));
        Boolean moduleLoaded = _module->load();

        if (moduleLoaded)
        {
            _manager = _module->getProviderManager(_logicalName);
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "ProviderManagerModule load failed.");
        }

        if (_manager == 0)
        {
            PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "Failed to load ProviderManager \"" + _physicalName + "\".");

            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                "ProviderManager.BasicProviderManagerRouter."
                    "PROVIDERMANAGER_LOAD_FAILED",
                "Failed to load the Provider Manager for interface type \"$0\""
                    " from library \"$1\".",
                _interfaceName, _physicalName);

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "ProviderManager.BasicProviderManagerRouter."
                    "PROVIDERMANAGER_LOAD_FAILED",
                "Failed to load the Provider Manager for interface type \"$0\""
                    " from library \"$1\".",
                _interfaceName, _physicalName));
        }

        _manager->setIndicationCallback(indicationCallback);
        _manager->setResponseChunkCallback(responseChunkCallback);

        _manager->setSubscriptionInitComplete (subscriptionInitComplete);
    }

    ProviderManagerContainer(
        const String& interfaceName,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        Boolean subscriptionInitComplete,
        ProviderManager* manager)
        :
        _interfaceName(interfaceName),
        _manager(manager),
        _module(0)
    {
        _manager->setIndicationCallback(indicationCallback);
        _manager->setResponseChunkCallback(responseChunkCallback);
        _manager->setSubscriptionInitComplete(subscriptionInitComplete);
    }

    ~ProviderManagerContainer()
    {
        delete _manager;

        if (_module.get())
            _module->unload();
    }

    ProviderManager* getProviderManager()
    {
        return _manager;
    }

    const String& getInterfaceName() const
    {
        return _interfaceName;
    }


private:

    ProviderManagerContainer();
    ProviderManagerContainer& operator=(const ProviderManagerContainer&);
    ProviderManagerContainer(const ProviderManagerContainer&);

    String _physicalName;
    String _logicalName;
    String _interfaceName;
    ProviderManager* _manager;
    AutoPtr<ProviderManagerModule> _module;
};

PEGASUS_INDICATION_CALLBACK_T
    BasicProviderManagerRouter::_indicationCallback = 0;

PEGASUS_RESPONSE_CHUNK_CALLBACK_T
    BasicProviderManagerRouter::_responseChunkCallback = 0;

ProviderManager*
    (*BasicProviderManagerRouter::_createDefaultProviderManagerCallback)() = 0;

BasicProviderManagerRouter::BasicProviderManagerRouter(
    PEGASUS_INDICATION_CALLBACK_T indicationCallback,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
    ProviderManager* (*createDefaultProviderManagerCallback)())
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::BasicProviderManagerRouter");

    _indicationCallback = indicationCallback;
    _responseChunkCallback = responseChunkCallback;
    _subscriptionInitComplete = false;
    _createDefaultProviderManagerCallback =
        createDefaultProviderManagerCallback;

    PEG_METHOD_EXIT();
}

BasicProviderManagerRouter::~BasicProviderManagerRouter()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::~BasicProviderManagerRouter");
    /* Clean up the provider managers */
    for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
    {
         ProviderManagerContainer* pmc=_providerManagerTable[i];
         delete pmc;
    }
    PEG_METHOD_EXIT();
}

Message* BasicProviderManagerRouter::processMessage(Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::processMessage");

    CIMRequestMessage* request = dynamic_cast<CIMRequestMessage *>(message);
    PEGASUS_ASSERT(request != 0);

    Message* response = 0;
    Boolean remoteNameSpaceRequest=false;

    //
    // Retrieve the ProviderManager routing information
    //

    CIMInstance providerModule;

    if ((dynamic_cast<CIMOperationRequestMessage*>(request) != 0) ||
        (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE) ||
        (request->getType() == CIM_INITIALIZE_PROVIDER_REQUEST_MESSAGE))
    {
        // Provider information is in OperationContext
        ProviderIdContainer pidc = (ProviderIdContainer)
            request->operationContext.get(ProviderIdContainer::NAME);
        providerModule = pidc.getModule();
        remoteNameSpaceRequest=pidc.isRemoteNameSpace();
    }
    else if (dynamic_cast<CIMIndicationRequestMessage*>(request) != 0)
    {
        // Provider information is in CIMIndicationRequestMessage
        CIMIndicationRequestMessage* indReq =
            dynamic_cast<CIMIndicationRequestMessage*>(request);
        ProviderIdContainer pidc =
            indReq->operationContext.get(ProviderIdContainer::NAME);
        providerModule = pidc.getModule();
    }
    else if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
    {
        // Provider information is in CIMEnableModuleRequestMessage
        CIMEnableModuleRequestMessage* emReq =
            dynamic_cast<CIMEnableModuleRequestMessage*>(request);
        providerModule = emReq->providerModule;
    }
    else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
    {
        // Provider information is in CIMDisableModuleRequestMessage
        CIMDisableModuleRequestMessage* dmReq =
            dynamic_cast<CIMDisableModuleRequestMessage*>(request);
        providerModule = dmReq->providerModule;
    }
    else if ((request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE) ||
             (request->getType() ==
              CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE) ||
             (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE))
    {
        // This operation is not provider-specific
    }
    else
    {
        // Error: Unrecognized message type.
        PEGASUS_ASSERT(0);
        CIMResponseMessage* resp = request->buildResponse();
        resp->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "Unknown message type.");
        response = resp;
    }

    //
    // Forward the request to the appropriate ProviderManager(s)
    //

    if ((request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE) ||
        (request->getType() ==
         CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE))
    {
        _subscriptionInitComplete = true;

        // Send CIMStopAllProvidersRequestMessage or
        // CIMSubscriptionInitCompleteRequestMessage to all ProviderManagers
        ReadLock tableLock(_providerManagerTableLock);
        for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
        {
            ProviderManagerContainer* pmc=_providerManagerTable[i];
            Message* resp = pmc->getProviderManager()->processMessage(request);
            delete resp;
        }

        response = request->buildResponse();
    }
    else if (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE)
    {
        // Do not need to forward this request to in-process provider
        // managers
        response = request->buildResponse();
    }
    else
    {
        // Retrieve the provider interface type
        String interfaceType;
        CIMValue itValue = providerModule.getProperty(
            providerModule.findProperty("InterfaceType")).getValue();
        itValue.get(interfaceType);
        // Get ProviderModule name.
        String providerModuleName;
        CIMValue nameValue = providerModule.getProperty(
            providerModule.findProperty("Name")).getValue();
        nameValue.get(providerModuleName); 
        ProviderManager* pm = 0;
        Boolean gotError = false;
        try
        {
            // Look up the appropriate ProviderManager by InterfaceType
            pm = _getProviderManager(interfaceType, providerModuleName);
        }
        catch (const CIMException& e)
        {
            CIMResponseMessage* cimResponse = request->buildResponse();
            cimResponse->cimException = e;
            response = cimResponse;
            gotError = true;
        }

        if (!gotError && remoteNameSpaceRequest && 
            !pm->supportsRemoteNameSpaces())
        {
            CIMResponseMessage* resp = request->buildResponse();
            resp->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                "Remote Namespace operations not supported for interface type "
                    + interfaceType);
            response = resp;
            gotError = true;
        }

        if (!gotError)
        {
            response = pm->processMessage(request);
        }
    }

    // preserve message key
    // set HTTP method in response from request
    // set closeConnect
   ((CIMResponseMessage *)response)->syncAttributes(request);

    PEG_METHOD_EXIT();
    return response;
}

// ATTN: May need to add interfaceVersion parameter to further constrain lookup
ProviderManager* BasicProviderManagerRouter::_getProviderManager(
    const String& interfaceType, const String& providerModuleName)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::_getProviderManager");

    //
    // Search for this InterfaceType in the table of loaded ProviderManagers
    //
    {
        ReadLock tableLock(_providerManagerTableLock);

        ProviderManager* pm = _lookupProviderManager(interfaceType);
        if (pm)
        {
            PEG_METHOD_EXIT();
            return pm;
        }
    }

    //
    // Load the ProviderManager for this InterfaceType and add it to the table
    //
    {
        WriteLock tableLock(_providerManagerTableLock);

        ProviderManager* pm = _lookupProviderManager(interfaceType);
        if (pm)
        {
            PEG_METHOD_EXIT();
            return pm;
        }

        // ATTN: this section is a temporary solution to populate the list of
        // enabled provider managers for a given distribution.  It includes
        // another temporary solution for converting a generic file name into
        // a file name useable by each platform.

        // The DefaultProviderManager is now statically linked rather than
        // dynamically loaded. This code is no longer used but remains for
        // reference purposes.

#if defined(PEGASUS_ENABLE_DEFAULT_PROVIDER_MANAGER)
        if (interfaceType == "C++Default" &&
            _createDefaultProviderManagerCallback)
        {
            pm = (*_createDefaultProviderManagerCallback)();
            ProviderManagerContainer* pmc = new ProviderManagerContainer(
                "C++Default",
                _indicationCallback,
                _responseChunkCallback,
                _subscriptionInitComplete,
                pm);
            _providerManagerTable.append(pmc);
            return pmc->getProviderManager();
        }
#endif

#if defined(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER)
        if (interfaceType == "CMPI")
        {
            ProviderManagerContainer* pmc = new ProviderManagerContainer(
                LIBRARY_NAME_CMPIPM,
                "CMPI",
                "CMPI",
                _indicationCallback,
                _responseChunkCallback,
                _subscriptionInitComplete);
            _providerManagerTable.append(pmc);
            return pmc->getProviderManager();
        }
#endif

#if defined(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER)
        if (  interfaceType == "JMPI"
           || interfaceType == "JMPIExperimental"
           )
        {
            ProviderManagerContainer* pmc = new ProviderManagerContainer(
                LIBRARY_NAME_JMPIPM,
                interfaceType,
                interfaceType,
                _indicationCallback,
                _responseChunkCallback,
                _subscriptionInitComplete);
            _providerManagerTable.append(pmc);
            return pmc->getProviderManager();
        }
#endif
        // END TEMP SECTION
    }

    // Error: ProviderManager not found for the specified interface type
    PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
      "Failed to get ProviderManager for interface type\"" 
                   + interfaceType + "\".");

    Logger::put_l(
        Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
        UNSUPPORTED_INTERFACE_TYPE_KEY,
        UNSUPPORTED_INTERFACE_TYPE, interfaceType, providerModuleName);

    PEG_METHOD_EXIT();

    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
          UNSUPPORTED_INTERFACE_TYPE_KEY,
          UNSUPPORTED_INTERFACE_TYPE, interfaceType, providerModuleName));
}

// NOTE: The caller must lock _providerManagerTableLock before calling this
// method.
ProviderManager* BasicProviderManagerRouter::_lookupProviderManager(
    const String& interfaceType)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::_lookupProviderManager");

    //
    // Search for this InterfaceType in the table of loaded ProviderManagers
    //
    for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
    {
        if (interfaceType == _providerManagerTable[i]->getInterfaceName())
        {
            ProviderManagerContainer* pmc = _providerManagerTable[i];
            PEG_METHOD_EXIT();
            return pmc->getProviderManager();
        }
    }

    // Not found
    PEG_METHOD_EXIT();
    return 0;
}

Boolean BasicProviderManagerRouter::hasActiveProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::hasActiveProviders");

    ReadLock tableLock(_providerManagerTableLock);
    for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
    {
        ProviderManagerContainer* pmc = _providerManagerTable[i];
        if (pmc->getProviderManager()->hasActiveProviders())
        {
            PEG_METHOD_EXIT();
            return true;
        }
    }

    PEG_METHOD_EXIT();
    return false;
}

void BasicProviderManagerRouter::unloadIdleProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::unloadIdleProviders");

    //
    // Save pointers to the ProviderManagerContainers so we don't hold the
    // _providerManagerTableLock while unloading idle providers
    //
    Array<ProviderManagerContainer*> pmcs;
    {
        ReadLock tableLock(_providerManagerTableLock);
        for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
        {
            pmcs.append(_providerManagerTable[i]);
        }
    }

    //
    // Unload idle providers in each of the active ProviderManagers
    // _providerManagerTableLock while unloading idle providers
    //
    for (Uint32 i = 0; i < pmcs.size(); i++)
    {
        pmcs[i]->getProviderManager()->unloadIdleProviders();
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
