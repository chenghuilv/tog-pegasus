//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CMPI_Version.h"

#include "CMPIProviderManager.h"

#include "CMPI_Object.h"
#include "CMPI_ContextArgs.h"
#include "CMPI_Instance.h"
#include "CMPI_ObjectPath.h"
#include "CMPI_Result.h"
#include "CMPI_SelectExp.h"

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h> //l10n

#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Server/CIMServer.h>

#include <Pegasus/ProviderManager2/ProviderType.h>
#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include <Pegasus/ProviderManager2/CMPI/CMPILocalProviderManager.h>
#include <Pegasus/ProviderManager2/ProviderManagerService.h>
//#include <Pegasus/ProviderManager2/Default/OperationResponseHandler.h>

#include <Pegasus/Server/ProviderRegistrationManager/ProviderRegistrationManager.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

static CMPILocalProviderManager providerManager;

int _cmpi_trace=0;

#define DDD(x) if (_cmpi_trace) x;

CMPIProviderManager::IndProvTab    CMPIProviderManager::provTab;
CMPIProviderManager::IndSelectTab  CMPIProviderManager::selxTab;
CMPIProviderManager::ProvRegistrar CMPIProviderManager::provReg;

class CMPIPropertyList {
   char **props;
   int pCount;
  public: 
   CMPIPropertyList(CIMPropertyList &propertyList) {
      if (!propertyList.isNull()) {
        Array<CIMName> p=propertyList.getPropertyNameArray();
        pCount=p.size();
        props=(char**)malloc((1+pCount)*sizeof(char*));
        for (int i=0; i<pCount; i++) {
           props[i]=strdup(p[i].getString().getCString());
        }
        props[pCount]=NULL;
      }
      else props=NULL;
   }
   ~CMPIPropertyList() {
      if (props) {
         for (int i=0; i<pCount; i++)
            free(props[i]);
         free(props);   
      }
   }
   char **getList() {
      return props;
   }
};

CMPIProviderManager::CMPIProviderManager(Mode m)
{
   mode=m;
   if (getenv("CMPI_TRACE")) _cmpi_trace=1;
   else _cmpi_trace=0;
   _repository = ProviderManagerService::_repository;
}

CMPIProviderManager::~CMPIProviderManager(void)
{
}

Boolean CMPIProviderManager::insertProvider(const ProviderName & name,
            const String &ns, const String &cn)
{
    String key(ns+String("::")+cn+String("::")+CIMValue(name.getCapabilitiesMask()).toString());
    return provReg.insert(key,name);
}


Message * CMPIProviderManager::processMessage(Message * request)
{
      PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::processMessage()");

    Message * response = 0;
    
    // pass the request message to a handler method based on message type
    switch(request->getType())
    {
    case CIM_GET_INSTANCE_REQUEST_MESSAGE:
        response = handleGetInstanceRequest(request);

        break;
    case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        response = handleEnumerateInstancesRequest(request);

        break;
    case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
        response = handleEnumerateInstanceNamesRequest(request);

        break;
    case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
        response = handleCreateInstanceRequest(request);

        break;
    case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
        response = handleModifyInstanceRequest(request);

        break;
    case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
        response = handleDeleteInstanceRequest(request);

        break;
    case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        response = handleExecQueryRequest(request);

        break;
    case CIM_ASSOCIATORS_REQUEST_MESSAGE:
        response = handleAssociatorsRequest(request);

        break;
    case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
        response = handleAssociatorNamesRequest(request);

        break;
    case CIM_REFERENCES_REQUEST_MESSAGE:
        response = handleReferencesRequest(request);

        break;
    case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
        response = handleReferenceNamesRequest(request);

        break;
    case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        response = handleInvokeMethodRequest(request);

        break;
    case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleCreateSubscriptionRequest(request);

        break;
/*    case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleModifySubscriptionRequest(request);

        break;
*/  case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleDeleteSubscriptionRequest(request);

        break;
    case CIM_ENABLE_INDICATIONS_REQUEST_MESSAGE:
        response = handleEnableIndicationsRequest(request);

        break;
    case CIM_DISABLE_INDICATIONS_REQUEST_MESSAGE:
        response = handleDisableIndicationsRequest(request);

        break;
/*    case CIM_CONSUME_INDICATION_REQUEST_MESSAGE:
        response = handleConsumeIndicationRequest(request);
        break;
*/
    case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
        response = handleDisableModuleRequest(request);

        break;
    case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
        response = handleEnableModuleRequest(request);

        break; 
    case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
        response = handleStopAllProvidersRequest(request);

        break;
    default:
        response = handleUnsupportedRequest(request);

        break;
    }
 
    PEG_METHOD_EXIT();

    return(response);
}

void CMPIProviderManager::unload_idle_providers(void) 
{
     providerManager.unload_idle_providers();  
}


#define CHARS(cstring) (char*)(strlen(cstring)?(const char*)cstring:NULL)


#define HandlerIntroBase(type,type1,message,request,response,handler,respType) \
    CIM##type##RequestMessage * request = \
        dynamic_cast<CIM##type##RequestMessage *>(const_cast<Message *>(message)); \
    PEGASUS_ASSERT(request != 0); \
    CIM##type##ResponseMessage * response = \
        new CIM##type##ResponseMessage( \
        request->messageId, \
        CIMException(), \
        request->queueIds.copyAndPop() \
        respType \
    PEGASUS_ASSERT(response != 0); \
    response->setKey(request->getKey()); \
    response->setHttpMethod(request->getHttpMethod()); \
    type1##ResponseHandler handler(request, response); 

#define VOIDINTRO );
#define NOVOIDINTRO(type) ,type);
#define METHODINTRO ,CIMValue(), Array<CIMParamValue>(), request->methodName );


#define HandlerIntroVoid(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler,VOIDINTRO)

#define HandlerIntroMethod(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler,METHODINTRO)

#define HandlerIntroInd(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler,VOIDINTRO)

#define HandlerIntro(type,message,request,response,handler,respType) \
     HandlerIntroBase(type,type,message,request,response,handler,NOVOIDINTRO(respType))
     
#define HandlerCatch(handler) \
    catch(CIMException & e)  \
    { PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: " + e.getMessage()); \
        handler.setStatus(e.getCode(), e.getContentLanguages(), e.getMessage()); \
    } \
    catch(Exception & e) \
    { PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: " + e.getMessage()); \
        handler.setStatus(CIM_ERR_FAILED, e.getContentLanguages(), e.getMessage()); \
    } \
    catch(...) \
    { PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: Unknown"); \
        handler.setStatus(CIM_ERR_FAILED, "Unknown error."); \
    }



Message * CMPIProviderManager::handleGetInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleGetInstanceRequest");

    HandlerIntro(GetInstance,message,request,response,handler,CIMInstance());

    try {
	Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CmpiProviderManager::handleGetInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph = providerManager.getProvider(name.getPhysicalName(),
	                                name.getLogicalName());

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.getInstance: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::getInstance"<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->getInstance
        (pr.miVector.instMI,&eCtx,&eRes,&eRef,props.getList());

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnumerateInstancesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleEnumerateInstanceRequest");

    HandlerIntro(EnumerateInstances,message,request,response,
                 handler,Array<CIMInstance>());
    try {  
      Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleEnumerateInstancesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(),
               String::EMPTY);

	// convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMPropertyList propertyList(request->propertyList);

        // forward request
	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enumerateInstances: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::enumerateInstances"<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIPropertyList props(propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->enumInstances
        (pr.miVector.instMI,&eCtx,&eRes,&eRef,props.getList()); 

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);

        STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnumerateInstanceNamesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleEnumerateInstanceNamesRequest");

    HandlerIntro(EnumerateInstanceNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleEnumerateInstanceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());
       
       // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

       // build an internal provider name from the request arguments
        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

	CMPIProvider & pr=ph.GetProvider();

	PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enumerateInstanceNames: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::enumerateInstanceNames"<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->enumInstanceNames(pr.miVector.instMI,&eCtx,&eRes,&eRef);

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleCreateInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleCreateInstanceRequest");

    HandlerIntro(CreateInstance,message,request,response,
                 handler,CIMObjectPath());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleCreateInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->newInstance.getPath().getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->newInstance.getPath().getClassName(),
            request->newInstance.getPath().getKeyBindings());

	ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(),
	           String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.createInstance: " +
            ph.GetProvider().getName());

        DDD(cerr<<"--- CMPIProviderManager::createInstances"<<endl);
	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_InstanceOnStack eInst(request->newInstance);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->createInstance
	   (pr.miVector.instMI,&eCtx,&eRes,&eRef,&eInst);

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleModifyInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleModifyInstanceRequest");

    HandlerIntroVoid(ModifyInstance,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleModifyInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->modifiedInstance.getPath().getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->modifiedInstance.getPath ().getClassName(),
            request->modifiedInstance.getPath ().getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.modifyInstance: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::modifyInstance"<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_InstanceOnStack eInst(request->modifiedInstance);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->setInstance
        (pr.miVector.instMI,&eCtx,&eRes,&eRef,&eInst,props.getList());

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDeleteInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleDeleteInstanceRequest");

    HandlerIntroVoid(DeleteInstance,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleDeleteInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.deleteInstance: " + pr.getName());

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->deleteInstance
	   (pr.miVector.instMI,&eCtx,&eRes,&eRef);

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleExecQueryRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleExecQueryRequest");

    HandlerIntro(ExecQuery,message,request,response,
                 handler,Array<CIMObject>());

    try {  
      Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::ExecQueryRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

        ProviderName name(
            objectPath,
            ProviderType::QUERY);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(),
               String::EMPTY);

	// convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.execQuery: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::execQuery"<<endl);

        const char **props=NULL;

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString queryLan=request->queryLanguage.getCString();
        const CString query=request->query.getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->execQuery
	   (pr.miVector.instMI,&eCtx,&eRes,&eRef,CHARS(queryLan),CHARS(query));

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);

        STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleAssociatorsRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleAssociatorsRequest");

    HandlerIntro(Associators,message,request,response,
                 handler,Array<CIMObject>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleAssociatorsRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath assocPath(
            System::getHostName(),
            request->nameSpace,
            request->assocClass.getString());

         ProviderName name(
            assocPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

       // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.associators: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::associators"<<" role: >"<<request->role<<"< aCls "<<
	   request->assocClass<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
	CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString aClass=request->assocClass.getString().getCString();
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        const CString resRole=request->resultRole.getCString();

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->associators(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,CHARS(aClass),
                         CHARS(rClass),CHARS(rRole),CHARS(resRole),props.getList());

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleAssociatorNamesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleAssociatorNamesRequest");

    HandlerIntro(AssociatorNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleAssociatorNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath assocPath(
            System::getHostName(),
            request->nameSpace,
            request->assocClass.getString());
      
      ProviderName name(
            assocPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.associatorNames: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::associatorNames"<<" role: >"<<request->role<<"< aCls "<<
	   request->assocClass<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
	CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString aClass=request->assocClass.getString().getCString();
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        const CString resRole=request->resultRole.getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->associatorNames(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,CHARS(aClass),
                         CHARS(rClass),CHARS(rRole),CHARS(resRole));

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleReferencesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleReferencesRequest");

    HandlerIntro(References,message,request,response,
                 handler,Array<CIMObject>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleReferencesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath resultPath(
            System::getHostName(),
            request->nameSpace,
            request->resultClass.getString());

        ProviderName name(
            resultPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.references: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::references"<<" role: >"<<request->role<<"< aCls "<<
	   request->resultClass<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
	CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->references(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,
                         CHARS(rClass),CHARS(rRole),props.getList());

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleReferenceNamesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleReferenceNamesRequest");

    HandlerIntro(ReferenceNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleReferenceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath resultPath(
            System::getHostName(),
            request->nameSpace,
            request->resultClass.getString());

        ProviderName name(
            resultPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

  	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.referenceNames: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::referenceNames"<<" role: >"<<request->role<<"< aCls "<<
	   request->resultClass<<endl);

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
	CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->referenceNames(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,
                         CHARS(rClass),CHARS(rRole));

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleInvokeMethodRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleInvokeMethodRequest");

    HandlerIntroMethod(InvokeMethod,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleInvokeMethodRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

       ProviderName name(
            objectPath,
            ProviderType::METHOD,
            request->methodName);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMObjectPath instanceReference(request->instanceName);

        // ATTN: propagate namespace
        instanceReference.setNameSpace(request->nameSpace);

        // forward request
 	CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.invokeMethod: " + pr.getName());

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        CMPI_ArgsOnStack eArgsIn(request->inParameters);
        Array<CIMParamValue> outArgs;
        CMPI_ArgsOnStack eArgsOut(outArgs);
        CString mName=request->methodName.getString().getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.methMI->ft->invokeMethod(
           pr.miVector.methMI,&eCtx,&eRes,&eRef,CHARS(mName),&eArgsIn,&eArgsOut);

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);

       for (int i=0,s=outArgs.size(); i<s; i++)
           handler.deliverParamValue(outArgs[i]);
       handler.complete();
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response); 
}

int LocateIndicationProviderNames(const CIMInstance& pInstance, const CIMInstance& pmInstance,
                                  String& providerName, String& location)
{
    Uint32 pos = pInstance.findProperty(CIMName ("Name"));
    pInstance.getProperty(pos).getValue().get(providerName);

    pos = pmInstance.findProperty(CIMName ("Location"));
    pmInstance.getProperty(pos).getValue().get(location);
    return 0;
}

String CMPIProviderManager::getFilter(CIMInstance &subscription)
{
   try {
   CIMValue filterValue = subscription.getProperty (subscription.findProperty
        ("Filter")).getValue ();
   CIMObjectPath filterReference;
   filterValue.get(filterReference);
   CIMNamespaceName ns("root/PG_InterOp");

   CIMInstance filter=_repository->getInstance(ns,filterReference);

   CIMValue queryValue = filter.getProperty (filter.findProperty
        ("Query")).getValue ();
   String query;
   queryValue.get(query);
   return query;
   }
   catch (CIMException &e) {
      cout<<"??? CMPIProviderManager::getFilter"<<e.getCode()<<" "<<e.getMessage()<<" ???"<<endl;
      abort();
  }
   catch (...) {
      cout<<"??? What Happend ???"<<endl;
      abort();
   }
   return String::EMPTY;
}

Message * CMPIProviderManager::handleCreateSubscriptionRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleCreateSubscriptionRequest");

    HandlerIntroInd(CreateSubscription,message,request,response,
                 handler);
    try {
        const CIMObjectPath &x=request->subscriptionInstance.getPath();

	String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleCreateSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
            System::getHostName(),
            request->nameSpace.getString(),
            providerName);

        String fileName = _resolvePhysicalName(providerLocation);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);

        indProvRecord *prec=NULL;
	provTab.lookup(providerName,prec);
	if (prec) prec->count++;
        else {
	   prec=new indProvRecord();
	   provTab.insert(providerName,prec);
	}

        indSelectRecord *srec=new indSelectRecord();
        const CIMObjectPath &sPath=request->subscriptionInstance.getPath();
	selxTab.insert(sPath.toString(),srec);

        // convert arguments
        OperationContext *context=new OperationContext();

        context->insert(IdentityContainer(request->userName));
        context->insert(SubscriptionInstanceContainer
            (request->subscriptionInstance));
        context->insert(SubscriptionFilterConditionContainer
            (request->condition, request->queryLanguage));

        context->insert(SubscriptionLanguageListContainer
            (request->acceptLanguages));
        context->insert(AcceptLanguageListContainer(request->acceptLanguages));
        context->insert(ContentLanguageListContainer(request->contentLanguages));

        CIMObjectPath subscriptionName = request->subscriptionInstance.getPath();

  	CMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(*context);
        CMPI_SelectExp *eSelx=new CMPI_SelectExp(*context,
	   getFilter(request->subscriptionInstance),
	   request->queryLanguage);
	srec->eSelx=eSelx;
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.createSubscriptionRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::createSubscriptionRequest"<<endl);

        for(Uint32 i = 0, n = request->classNames.size(); i < n; i++) {
            CIMObjectPath className(
                System::getHostName(),
                request->nameSpace,
                request->classNames[i]);
            eSelx->classNames.append(className);
        }
        CMPI_ObjectPathOnStack eRef(eSelx->classNames[0]);

        CIMPropertyList propertyList = request->propertyList;
	if (!propertyList.isNull()) {
           Array<CIMName> p=propertyList.getPropertyNameArray();
           int pCount=p.size();
           eSelx->props=(const char**)malloc((1+pCount)*sizeof(char*));
           for (int i=0; i<pCount; i++) {
              eSelx->props[i]=strdup(p[i].getString().getCString());
	   }
           eSelx->props[pCount]=NULL;
        }

        Uint16 repeatNotificationPolicy = request->repeatNotificationPolicy;

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.indMI->ft->activateFilter(
           pr.miVector.indMI,&eCtx,NULL,eSelx,
           CHARS(eSelx->classNames[0].getClassName().getString().getCString()),
	   &eRef,false);

       STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDeleteSubscriptionRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleDeleteSubscriptionRequest");

    HandlerIntroInd(DeleteSubscription,message,request,response,
                 handler);
    try {
	String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleDeleteSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
            System::getHostName(),
            request->nameSpace.getString(),
            providerName);

        String fileName = _resolvePhysicalName(providerLocation);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);


        indProvRecord *prec=NULL;
	provTab.lookup(providerName,prec);
	if (--prec->count<=0) {
	   provTab.remove(providerName);
	   prec=NULL;
	}

        indSelectRecord *srec=NULL;
        const CIMObjectPath &sPath=request->subscriptionInstance.getPath();
	String sPathString=sPath.toString();
	selxTab.lookup(sPathString,srec);

        CMPI_SelectExp *eSelx=srec->eSelx;
        CMPI_ObjectPathOnStack eRef(eSelx->classNames[0]);
	selxTab.remove(sPathString);

	// convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(SubscriptionInstanceContainer
            (request->subscriptionInstance));

        context.insert(SubscriptionLanguageListContainer
            (request->acceptLanguages));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMObjectPath subscriptionName = request->subscriptionInstance.getPath();

  	CMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.deleteSubscriptionRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::deleteSubscriptionRequest"<<endl);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;
        rc=pr.miVector.indMI->ft->deActivateFilter(
           pr.miVector.indMI,&eCtx,NULL,eSelx,
           CHARS(eSelx->classNames[0].getClassName().getString().getCString()),
	   &eRef,prec==NULL);

       delete eSelx;

       STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc,
	       rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnableIndicationsRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager:: handleEnableIndicationsRequest");

    HandlerIntroInd(EnableIndications,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        indProvRecord *provRec;
        if (provTab.lookup(providerName,provRec)) {
           provRec->enabled=true;
           provRec->handler=new EnableIndicationsResponseHandler(request, response,
              request->provider, ProviderManagerService::providerManagerService);
        }

        String fileName = _resolvePhysicalName(providerLocation);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

  	CMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.EnableIndicationRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::enableIndicationRequest"<<endl);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        pr.miVector.indMI->ft->enableIndications(
           pr.miVector.indMI);

       STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDisableIndicationsRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager:: handleDisableIndicationsRequest");

    HandlerIntroInd(DisableIndications,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        indProvRecord *provRec;
        if (provTab.lookup(providerName,provRec)) {
           provRec->enabled=false;
           if (provRec->handler) delete provRec->handler;
	   provRec->handler=NULL;
        }

        String fileName = _resolvePhysicalName(providerLocation);

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

  	CMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.DisableIndicationRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::disableIndicationRequest"<<endl);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        pr.miVector.indMI->ft->disableIndications(
           pr.miVector.indMI);

       STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}
//
// Provider module status
//
static const Uint16 _MODULE_OK       = 2;
static const Uint16 _MODULE_STOPPING = 9;
static const Uint16 _MODULE_STOPPED  = 10;

Message * CMPIProviderManager::handleDisableModuleRequest(const Message * message)
{
    // HACK
    ProviderRegistrationManager * _providerRegistrationManager = GetProviderRegistrationManager();

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleDisableModuleRequest");

    CIMDisableModuleRequestMessage * request =
        dynamic_cast<CIMDisableModuleRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    // get provider module name
    String moduleName;
    CIMInstance mInstance = request->providerModule;
    Uint32 pos = mInstance.findProperty(CIMName ("Name"));

    if(pos != PEG_NOT_FOUND)
    {
        mInstance.getProperty(pos).getValue().get(moduleName);
    }

    Boolean disableProviderOnly = request->disableProviderOnly;

    //
    // get operational status
    //
    Array<Uint16> operationalStatus;

    if(!disableProviderOnly)
    {
        Uint32 pos2 = mInstance.findProperty(CIMName ("OperationalStatus"));

        if(pos2 != PEG_NOT_FOUND)
        {
            //
            //  ATTN-CAKG-P2-20020821: Check for null status?
            //
            mInstance.getProperty(pos2).getValue().get(operationalStatus);
        }

        //
        // update module status from OK to Stopping
        //
        for(Uint32 i=0, n = operationalStatus.size(); i < n; i++)
        {
            if(operationalStatus[i] == _MODULE_OK)
            {
                operationalStatus.remove(i);
            }
        }
        operationalStatus.append(_MODULE_STOPPING);

        if(_providerRegistrationManager->setProviderModuleStatus
            (moduleName, operationalStatus) == false)
        {
            //l10n
            //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "set module status failed.");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "ProviderManager.CMPIProviderManager.SET_MODULE_STATUS_FAILED",
                "set module status failed."));
        }
    }

    // Unload providers
    Array<CIMInstance> _pInstances = request->providers;
    String physicalName=_resolvePhysicalName(request->providerModule.getProperty(
	      request->providerModule.findProperty("Location")).getValue().toString());

    for(Uint32 i = 0, n = _pInstances.size(); i < n; i++)
    {
        providerManager.unloadProvider(_pInstances[i].getProperty(
	                                  request->providerModule.findProperty
                                          ("Name")).getValue ().toString (),
                                       physicalName);
    }

    if(!disableProviderOnly)
    {
        // update module status from Stopping to Stopped
        for(Uint32 i=0, n = operationalStatus.size(); i < n; i++)
        {
            if(operationalStatus[i] == _MODULE_STOPPING)
            {
                operationalStatus.remove(i);
            }
        }

        operationalStatus.append(_MODULE_STOPPED);

        if(_providerRegistrationManager->setProviderModuleStatus
            (moduleName, operationalStatus) == false)
        {
            //l10n
            //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            //"set module status failed.");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "ProviderManager.CMPIProviderManager.SET_MODULE_STATUS_FAILED",
                "set module status failed."));
        }
    }

    CIMDisableModuleResponseMessage * response =
        new CIMDisableModuleResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop(),
        operationalStatus);

    PEGASUS_ASSERT(response != 0);

    // preserve message key
    response->setKey(request->getKey());

    //
    //  Set HTTP method in response from request
    //
    response->setHttpMethod (request->getHttpMethod ());

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnableModuleRequest(const Message * message)
{
    // HACK
    ProviderRegistrationManager * _providerRegistrationManager = GetProviderRegistrationManager();

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleEnableModuleRequest");

    CIMEnableModuleRequestMessage * request =
        dynamic_cast<CIMEnableModuleRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    //
    // get module status
    //
    CIMInstance mInstance = request->providerModule;
    Array<Uint16> operationalStatus;
    Uint32 pos = mInstance.findProperty(CIMName ("OperationalStatus"));

    if(pos != PEG_NOT_FOUND)
    {
        //
        //  ATTN-CAKG-P2-20020821: Check for null status?
        //
        mInstance.getProperty(pos).getValue().get(operationalStatus);
    }

    // update module status from Stopped to OK
    for(Uint32 i=0, n = operationalStatus.size(); i < n; i++)
    {
        if(operationalStatus[i] == _MODULE_STOPPED)
        {
            operationalStatus.remove(i);
        }
    }

    operationalStatus.append(_MODULE_OK);

    //
    // get module name
    //
    String moduleName;

    Uint32 pos2 = mInstance.findProperty(CIMName ("Name"));

    if(pos2 != PEG_NOT_FOUND)
    {
        mInstance.getProperty(pos2).getValue().get(moduleName);
    }

    if(_providerRegistrationManager->setProviderModuleStatus
        (moduleName, operationalStatus) == false)
    {
        //l10n
        //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "set module status failed.");
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "ProviderManager.CMPIProviderManager.SET_MODULE_STATUS_FAILED",
            "set module status failed."));
    }

    CIMEnableModuleResponseMessage * response =
        new CIMEnableModuleResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop(),
        operationalStatus);

    PEGASUS_ASSERT(response != 0);

    // preserve message key
    response->setKey(request->getKey());

    //  Set HTTP method in response from request
    response->setHttpMethod (request->getHttpMethod ());

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleStopAllProvidersRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleStopAllProvidersRequest");

    CIMStopAllProvidersRequestMessage * request =
        dynamic_cast<CIMStopAllProvidersRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    CIMStopAllProvidersResponseMessage * response =
        new CIMStopAllProvidersResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop());

    PEGASUS_ASSERT(response != 0);

    // preserve message key
    response->setKey(request->getKey());

    //  Set HTTP method in response from request
    response->setHttpMethod (request->getHttpMethod ());

    // tell the provider manager to shutdown all the providers
    providerManager.shutdownAllProviders();

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleUnsupportedRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleUnsupportedRequest");

    PEG_METHOD_EXIT();

    // a null response implies unsupported or unknown operation
    return(0);
}

ProviderName CMPIProviderManager::_resolveProviderName(const ProviderName & providerName)
{
    ProviderName temp = findProvider(providerName);

    String physicalName=_resolvePhysicalName(temp.getPhysicalName());

    temp.setPhysicalName(physicalName);

    return(temp);
}

PEGASUS_NAMESPACE_END
