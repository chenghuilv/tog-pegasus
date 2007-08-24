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
// NOCHKSRC

#include "JMPIProviderManager.h"

#include "JMPIImpl.h"

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/ArrayInternal.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProviderModule.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

int JMPIProviderManager::trace=0;

#ifdef PEGASUS_DEBUG
#define DDD(x) if (JMPIProviderManager::trace) x;
#else
#define DDD(x)
#endif

// request->localOnly is replaced with JMPI_LOCALONLY for getInstance () and enumerateInstances ()
#define JMPI_LOCALONLY false

/* Fix for 4092 */
// request->includeQualifiers is replaced with JMPI_INCLUDE_QUALIFIERS for getInstance (),
//    setInstance (), enumerateInstances (), associators (), and references ()
#define JMPI_INCLUDE_QUALIFIERS false

#include "Convert.h"

void JMPIProviderManager::debugPrintMethodPointers (JNIEnv *env, jclass jc)
{
   // cd ${PEGAUSE_HOME}/src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/tests/JMPI_TestPropertyTypes
   // javap -s -p JMPI_TestPropertyTypes
   static const char *methodNames[][3] = {
      // CIMProvider
      //   cimom-2003-11-24/org/snia/wbem/provider/CIMProvider.java
      //   src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/CIMProvider.java
      {"snia 2.0","initialize","(Lorg/pegasus/jmpi/CIMOMHandle;)V"},
      {"snia 2.0","cleanup","()V"},
      // InstanceProvider
      //   cimom-2003-11-24/org/snia/wbem/provider/InstanceProvider.java
      //   src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/CIMInstanceProvider.java
      {"snia 2.0","enumInstances","(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;Z)Ljava/util/Vector;"},
      {"pegasus 2.4","enumInstances","(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ[Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMInstance;"},
      /* Begin Fix for 4189 */
      {"pegasus 2.5","enumerateInstances","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      /* End Fix for 4189 */
      {"snia 2.0","enumInstances","(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;"},
      {"pegasus 2.4","enumerateInstanceNames","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMObjectPath;"},
      {"pegasus 2.5","enumerateInstanceNames","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;"},
      {"snia 2.0","getInstance","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Z)Lorg/pegasus/jmpi/CIMInstance;"},
      {"pegasus 2.4","getInstance","(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ[Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)Lorg/pegasus/jmpi/CIMInstance;"},
      /* Begin Fix for 4238 */
      {"pegasus 2.5","getInstance","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)Lorg/pegasus/jmpi/CIMInstance;"},
      /* End Fix for 4238 */
      {"snia 2.0","createInstance","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)Lorg/pegasus/jmpi/CIMObjectPath;"},
      {"snia 2.0","setInstance","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)V"},
      {"pegasus 2.4","setInstance","(Lorg/pegasus/jmpi/CIMObjectPath;Z[Ljava/lang/String)V"},
      {"snia 2.0","deleteInstance","(Lorg/pegasus/jmpi/CIMObjectPath;)V"},
      {"snia 2.0","execQuery","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ILorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;"},
      {"pegasus 2.4","execQuery","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMInstance;"},
      {"pegasus 2.5","execQuery","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;"},
      // MethodProvider
      //   cimom-2003-11-24/org/snia/wbem/provider/MethodProvider.java
      //   src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/CIMMethodProvider.java
      {"snia 2.0","invokeMethod","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/util/Vector;Ljava/util/Vector;)Lorg/pegasus/jmpi/CIMValue;"},
      {"pegasus 2.4","invokeMethod","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;[Lorg/pegasus/jmpi/CIMArgument;[Lorg/pegasus/jmpi/CIMArgument;)Lorg/pegasus/jmpi/CIMValue;"},
      // PropertyProvider
      //   cimom-2003-11-24/org/snia/wbem/provider/PropertyProvider.java
      //   src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/PropertyProvider.java
      {"snia 2.0","getPropertyValue","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;)Lorg/pegasus/jmpi/CIMValue;"},
      {"snia 2.0","setPropertyValue","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Lorg/pegasus/jmpi/CIMValue;)V"},
      // AssociatorProvider
      //   cimom-2003-11-24/org/snia/wbem/provider20/AssociatorProvider.java
      //   src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/AssociatorProvider.java
      {"snia 2.0","associators","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4","associators","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"snia 2.0","associatorNames","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4","associatorNames","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;"},
      {"snia 2.0","references","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4","references","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"snia 2.0","referenceNames","(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4","referenceNames","(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)Ljava/util/Vector;"},
      // CIMProviderRouter
      //   cimom-2003-11-24/org/snia/wbem/provider20/CIMProviderRouter.java
      // EventProvider
      //   cimom-2003-11-24/org/snia/wbem/provider20/EventProvider.java
      //   src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/EventProvider.java
      {"snia 2.0","activateFilter","(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;Lorg/pegasus/jmpi/CIMObjectPath;Z)V"},
      {"snia 2.0","deActivateFilter","(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;Lorg/pegasus/jmpi/CIMObjectPath;Z)V"},
      // IndicationHandler
      //   cimom-2003-11-24/org/snia/wbem/provider20/IndicationHandler.java
      // ProviderAdapter
      //   cimom-2003-11-24/org/snia/wbem/provider20/ProviderAdapter.java
      // JMPI_TestPropertyTypes
      {"JMPI_TestPropertyTypes","findObjectPath","(Lorg/pegasus/jmpi/CIMObjectPath;)I"},
      {"JMPI_TestPropertyTypes","testPropertyTypesValue","(Lorg/pegasus/jmpi/CIMInstance;)V"}
   };

   if (!env)
   {
      DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::debugPrintMethodPointers: env is NULL!"<<PEGASUS_STD(endl));
      return;
   }
   if (!jc)
   {
      DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::debugPrintMethodPointers: jc is NULL!"<<PEGASUS_STD(endl));
      return;
   }

   for (int i = 0; i < (int)(sizeof (methodNames)/sizeof (methodNames[0])); i++)
   {
      jmethodID id = env->GetMethodID(jc,methodNames[i][1], methodNames[i][2]);
      DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::debugPrintMethodPointers: "<<methodNames[i][0]<<", "<<methodNames[i][1]<<", id = "<<PEGASUS_STD(hex)<<(long)id<<PEGASUS_STD(dec)<<PEGASUS_STD(endl));
      env->ExceptionClear();
   }

   env->ExceptionClear();
}

void
debugIntrospectJavaObject (JNIEnv *env, jobject jInst)
{
   jclass       jInstClass             = env->GetObjectClass(jInst);
   jclass       jInstSuperClass        = env->GetSuperclass(jInstClass);
   jmethodID    jmidGetDeclaredMethods = env->GetMethodID(jInstClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");

   if (!jmidGetDeclaredMethods)
   {
      env->ExceptionClear();
      jmidGetDeclaredMethods = env->GetMethodID(jInstSuperClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
   }

   if (jmidGetDeclaredMethods)
   {
      jobjectArray jarrayDeclaredMethods  = (jobjectArray)env->CallObjectMethod(jInstClass,
                                                                                jmidGetDeclaredMethods);

      if (jarrayDeclaredMethods)
      {
         for (int i = 0, iLen = env->GetArrayLength (jarrayDeclaredMethods); i < iLen; i++)
         {
            JMPIjvm::checkException(env);

            jobject jDeclaredMethod      = env->GetObjectArrayElement (jarrayDeclaredMethods, i);
            jclass  jDeclaredMethodClass = env->GetObjectClass (jDeclaredMethod);

            JMPIjvm::checkException(env);

            jmethodID   jmidToString  = env->GetMethodID (jDeclaredMethodClass, "toString", "()Ljava/lang/String;");
            jstring     jstringResult = (jstring)env->CallObjectMethod (jDeclaredMethod, jmidToString);
            const char *pszResult     = env->GetStringUTFChars(jstringResult, 0);

            PEGASUS_STD(cout)<<"--- JMPIProviderManager::debugIntrospectJavaObject: "<<pszResult<<PEGASUS_STD(endl);

            env->ReleaseStringUTFChars (jstringResult, pszResult);
         }
      }
   }


   env->ExceptionClear();
}

void
debugDumpJavaObject (JNIEnv *env, jobject jInst)
{
   jclass      jInstClass      = env->GetObjectClass(jInst);
   jclass      jInstSuperClass = env->GetSuperclass(jInstClass);
   jmethodID   jmidToString1   = env->GetMethodID(jInstClass,      "toString", "()Ljava/lang/String;");
   jmethodID   jmidToString2   = env->GetMethodID(jInstSuperClass, "toString", "()Ljava/lang/String;");
   if (!jmidToString1 || !jmidToString2)
   {
      env->ExceptionClear();
      return;
   }
   jstring     jstringResult1  = (jstring)env->CallObjectMethod(jInstClass,      jmidToString1);
   jstring     jstringResult2  = (jstring)env->CallObjectMethod(jInstSuperClass, jmidToString2);
   jstring     jstringResult3  = (jstring)env->CallObjectMethod(jInst,           jmidToString1);
   const char *pszResult1      = env->GetStringUTFChars(jstringResult1, 0);
   const char *pszResult2      = env->GetStringUTFChars(jstringResult2, 0);
   const char *pszResult3      = env->GetStringUTFChars(jstringResult3, 0);

   jmethodID jmidCInst = env->GetMethodID(env->GetObjectClass(jInst),JMPIjvm::jv.instanceMethodNames[22].methodName, JMPIjvm::jv.instanceMethodNames[22].signature);

   PEGASUS_STD(cout)<<"--- JMPIProviderManager::debugIntrospectJavaObject: jInstClass = "<<jInstClass<<", jInstSuperClass = "<<jInstSuperClass<<", jClassShouldBe = "<<JMPIjvm::jv.classRefs[18]<<", jmidCInst = "<<jmidCInst<<PEGASUS_STD(endl);
   PEGASUS_STD(cout)<<"pszResult1 = "<<pszResult1<<PEGASUS_STD(endl);
   PEGASUS_STD(cout)<<"pszResult2 = "<<pszResult2<<PEGASUS_STD(endl);
   PEGASUS_STD(cout)<<"pszResult3 = "<<pszResult3<<PEGASUS_STD(endl);

   env->ReleaseStringUTFChars (jstringResult1, pszResult1);
   env->ReleaseStringUTFChars (jstringResult2, pszResult2);
   env->ReleaseStringUTFChars (jstringResult3, pszResult3);

   env->ExceptionClear();
}

bool JMPIProviderManager::getInterfaceType (ProviderIdContainer pidc,
                                            String&             interfaceType,
                                            String&             interfaceVersion)
{
///CIMInstance ciProvider       = pidc.getProvider ();
   CIMInstance ciProviderModule = pidc.getModule ();
   Uint32      idx;
   bool        fRet             = true;

///PEGASUS_STD(cout)<<"--- JMPIProviderManager::getInterfaceType: ciProvider       = "<<ciProvider.getPath ().toString ()<<PEGASUS_STD(endl);
///PEGASUS_STD(cout)<<"--- JMPIProviderManager::getInterfaceType: ciProviderModule = "<<ciProviderModule.getPath ().toString ()<<PEGASUS_STD(endl);

   idx = ciProviderModule.findProperty ("InterfaceType");

   if (idx != PEG_NOT_FOUND)
   {
      CIMValue itValue;

      itValue = ciProviderModule.getProperty (idx).getValue ();

      itValue.get (interfaceType);

      DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::getInterfaceType: interfaceType = "<<interfaceType<<PEGASUS_STD(endl));
   }
   else
   {
      fRet = false;
   }

   idx = ciProviderModule.findProperty ("InterfaceVersion");

   if (idx != PEG_NOT_FOUND)
   {
      CIMValue itValue;

      itValue = ciProviderModule.getProperty (idx).getValue ();

      itValue.get (interfaceVersion);

      DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::getInterfaceType: interfaceVersion = "<<interfaceVersion<<PEGASUS_STD(endl));
   }
   else
   {
      fRet = false;
   }

   return fRet;
}

bool JMPIProviderManager::interfaceIsUsed (JNIEnv  *env,
                                           jobject  jObject,
                                           String   searchInterfaceName)
{
   jobjectArray jInterfaces       = 0;
   jsize        jInterfacesLength = 0;
   bool         fFound            = false;

   if (!jObject)
   {
      return false;
   }

   jInterfaces = (jobjectArray)env->CallObjectMethod (env->GetObjectClass (jObject),
                                                      JMPIjvm::jv.ClassGetInterfaces);

   if (!jInterfaces)
   {
      return false;
   }

   jInterfacesLength = env->GetArrayLength (jInterfaces);

   for (jsize i = 0; !fFound && i < jInterfacesLength; i++)
   {
      jobject jInterface = env->GetObjectArrayElement (jInterfaces, i);

      if (jInterface)
      {
         jstring jInterfaceName = (jstring)env->CallObjectMethod (jInterface,
                                                                  JMPIjvm::jv.ClassGetName);

         if (jInterfaceName)
         {
            const char *pszInterfaceName = env->GetStringUTFChars (jInterfaceName,
                                                                   0);
            String      interfaceName    = pszInterfaceName;

            if (String::equal (interfaceName, searchInterfaceName))
            {
               fFound = true;
            }

            env->ReleaseStringUTFChars (jInterfaceName, pszInterfaceName);
         }
      }
   }

   return fFound;
}

JMPIProviderManager::IndProvTab    JMPIProviderManager::provTab;
Mutex                              JMPIProviderManager::mutexProvTab;
JMPIProviderManager::IndSelectTab  JMPIProviderManager::selxTab;
Mutex                              JMPIProviderManager::mutexSelxTab;
JMPIProviderManager::ProvRegistrar JMPIProviderManager::provReg;
Mutex                              JMPIProviderManager::mutexProvReg;

JMPIProviderManager::JMPIProviderManager()
{
   _subscriptionInitComplete = false;

#ifdef PEGASUS_DEBUG
   if (getenv("PEGASUS_JMPI_TRACE"))
      JMPIProviderManager::trace = 1;
   else
      JMPIProviderManager::trace = 0;
#else
   JMPIProviderManager::trace = 0;
#endif
}

JMPIProviderManager::~JMPIProviderManager(void)
{
}

Boolean JMPIProviderManager::insertProvider(const ProviderName & name,
            const String &ns, const String &cn)
{
    String key(ns + String("::") + cn);

    DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::insertProvider: "<<key<<PEGASUS_STD(endl));

    Boolean ret = false;

    {
       AutoMutex lock (mutexProvReg);

       ret = provReg.insert(key,name);
    }

    return ret;
}

Message * JMPIProviderManager::processMessage(Message * request) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::processMessage()");

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

    case CIM_GET_PROPERTY_REQUEST_MESSAGE:
        response = handleGetPropertyRequest(request);
        break;

    case CIM_SET_PROPERTY_REQUEST_MESSAGE:
        response = handleSetPropertyRequest(request);
        break;

    case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        response = handleInvokeMethodRequest(request);
        break;

    case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleCreateSubscriptionRequest(request);
        break;

/*  case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleModifySubscriptionRequest(request);
        break;
*/
    case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleDeleteSubscriptionRequest(request);
        break;

/*  case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
        response = handleExportIndicationRequest(request);
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

    case CIM_INITIALIZE_PROVIDER_REQUEST_MESSAGE:
        response = handleInitializeProviderRequest(request);
        break;

    case CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE:
        response = handleSubscriptionInitCompleteRequest (request);
        break;

    default:
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                   "*** Unsupported Request %d",
                   request->getType()
                  ));
        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::processMessage: Unsupported request "<<request->getType ()<<PEGASUS_STD(endl));

        response = handleUnsupportedRequest(request);
        break;
    }

    PEG_METHOD_EXIT();

    return(response);
}

Boolean JMPIProviderManager::hasActiveProviders()
{
     return providerManager.hasActiveProviders();
}

void JMPIProviderManager::unloadIdleProviders()
{
     providerManager.unloadIdleProviders();
}

#define STRDUPA(s,o) \
   if (s) { \
      o=(const char*)alloca(strlen(s)); \
      strcpy((char*)(o),(s)); \
   } \
   else o=NULL;

#define CHARS(cstring) (char*)(strlen(cstring)?(const char*)cstring:NULL)


#define HandlerIntroBase(type,type1,message,request,response,handler) \
    CIM##type##RequestMessage * request = \
        dynamic_cast<CIM##type##RequestMessage *>(const_cast<Message *>(message)); \
    PEGASUS_ASSERT(request != 0); \
    CIM##type##ResponseMessage * response = \
        dynamic_cast<CIM##type##ResponseMessage*>(request->buildResponse()); \
    PEGASUS_ASSERT(response != 0); \
    type1##ResponseHandler handler(request, response, _responseChunkCallback);

#define HandlerIntroInd(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler)

#define HandlerIntroInit(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler)

#define HandlerIntro(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler)

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
    { PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: Unknown"); \
        handler.setStatus(CIM_ERR_FAILED, "Unknown error."); \
    }

static jobjectArray getList(JvmVector *jv, JNIEnv *env, CIMPropertyList &list)
{
    Uint32 s=list.size();
    jobjectArray pl=NULL;
    if (s) {
       jstring initial=env->NewString(NULL,0);
       pl=(jobjectArray)env->NewObjectArray(s,jv->StringClassRef,initial);
       for (Uint32 i=0; i<s; i++) {
           env->SetObjectArrayElement
              (pl,i,env->NewStringUTF(list[i].getString().getCString()));
       }
    }
    return pl;
}

Message * JMPIProviderManager::handleGetInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleGetInstanceRequest");

    HandlerIntro(GetInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2, // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleGetInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->instanceName.getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->instanceName.getClassName(),
                                                       request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName());
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.getInstance: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: Calling provider getInstance: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

////////DDD(debugPrintMethodPointers (env, (jclass)pr.jProviderClass));

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public org.pegasus.jmpi.CIMInstance getInstance (org.pegasus.jmpi.CIMObjectPath cop,
           //                                                  org.pegasus.jmpi.CIMClass      cimClass,
           //                                                  boolean                        localOnly)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "getInstance",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Z)Lorg/pegasus/jmpi/CIMInstance;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMInstance getInstance (org.pegasus.jmpi.CIMObjectPath op,
               //                                                  boolean                        localOnly,
               //                                                  boolean                        includeQualifiers,
               //                                                  boolean                        includeClassOrigin,
               //                                                  java.lang.String[]             propertyList,
               //                                                  org.pegasus.jmpi.CIMClass      cc)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "getInstance",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ[Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: found METHOD_CIMINSTANCEPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           /* Fix for 4238 */
           // public org.pegasus.jmpi.CIMInstance getInstance (org.pegasus.jmpi.OperationContext oc
           //                                                  org.pegasus.jmpi.CIMObjectPath    cop,
           //                                                  org.pegasus.jmpi.CIMClass         cimClass,
           //                                                  boolean                           includeQualifiers,
           //                                                  boolean                           includeClassOrigin,
           //                                                  String                            propertyList[])
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "getInstance",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)Lorg/pegasus/jmpi/CIMInstance;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }
           /* Fix for 4238 */
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong   jopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->instanceName.getClassName()<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->instanceName.getClassName(),
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->instanceName.getClassName()<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobject jciRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                   id,
                                                   jop,
                                                   JMPI_LOCALONLY,
                                                   JMPI_INCLUDE_QUALIFIERS,
                                                   request->includeClassOrigin,
                                                   jPropertyList,
                                                   jcimClass);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jciRet) {
               jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
               CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

               handler.deliver(*ciRet);
            }
            handler.complete();
            break;
        }

        /* Fix for 4238 */
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->instanceName.getClassName()<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->instanceName.getClassName(),
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->instanceName.getClassName()<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobject      jciRet        = env->CallObjectMethod((jobject)pr.jProvider,
                                                               id,
                                                               joc,
                                                               jop,
                                                               jcimClass,
                                                               JMPI_INCLUDE_QUALIFIERS,
                                                               request->includeClassOrigin,
                                                               jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            if (jciRet) {
               jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
               CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

               handler.deliver(*ciRet);
            }
            handler.complete();
            break;
        }
        /* Fix for 4238 */

        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->instanceName.getClassName()<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->instanceName.getClassName(),
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->instanceName.getClassName()<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            // Modified for Bugzilla# 3679
            jobject jciRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                   id,
                                                   jop,
                                                   jcimClass,
                                                   JMPI_LOCALONLY);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jciRet) {
               jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
               CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

               handler.deliver(*ciRet);
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetInstanceRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnumerateInstancesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleEnumerateInstanceRequest");

    HandlerIntro(EnumerateInstances,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_CIMINSTANCEPROVIDER2,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
      Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleEnumerateInstancesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->className.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->className);

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.enumerateInstances: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Calling provider enumerateInstances: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract java.util.Vector enumInstances (org.pegasus.jmpi.CIMObjectPath cop,
           //                                                 boolean                        deep,
           //                                                 org.pegasus.jmpi.CIMClass      cimClass,
           //                                                 boolean                        localOnly)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "enumInstances",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;Z)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMInstance[] enumerateInstances (org.pegasus.jmpi.CIMObjectPath cop,
               //                                                           boolean                        localOnly,
               //                                                           boolean                        includeQualifiers,
               //                                                           boolean                        includeClassOrigin,
               //                                                           java.lang.String[]             propertyList,
               //                                                           org.pegasus.jmpi.CIMClass      cimClass)
               //         throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "enumerateInstances",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ[Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: found METHOD_CIMINSTANCEPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           /* Fix for 4189 */
           // public abstract java.util.Vector enumerateInstances (org.pegasus.jmpi.OperationContext oc
           //                                                      org.pegasus.jmpi.CIMObjectPath    cop,
           //                                                      org.pegasus.jmpi.CIMClass         cimClass,
           //                                                      boolean                           includeQualifiers,
           //                                                      boolean                           includeClassOrigin,
           //                                                      java.lang.String[]                propertyList)
           //         throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "enumerateInstances",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }
           /* Fix for 4189 */

           if (id == NULL)
           {
               env->ExceptionClear();

               // public abstract org.pegasus.jmpi.CIMInstance[] enumerateInstances (org.pegasus.jmpi.OperationContext oc
               //                                                                    org.pegasus.jmpi.CIMObjectPath    cop,
               //                                                                    org.pegasus.jmpi.CIMClass         cimClass,
               //                                                                    boolean                           includeQualifiers,
               //                                                                    boolean                           includeClassOrigin,
               //                                                                    java.lang.String[]                propertyList)
               //         throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "enumerateInstances",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: found METHOD_CIMINSTANCEPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jccRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr           = (jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                             id,
                                                                             jcop,
                                                                             JMPI_LOCALONLY,
                                                                             JMPI_INCLUDE_QUALIFIERS,
                                                                             request->includeClassOrigin,
                                                                             jPropertyList,
                                                                             jcc);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    /* Fix for 4237 */
                    CIMClass cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op  = ciRet->getPath();
                    CIMObjectPath        iop = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jccRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr           = (jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                             id,
                                                                             joc,
                                                                             jcop,
                                                                             jcc,
                                                                             JMPI_INCLUDE_QUALIFIERS,
                                                                             request->includeClassOrigin,
                                                                             jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    /* Fix for 4237 */
                    CIMClass cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op  = ciRet->getPath();
                    CIMObjectPath        iop = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        /* Fix for 4189 */
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass (context,
                                                 request->nameSpace,
                                                 request->className,
                                                 false,
                                                 true,
                                                 true,
                                                 CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jccRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobject      jVec          = env->CallObjectMethod((jobject)pr.jProvider,
                                                               id,
                                                               joc,
                                                               jcop,
                                                               jcc,
                                                               JMPI_INCLUDE_QUALIFIERS,
                                                               request->includeClassOrigin,
                                                               jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    /* Fix for 4237 */
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op  = ciRet->getPath();
                    CIMObjectPath        iop = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());

                    ciRet->setPath(iop);
                    /* Fix for 4237*/

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }
        /* Fix for 4189 */

        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jccRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            // Modified for Bugzilla# 3679
            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 jcop,
                                                 request->deepInheritance,
                                                 jcc,
                                                 JMPI_LOCALONLY);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    /* Fix for 4237 */
                    CIMClass cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op  = ciRet->getPath();
                    CIMObjectPath        iop = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());

                    ciRet->setPath(iop);
                    /* Fix for 4237*/

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstancesRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnumerateInstanceNamesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleEnumerateInstanceNamesRequest");

    HandlerIntro(EnumerateInstanceNames,message,request,response, handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_CIMINSTANCEPROVIDER2,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleEnumerateInstanceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->className.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->className);

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName());

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.enumerateInstanceNames: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: Calling provider : enumerateInstanceNames: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract java.util.Vector enumInstances (org.pegasus.jmpi.CIMObjectPath cop,
           //                                                 boolean                        deep,
           //                                                 org.pegasus.jmpi.CIMClass      cimClass)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "enumInstances",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMObjectPath[] enumerateInstanceNames (org.pegasus.jmpi.CIMObjectPath op,
               //                                                                 org.pegasus.jmpi.CIMClass      cc)
               //         throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "enumerateInstanceNames",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: found METHOD_CIMINSTANCEPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract java.util.Vector enumerateInstanceNames (org.pegasus.jmpi.OperationContext oc
           //                                                          org.pegasus.jmpi.CIMObjectPath    cop,
           //                                                          org.pegasus.jmpi.CIMClass         cimClass)
           //         throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "enumerateInstanceNames",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public abstract org.pegasus.jmpi.CIMObjectPath[] enumerateInstanceNames (org.pegasus.jmpi.OperationContext oc
               //                                                                          org.pegasus.jmpi.CIMObjectPath    cop,
               //                                                                          org.pegasus.jmpi.CIMClass         cimClass)
               //         throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "enumerateInstanceNames",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: found METHOD_CIMINSTANCEPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                   id,
                                                                   jcop,
                                                                   jcimClass);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                   id,
                                                                   joc,
                                                                   jcop,
                                                                   jcimClass);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 joc,
                                                 jcop,
                                                 jcimClass);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass    = env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 jcop,
                                                 true,
                                                 jcimClass);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleEnumerateInstanceNamesRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleCreateInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleCreateInstanceRequest");

    HandlerIntro(CreateInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_INSTANCEPROVIDER, // same as METHOD_CIMINSTANCEPROVIDER
       METHOD_INSTANCEPROVIDER2 // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleCreateInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->newInstance.getPath().getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->newInstance.getPath().getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->newInstance.getPath().getClassName(),
                                                       request->newInstance.getPath().getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.createInstance: " + ph.GetProvider().getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: Calling provider createInstance: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public org.pegasus.jmpi.CIMObjectPath createInstance (org.pegasus.jmpi.CIMObjectPath cop,
           //                                                       org.pegasus.jmpi.CIMInstance   cimInstance)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "createInstance",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)Lorg/pegasus/jmpi/CIMObjectPath;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public org.pegasus.jmpi.CIMObjectPath createInstance (org.pegasus.jmpi.OperationContext oc
           //                                                       org.pegasus.jmpi.CIMObjectPath    cop,
           //                                                       org.pegasus.jmpi.CIMInstance      cimInstance)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "createInstance",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)Lorg/pegasus/jmpi/CIMObjectPath;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->newInstance);
            jlong        jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject      jci    = env->NewObject(jv->CIMInstanceClassRef,jv->CIMInstanceNewJ,jciRef);

            JMPIjvm::checkException(env);

            DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: id = "<<id<<", jcop = "<<jcop<<", jci = "<<jci<<PEGASUS_STD(endl));

            StatProviderTimeMeasurement providerTime(response);

            jobject jcopRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                    id,
                                                    jcop,
                                                    jci);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jcopRet) {
                jlong          jCopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCopRetRef);

                handler.deliver(*copRet);
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->newInstance);
            jlong        jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject      jci    = env->NewObject(jv->CIMInstanceClassRef,jv->CIMInstanceNewJ,jciRef);

            JMPIjvm::checkException(env);

            DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: id = "<<id<<", jcop = "<<jcop<<", jci = "<<jci<<PEGASUS_STD(endl));

            StatProviderTimeMeasurement providerTime(response);

            jobject jcopRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                    id,
                                                    joc,
                                                    jcop,
                                                    jci);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            if (jcopRet) {
                jlong          jCopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCopRetRef);

                handler.deliver(*copRet);
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateInstanceRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleModifyInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleModifyInstanceRequest");

    HandlerIntro(ModifyInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2, // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleModifyInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->modifiedInstance.getPath().getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->modifiedInstance.getPath().getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->modifiedInstance.getPath ().getClassName(),
                                                       request->modifiedInstance.getPath ().getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: provider name physical = "<<name.getPhysicalName()<<", logical = "<<name.getLogicalName()<<PEGASUS_STD(endl));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.modifyInstance: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: Calling provider "<<PEGASUS_STD(hex)<<(long)&pr<<PEGASUS_STD(dec)<<", name = "<<pr.getName ()<<", module = "<<pr.getModule()<<" modifyInstance: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract void setInstance (org.pegasus.jmpi.CIMObjectPath cop,
           //                                   org.pegasus.jmpi.CIMInstance   cimInstance)
           //        org.pegasus.jmpi.throws CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "setInstance",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public void setInstance (org.pegasus.jmpi.CIMObjectPath op,
               //                          org.pegasus.jmpi.CIMInstance   ci,
               //                          boolean                        includeQualifiers,
               //                          java.lang.String[]             propertyList)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "setInstance",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Z[Ljava/lang/String)V");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: found METHOD_CIMINSTANCEPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract void setInstance (org.pegasus.jmpi.OperationContext oc,
           //                                   org.pegasus.jmpi.CIMObjectPath    op,
           //                                   org.pegasus.jmpi.CIMInstance      cimInstance);
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "setInstance",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->modifiedInstance);
            jlong        jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject      jci    = env->NewObject(jv->CIMInstanceClassRef,jv->CIMInstanceNewJ,jciRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                joc,
                                jcop,
                                jci);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->modifiedInstance);
            jlong        jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject      jci    = env->NewObject(jv->CIMInstanceClassRef,jv->CIMInstanceNewJ,jciRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                jcop,
                                jci,
                                JMPI_INCLUDE_QUALIFIERS,
                                jPropertyList);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->modifiedInstance);
            jlong        jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject      jci    = env->NewObject(jv->CIMInstanceClassRef,jv->CIMInstanceNewJ,jciRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                jcop,
                                jci);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleModifyInstanceRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDeleteInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleDeleteInstanceRequest");

    HandlerIntro(DeleteInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_INSTANCEPROVIDER,  // same as METHOD_CIMINSTANCEPROVIDER
       METHOD_INSTANCEPROVIDER2, // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleDeleteInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteInstanceRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->instanceName.getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->instanceName.getClassName(),
                                                       request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.deleteInstance: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteInstanceRequest: Calling provider deleteInstance: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract void deleteInstance (org.pegasus.jmpi.CIMObjectPath cop)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "deleteInstance",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteInstanceRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract void deleteInstance (org.pegasus.jmpi.OperationContext oc,
           //                                      org.pegasus.jmpi.CIMObjectPath    cop)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "deleteInstance",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteInstanceRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteInstanceRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                joc,
                                jcop);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                jcop);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteInstanceRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleExecQueryRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleExecQueryRequest");

    HandlerIntro(ExecQuery,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_CIMINSTANCEPROVIDER2,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleExecQueryRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->className.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->className);

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.execQuery: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: Calling provider execQuery: "<<pr.getName()<<", queryLanguage: "<<request->queryLanguage<<", query: "<<request->query<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract java.util.Vector execQuery (org.pegasus.jmpi.CIMObjectPath cop,
           //                                             java.lang.String               queryStatement,
           //                                             int                            ql,
           //                                             org.pegasus.jmpi.CIMClass      cimClass)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "execQuery",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ILorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: found METHOD_INSTANCEPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public abstract org.pegasus.jmpi.CIMInstance[] execQuery(org.pegasus.jmpi.CIMObjectPath cop,
               //                                                          java.lang.String               query,
               //                                                          java.lang.String               ql,
               //                                                          org.pegasus.jmpi.CIMClass      cimClass)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "execQuery",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: found METHOD_CIMINSTANCEPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract java.util.Vector execQuery (org.pegasus.jmpi.OperationContext oc,
           //                                             org.pegasus.jmpi.CIMObjectPath    cop,
           //                                             org.pegasus.jmpi.CIMClass         cimClass,
           //                                             java.lang.String                  queryStatement,
           //                                             java.lang.String                  queryLanguage)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "execQuery",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: found METHOD_INSTANCEPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public abstract org.pegasus.jmpi.CIMInstance[] execQuery (org.pegasus.jmpi.OperationContext oc,
               //                                                           org.pegasus.jmpi.CIMObjectPath    cop,
               //                                                           org.pegasus.jmpi.CIMClass         cimClass,
               //                                                           java.lang.String                  queryStatement,
               //                                                           java.lang.String                  queryLanguage)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "execQuery",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Ljava/lang/String;Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: found METHOD_CIMINSTANCEPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(request->query.getCString());

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcls);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                   id,
                                                                   jcop,
                                                                   jquery,
                                                                   jqueryLanguage,
                                                                   jCc);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(request->query.getCString());

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcls);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                   id,
                                                                   joc,
                                                                   jcop,
                                                                   jquery,
                                                                   jqueryLanguage,
                                                                   jCc);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(request->query.getCString());

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcls);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod ((jobject)pr.jProvider,
                                                  id,
                                                  joc,
                                                  jcop,
                                                  jCc,
                                                  jquery,
                                                  jqueryLanguage);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec)
            {
                for (int i = 0, m = env->CallIntMethod (jVec, JMPIjvm::jv.VectorSize); i < m; i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
                    DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: jciRet = "<<jciRet<<PEGASUS_STD(endl));

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: done!"<<PEGASUS_STD(endl));
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(request->query.getCString());

            CIMClass cls;

            try
            {
               DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<request->className<<PEGASUS_STD(endl));
            }
            catch (CIMException e)
            {
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewJ,jcls);

            JMPIjvm::checkException(env);

            jlong jql = 0; // @BUG - how to convert?

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod ((jobject)pr.jProvider,
                                                  id,
                                                  jcop,
                                                  jquery,
                                                  jql,
                                                  jCc);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong        jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleExecQueryRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleAssociatorsRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleAssociatorsRequest");

    HandlerIntro(Associators,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleAssociatorsRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->objectName.getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->objectName.getClassName(),
                                                       request->objectName.getKeyBindings());
        CIMObjectPath *assocPath  = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->assocClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.associators: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: Calling provider associators: "<<pr.getName()<<", role: "<<request->role<<", aCls: "<<request->assocClass<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public java.util.Vector associators (org.pegasus.jmpi.CIMObjectPath assocName,
           //                                      org.pegasus.jmpi.CIMObjectPath pathName,
           //                                      java.lang.String               resultClass,
           //                                      java.lang.String               role,
           //                                      java.lang.String               resultRole,
           //                                      boolean                        includeQualifiers,
           //                                      boolean                        includeClassOrigin,
           //                                      java.lang.String[]             propertyList)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "associators",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: found METHOD_ASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMInstance[] associators (org.pegasus.jmpi.CIMObjectPath assocName,
               //                                                    org.pegasus.jmpi.CIMObjectPath pathName,
               //                                                    java.lang.String               resultClass,
               //                                                    java.lang.String               role,
               //                                                    java.lang.String               resultRole,
               //                                                    boolean                        includeQualifiers,
               //                                                    boolean                        includeClassOrigin,
               //                                                    java.lang.String[]             propertyList)
               //        throws org.pegasus.jmpi.CIMException
               //
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "associators",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ[Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: found METHOD_CIMASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public java.util.Vector associators (org.pegasus.jmpi.OperationContext oc,
           //                                      org.pegasus.jmpi.CIMObjectPath    assocName,
           //                                      org.pegasus.jmpi.CIMObjectPath    pathName,
           //                                      java.lang.String                  resultClass,
           //                                      java.lang.String                  role,
           //                                      java.lang.String                  resultRole,
           //                                      boolean                           includeQualifiers,
           //                                      boolean                           includeClassOrigin,
           //                                      java.lang.String[]                propertyList)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "associators",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: found METHOD_ASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMInstance[] associators (org.pegasus.jmpi.OperationContext oc,
               //                                                    org.pegasus.jmpi.CIMObjectPath assocName,
               //                                                    org.pegasus.jmpi.CIMObjectPath pathName,
               //                                                    java.lang.String               resultClass,
               //                                                    java.lang.String               role,
               //                                                    java.lang.String               resultRole,
               //                                                    boolean                        includeQualifiers,
               //                                                    boolean                        includeClassOrigin,
               //                                                    java.lang.String[]             propertyList)
               //        throws org.pegasus.jmpi.CIMException
               //
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "associators",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ[Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: found METHOD_CIMASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: assocName          = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: pathName           = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultClass        = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultRole         = "<<request->resultRole<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance         *ciRet = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op    = ciRet->getPath();
                    CIMObjectPath        iop   = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: assocName          = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: pathName           = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultClass        = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultRole         = "<<request->resultRole<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance         *ciRet = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op    = ciRet->getPath();
                    CIMObjectPath        iop   = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: assocName          = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: pathName           = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultClass        = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultRole         = "<<request->resultRole<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance         *ciRet = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op    = ciRet->getPath();
                    CIMObjectPath        iop   = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: assocName          = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: pathName           = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultClass        = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: resultRole         = "<<request->resultRole<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorsRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong                jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance         *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op        = ciRet->getPath();
                    CIMObjectPath        iop       = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorsRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleAssociatorNamesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleAssociatorNamesRequest");

    HandlerIntro(AssociatorNames,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleAssociatorNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->objectName.getClassName().getString()<<", assocName = "<<request->assocClass.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->objectName.getClassName(),
                                                       request->objectName.getKeyBindings());
        CIMObjectPath *assocPath  = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->assocClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.associatorNames: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: Calling provider associatorNames: "<<pr.getName()<<", role: "<<request->role<<", aCls: "<<request->assocClass<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public java.util.Vector associatorNames (org.pegasus.jmpi.CIMObjectPath assocName,
           //                                          org.pegasus.jmpi.CIMObjectPath pathName,
           //                                          java.lang.String               resultClass,
           //                                          java.lang.String               role,
           //                                          java.lang.String               resultRole)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "associatorNames",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: found METHOD_ASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMObjectPath[] associatorNames (org.pegasus.jmpi.CIMObjectPath assocName,
               //                                                          org.pegasus.jmpi.CIMObjectPath pathName,
               //                                                          java.lang.String               resultClass,
               //                                                          java.lang.String               role,
               //                                                          java.lang.String               resultRole)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "associatorNames",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: found METHOD_CIMASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public java.util.Vector associatorNames (org.pegasus.jmpi.OperationContext oc,
           //                                          org.pegasus.jmpi.CIMObjectPath    assocName,
           //                                          org.pegasus.jmpi.CIMObjectPath    pathName,
           //                                          java.lang.String                  resultClass,
           //                                          java.lang.String                  role,
           //                                          java.lang.String                  resultRole)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "associatorNames",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: found METHOD_ASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMObjectPath[] associatorNames (org.pegasus.jmpi.OperationContext oc,
               //                                                          org.pegasus.jmpi.CIMObjectPath    assocName,
               //                                                          org.pegasus.jmpi.CIMObjectPath    pathName,
               //                                                          java.lang.String                  resultClass,
               //                                                          java.lang.String                  role,
               //                                                          java.lang.String                  resultRole)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "associatorNames",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: found METHOD_CIMASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: assocName   = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultClass = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: role        = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultRole  = "<<request->resultRole<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: assocName   = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultClass = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: role        = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultRole  = "<<request->resultRole<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: assocName   = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: pathName    = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultClass = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: role        = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultRole  = "<<request->resultRole<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, assocPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass = env->NewStringUTF(request->resultClass.getString().getCString());
            jstring jRole        = env->NewStringUTF(request->role.getCString());
            jstring jResultRole  = env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: assocName   = "<<assocPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: pathName    = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultClass = "<<request->resultClass<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: role        = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: resultRole  = "<<request->resultRole<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jResultClass,
                                                                  jRole,
                                                                  jResultRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong          jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet     = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleAssociatorNamesRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleReferencesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleReferencesRequest");

    HandlerIntro(References,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleReferencesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->objectName.getClassName().getString()<<", result = "<<request->resultClass.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->objectName.getClassName(),
                                                       request->objectName.getKeyBindings());
        CIMObjectPath *resultPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->resultClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.references: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: Calling provider references: "<<pr.getName()<<", role: "<<request->role<<" aCls: "<<request->resultClass<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public java.util.Vector references (org.pegasus.jmpi.CIMObjectPath assocName,
           //                                     org.pegasus.jmpi.CIMObjectPath pathName,
           //                                     java.lang.String               role,
           //                                     boolean                        includeQualifiers,
           //                                     boolean                        includeClassOrigin,
           //                                     java.lang.String[]             propertyList)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "references",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: found METHOD_ASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMInstance[] references (org.pegasus.jmpi.CIMObjectPath assocName,
               //                                                   org.pegasus.jmpi.CIMObjectPath pathName,
               //                                                   java.lang.String               role,
               //                                                   boolean                        includeQualifiers,
               //                                                   boolean                        includeClassOrigin,
               //                                                   java.lang.String[]             propertyList)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "references",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ZZ[Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: found METHOD_CIMASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public java.util.Vector references (org.pegasus.jmpi.OperationContext oc,
           //                                     org.pegasus.jmpi.CIMObjectPath    assocName,
           //                                     org.pegasus.jmpi.CIMObjectPath    pathName,
           //                                     java.lang.String                  role,
           //                                     boolean                           includeQualifiers,
           //                                     boolean                           includeClassOrigin,
           //                                     java.lang.String[]                propertyList)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "references",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: found METHOD_ASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMInstance[] references (org.pegasus.jmpi.OperationContext oc,
               //                                                   org.pegasus.jmpi.CIMObjectPath    assocName,
               //                                                   org.pegasus.jmpi.CIMObjectPath    pathName,
               //                                                   java.lang.String                  role,
               //                                                   boolean                           includeQualifiers,
               //                                                   boolean                           includeClassOrigin,
               //                                                   java.lang.String[]                propertyList)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "references",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;ZZ[Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: found METHOD_CIMASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: assocName          = "<<resultPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance         *ciRet = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op    = ciRet->getPath();
                    CIMObjectPath        iop   = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: assocName          = "<<resultPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance         *ciRet = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op    = ciRet->getPath();
                    CIMObjectPath        iop   = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: assocName          = "<<resultPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: pathName           = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong                jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance         *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op        = ciRet->getPath();
                    CIMObjectPath        iop       = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: assocName          = "<<resultPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: pathName           = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeQualifiers  = "<<false<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferencesRequest: includeClassOrigin = "<<false<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jRole,
                                                                  JMPI_INCLUDE_QUALIFIERS,
                                                                  request->includeClassOrigin,
                                                                  jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong                jciRetRef = env->CallLongMethod(jciRet,JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance         *ciRet     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       DDD (PEGASUS_STD(cout)<<"enter: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       DDD (PEGASUS_STD(cout)<<"exit: cimom_handle->getClass("<<__LINE__<<") "<<ciRet->getClassName()<<PEGASUS_STD(endl));
                    }
                    catch (CIMException e)
                    {
                       DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: Error: Caught CIMExcetion during cimom_handle->getClass("<<__LINE__<<") "<<PEGASUS_STD(endl));
                       throw;
                    }

                    const CIMObjectPath& op        = ciRet->getPath();
                    CIMObjectPath        iop       = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferencesRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleReferenceNamesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleReferenceNamesRequest");

    HandlerIntro(ReferenceNames,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleReferenceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->objectName.getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->objectName.getClassName(),
                                                       request->objectName.getKeyBindings());
        CIMObjectPath *resultPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->resultClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.referenceNames: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: Calling provider referenceNames: "<<pr.getName()<<", role: "<<request->role<<", aCls: "<<request->resultClass<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public java.util.Vector referenceNames (org.pegasus.jmpi.CIMObjectPath assocName,
           //                                         org.pegasus.jmpi.CIMObjectPath pathName,
           //                                         java.lang.String               role)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "referenceNames",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: found METHOD_ASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMObjectPath[] referenceNames (org.pegasus.jmpi.CIMObjectPath assocName,
               //                                                         org.pegasus.jmpi.CIMObjectPath pathName,
               //                                                         java.lang.String               role)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "referenceNames",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: found METHOD_CIMASSOCIATORPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public java.util.Vector referenceNames (org.pegasus.jmpi.OperationContext oc,
           //                                         org.pegasus.jmpi.CIMObjectPath    assocName,
           //                                         org.pegasus.jmpi.CIMObjectPath    pathName,
           //                                         java.lang.String                  role)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "referenceNames",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: found METHOD_ASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMObjectPath[] referenceNames (org.pegasus.jmpi.OperationContext oc,
               //                                                         org.pegasus.jmpi.CIMObjectPath    assocName,
               //                                                         org.pegasus.jmpi.CIMObjectPath    pathName,
               //                                                         java.lang.String                  role)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "referenceNames",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: found METHOD_CIMASSOCIATORPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: assocName          = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jPathName,
                                                                  jAssociationName,
                                                                  jRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: assocName          = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jPathName,
                                                                  jAssociationName,
                                                                  jRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: assocName          = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: pathName           = "<<resultPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jAssociationNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, resultPath);
            jobject jAssociationName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jPathName    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: assocName          = "<<objectPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: pathName           = "<<resultPath->toString ()<<PEGASUS_STD(endl));
            DDD(PEGASUS_STD(cerr)<<"--- JMPIProviderManager::handleReferenceNamesRequest: role               = "<<request->role<<PEGASUS_STD(endl));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod((jobject)pr.jProvider,
                                                                  id,
                                                                  joc,
                                                                  jAssociationName,
                                                                  jPathName,
                                                                  jRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(jcopRet,JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleReferenceNamesRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleGetPropertyRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleGetPropertyRequest");

    HandlerIntro(GetProperty,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_PROPERTYPROVIDER,
       METHOD_PROPERTYPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleGetPropertyRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetPropertyRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->className.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->instanceName.getClassName(),
                                                       request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.getPropertyValue: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetPropertyRequest: Calling provider getPropertyValue: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract org.pegasus.jmpi.CIMValue getPropertyValue (org.pegasus.jmpi.CIMObjectPath cop,
           //                                                             java.lang.String               oclass,
           //                                                             java.lang.String               pName)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "getPropertyValue",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;)Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetPropertyRequest: found METHOD_PROPERTYPROVIDER."<<PEGASUS_STD(endl));
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract org.pegasus.jmpi.CIMValue getPropertyValue (org.pegasus.jmpi.OperationContext oc,
           //                                                             org.pegasus.jmpi.CIMObjectPath    cop,
           //                                                             java.lang.String                  oclass,
           //                                                             java.lang.String                  pName)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "getPropertyValue",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;)Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetPropertyRequest: found METHOD_PROPERTYPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetPropertyRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_PROPERTYPROVIDER:
        {
            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring joclass = env->NewStringUTF(request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName = env->NewStringUTF(request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jvalRet = env->CallObjectMethod ((jobject)pr.jProvider,
                                                     id,
                                                     jcop,
                                                     joclass,
                                                     jpName);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jvalRet)
            {
               jlong     jvalRetRef = env->CallLongMethod(jvalRet,JMPIjvm::jv.CIMValueCInst);
               CIMValue *valRet     = DEBUG_ConvertJavaToC (jlong, CIMValue*, jvalRetRef);

               JMPIjvm::checkException(env);

               handler.deliver(*valRet);
            }
            handler.complete();
            break;
        }

        case METHOD_PROPERTYPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring joclass = env->NewStringUTF(request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName = env->NewStringUTF(request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jvalRet = env->CallObjectMethod ((jobject)pr.jProvider,
                                                     id,
                                                     joc,
                                                     jcop,
                                                     joclass,
                                                     jpName);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            if (jvalRet)
            {
               jlong     jvalRetRef = env->CallLongMethod(jvalRet,JMPIjvm::jv.CIMValueCInst);
               CIMValue *valRet     = DEBUG_ConvertJavaToC (jlong, CIMValue*, jvalRetRef);

               JMPIjvm::checkException(env);

               handler.deliver(*valRet);
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleGetPropertyRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleSetPropertyRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleSetPropertyRequest");

    HandlerIntro(SetProperty,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_PROPERTYPROVIDER,
       METHOD_PROPERTYPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleSetPropertyRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSetPropertyRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->className.getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->instanceName.getClassName(),
                                                       request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Calling provider.setPropertyValue: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSetPropertyRequest: Calling provider setPropertyValue: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract void setPropertyValue (org.pegasus.jmpi.CIMObjectPath cop,
           //                                        java.lang.String               oclass,
           //                                        java.lang.String               pName,
           //                                        org.pegasus.jmpi.CIMValue      val)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "setPropertyValue",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Lorg/pegasus/jmpi/CIMValue;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSetPropertyRequest: found METHOD_PROPERTYPROVIDER."<<PEGASUS_STD(endl));
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract void setPropertyValue (org.pegasus.jmpi.OperationContext oc,
           //                                        org.pegasus.jmpi.CIMObjectPath    cop,
           //                                        java.lang.String                  oclass,
           //                                        java.lang.String                  pName,
           //                                        org.pegasus.jmpi.CIMValue         val)
           //        throws org.pegasus.jmpi.CIMException
           //
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "setPropertyValue",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;Lorg/pegasus/jmpi/CIMValue;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSetPropertyRequest: found METHOD_PROPERTYPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSetPropertyRequest: found no method!"<<PEGASUS_STD(endl));

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                               "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_PROPERTYPROVIDER:
        {
            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring joclass = env->NewStringUTF(request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName = env->NewStringUTF(request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            CIMValue *val = new CIMValue (request->newValue);

            JMPIjvm::checkException(env);

            jlong   jvalref = DEBUG_ConvertCToJava (CIMValue*, jlong, val);
            jobject jval    = env->NewObject(jv->CIMValueClassRef, jv->CIMValueNewJ, jvalref);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 jcop,
                                 joclass,
                                 jpName,
                                 jval);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_PROPERTYPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopref = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef, jv->CIMObjectPathNewJ, jcopref);

            JMPIjvm::checkException(env);

            jstring joclass = env->NewStringUTF(request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName = env->NewStringUTF(request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            CIMValue *val = new CIMValue (request->newValue);

            JMPIjvm::checkException(env);

            jlong   jvalref = DEBUG_ConvertCToJava (CIMValue*, jlong, val);
            jobject jval    = env->NewObject(jv->CIMValueClassRef, jv->CIMValueNewJ, jvalref);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jcop,
                                 joclass,
                                 jpName,
                                 jval);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSetPropertyRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleInvokeMethodRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleInvokeMethodRequest");

    HandlerIntro(InvokeMethod,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMMETHODPROVIDER,
       METHOD_CIMMETHODPROVIDER2,
       METHOD_METHODPROVIDER,
       METHOD_METHODPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleInvokeMethodRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: hostname = "<<System::getHostName()<<", namespace = "<<request->nameSpace.getString()<<", classname = "<<request->instanceName.getClassName().getString()<<PEGASUS_STD(endl));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->instanceName.getClassName(),
                                                       request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

        JMPIProvider &pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.invokeMethod: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: Calling provider invokeMethod: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public abstract org.pegasus.jmpi.CIMValue invokeMethod (org.pegasus.jmpi.CIMObjectPath cop,
           //                                                         java.lang.String               name,
           //                                                         java.util.Vector               in,
           //                                                         java.util.Vector               out)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "invokeMethod",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/util/Vector;Ljava/util/Vector;)Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_METHODPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: found METHOD_METHODPROVIDER."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMValue invokeMethod (org.pegasus.jmpi.CIMObjectPath op,
               //                                                java.lang.String               methodName,
               //                                                org.pegasus.jmpi.CIMArgument[] inArgs,
               //                                                org.pegasus.jmpi.CIMArgument[] outArgs)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "invokeMethod",
                                     "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;[Lorg/pegasus/jmpi/CIMArgument;[Lorg/pegasus/jmpi/CIMArgument;)Lorg/pegasus/jmpi/CIMValue;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMMETHODPROVIDER;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: found METHOD_CIMMETHODPROVIDER."<<PEGASUS_STD(endl));
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public abstract org.pegasus.jmpi.CIMValue invokeMethod (org.pegasus.jmpi.OperationContext oc,
           //                                                         org.pegasus.jmpi.CIMObjectPath    cop,
           //                                                         java.lang.String                  name,
           //                                                         java.util.Vector                  in,
           //                                                         java.util.Vector                  out)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "invokeMethod",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/util/Vector;Ljava/util/Vector;)Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_METHODPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: found METHOD_METHODPROVIDER2."<<PEGASUS_STD(endl));
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               // public org.pegasus.jmpi.CIMValue invokeMethod (org.pegasus.jmpi.OperationContext oc,
               //                                                org.pegasus.jmpi.CIMObjectPath    op,
               //                                                java.lang.String                  methodName,
               //                                                org.pegasus.jmpi.CIMArgument[]    inArgs,
               //                                                org.pegasus.jmpi.CIMArgument[]    outArgs)
               //        throws org.pegasus.jmpi.CIMException
               id = env->GetMethodID((jclass)pr.jProviderClass,
                                     "invokeMethod",
                                     "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;[Lorg/pegasus/jmpi/CIMArgument;[Lorg/pegasus/jmpi/CIMArgument;)Lorg/pegasus/jmpi/CIMValue;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMMETHODPROVIDER2;
                   DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: found METHOD_CIMMETHODPROVIDER2."<<PEGASUS_STD(endl));
               }
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMMETHODPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod = env->NewStringUTF(request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            Uint32 m=request->inParameters.size();

            jobjectArray jArIn=(jobjectArray)env->NewObjectArray(m,jv->CIMArgumentClassRef,NULL);

            for (Uint32 i=0; i<m; i++) {
              CIMParamValue *parm    = new CIMParamValue(request->inParameters[i]);
              jlong          jArgRef = DEBUG_ConvertCToJava (CIMParamValue*, jlong, parm);
              jobject        jArg    = env->NewObject(jv->CIMArgumentClassRef,jv->CIMArgumentNewJ,jArgRef);

              env->SetObjectArrayElement(jArIn,i,jArg);
            }

            jobjectArray jArOut=(jobjectArray)env->NewObjectArray(24,jv->CIMArgumentClassRef,NULL);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      jcop,
                                                      jMethod,
                                                      jArIn,
                                                      jArOut);
            JMPIjvm::checkException(env);

            handler.processing();

            jlong     jValueRetRef = env->CallLongMethod(jValueRet,JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet     = DEBUG_ConvertJavaToC (jlong, CIMValue*, jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0; i<24; i++) {
                jobject jArg = env->GetObjectArrayElement(jArOut,i);

                JMPIjvm::checkException(env);

                if (jArg==NULL)
                   break;

                jlong          jpRef = env->CallLongMethod(jArg,JMPIjvm::jv.CIMArgumentCInst);
                CIMParamValue *p     = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(*p);
            }

            handler.complete();
            break;
        }

        case METHOD_CIMMETHODPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod = env->NewStringUTF(request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            Uint32 m=request->inParameters.size();

            jobjectArray jArIn=(jobjectArray)env->NewObjectArray(m,jv->CIMArgumentClassRef,NULL);

            for (Uint32 i=0; i<m; i++) {
              CIMParamValue *parm    = new CIMParamValue(request->inParameters[i]);
              jlong          jArgRef = DEBUG_ConvertCToJava (CIMParamValue*, jlong, parm);
              jobject        jArg    = env->NewObject(jv->CIMArgumentClassRef,jv->CIMArgumentNewJ,jArgRef);

              env->SetObjectArrayElement(jArIn,i,jArg);
            }

            jobjectArray jArOut=(jobjectArray)env->NewObjectArray(24,jv->CIMArgumentClassRef,NULL);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      joc,
                                                      jcop,
                                                      jMethod,
                                                      jArIn,
                                                      jArOut);
            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            jlong     jValueRetRef = env->CallLongMethod(jValueRet,JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet     = DEBUG_ConvertJavaToC (jlong, CIMValue*, jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0; i<24; i++) {
                jobject jArg = env->GetObjectArrayElement(jArOut,i);

                JMPIjvm::checkException(env);

                if (jArg==NULL)
                   break;

                jlong          jpRef = env->CallLongMethod(jArg,JMPIjvm::jv.CIMArgumentCInst);
                CIMParamValue *p     = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(*p);
            }

            handler.complete();
            break;
        }

        case METHOD_METHODPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod = env->NewStringUTF(request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            jobject jVecIn = env->NewObject(jv->VectorClassRef,jv->VectorNew);

            JMPIjvm::checkException(env);

            for (int i=0,m=request->inParameters.size(); i<m; i++)
            {
                const CIMParamValue &parm  = request->inParameters[i];
                const CIMValue       v     = parm.getValue();
                CIMProperty         *p     = new CIMProperty(parm.getParameterName(),v,v.getArraySize());
                jlong                jpRef = DEBUG_ConvertCToJava (CIMProperty*, jlong, p);
                jobject              jp    = env->NewObject(jv->CIMPropertyClassRef,jv->CIMPropertyNewJ,jpRef);

                env->CallVoidMethod(jVecIn,jv->VectorAddElement,jp);
             }

            jobject jVecOut=env->NewObject(jv->VectorClassRef,jv->VectorNew);
            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      jcop,
                                                      jMethod,
                                                      jVecIn,
                                                      jVecOut);
            JMPIjvm::checkException(env);

            handler.processing();

            jlong     jValueRetRef = env->CallLongMethod(jValueRet,JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet     = DEBUG_ConvertJavaToC (jlong, CIMValue*, jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0,m=env->CallIntMethod(jVecOut,JMPIjvm::jv.VectorSize); i<m; i++)
            {
                JMPIjvm::checkException(env);

                jobject jProp = env->CallObjectMethod(jVecOut,JMPIjvm::jv.VectorElementAt,i);

                JMPIjvm::checkException(env);

                jlong        jpRef = env->CallLongMethod(jProp,JMPIjvm::jv.CIMPropertyCInst);
                CIMProperty *p     = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(CIMParamValue(p->getName().getString(),p->getValue()));
            }

            handler.complete();
            break;
        }

        case METHOD_METHODPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            jlong   jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, objectPath);
            jobject jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod = env->NewStringUTF(request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            jobject jVecIn = env->NewObject(jv->VectorClassRef,jv->VectorNew);

            JMPIjvm::checkException(env);

            for (int i=0,m=request->inParameters.size(); i<m; i++)
            {
                const CIMParamValue &parm  = request->inParameters[i];
                const CIMValue       v     = parm.getValue();
                CIMProperty         *p     = new CIMProperty(parm.getParameterName(),v,v.getArraySize());
                jlong                jpRef = DEBUG_ConvertCToJava (CIMProperty*, jlong, p);
                jobject              jp    = env->NewObject(jv->CIMPropertyClassRef,jv->CIMPropertyNewJ,jpRef);

                env->CallVoidMethod(jVecIn,jv->VectorAddElement,jp);
             }

            jobject jVecOut=env->NewObject(jv->VectorClassRef,jv->VectorNew);
            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      joc,
                                                      jcop,
                                                      jMethod,
                                                      jVecIn,
                                                      jVecOut);
            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            jlong     jValueRetRef = env->CallLongMethod(jValueRet,JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet     = DEBUG_ConvertJavaToC (jlong, CIMValue*, jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0,m=env->CallIntMethod(jVecOut,JMPIjvm::jv.VectorSize); i<m; i++)
            {
                JMPIjvm::checkException(env);

                jobject jProp = env->CallObjectMethod(jVecOut,JMPIjvm::jv.VectorElementAt,i);

                JMPIjvm::checkException(env);

                jlong        jpRef = env->CallLongMethod(jProp,JMPIjvm::jv.CIMPropertyCInst);
                CIMProperty *p     = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(CIMParamValue(p->getName().getString(),p->getValue()));
            }

            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleInvokeMethodRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

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

WQLSelectStatement *
newSelectExp (String& query,
              String& queryLanguage)
{
   WQLSelectStatement *stmt = new WQLSelectStatement (queryLanguage, query);

   try
   {
      WQLParser::parse (query, *stmt);
   }
   catch (const Exception &e)
   {
      cerr << "Error: newSelectExp caught: " << e.getMessage () << endl;
   }

   return stmt;
}

Message * JMPIProviderManager::handleCreateSubscriptionRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleCreateSubscriptionRequest");

    HandlerIntroInd(CreateSubscription,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_EVENTPROVIDER,
       METHOD_EVENTPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION  eMethodFound = METHOD_UNKNOWN;
    JNIEnv         *env          = NULL;

    try {
        String               fileName,
                             providerName,
                             providerLocation;
        CIMInstance          req_provider,
                             req_providerModule;
        ProviderIdContainer  pidc               = (ProviderIdContainer)request->operationContext.get(ProviderIdContainer::NAME);

        req_provider       = pidc.getProvider ();
        req_providerModule = pidc.getModule ();

        LocateIndicationProviderNames (req_provider,
                                       req_providerModule,
                                       providerName,
                                       providerLocation);

        fileName = resolveFileName (providerLocation);

        Logger::put (Logger::STANDARD_LOG,
                     System::CIMSERVER,
                     Logger::TRACE,
                     "JMPIProviderManager::handleCreateSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
                     System::getHostName(),
                     request->nameSpace.getString(),
                     providerName);

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: hostname = "
                             <<System::getHostName()
                             <<", namespace = "
                             <<request->nameSpace.getString()
                             <<", providername = "
                             <<providerName
                             <<", fileName = "
                             <<fileName
                             <<PEGASUS_STD(endl));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (fileName,
                                                                         providerName,
                                                                         String::EMPTY);

        //
        //  Save the provider instance from the request
        //
        ph.GetProvider ().setProviderInstance (req_provider);

        JMPIProvider &pr = ph.GetProvider ();

        //
        //  Increment count of current subscriptions for this provider
        //
        pr.testIfZeroAndIncrementSubscriptions ();

        SubscriptionFilterConditionContainer  sub_cntr = request->operationContext.get (SubscriptionFilterConditionContainer::NAME);
        indProvRecord                        *prec     = NULL;
        bool                                  fNewPrec = false;

        {
           AutoMutex lock (mutexProvTab);

           provTab.lookup (providerName, prec);

           if (!prec)
           {
               fNewPrec = true;

               prec = new indProvRecord ();

               // convert arguments
               prec->ctx = new OperationContext ();

               prec->ctx->insert (request->operationContext.get (IdentityContainer::NAME));
               prec->ctx->insert (request->operationContext.get (AcceptLanguageListContainer::NAME));
               prec->ctx->insert (request->operationContext.get (ContentLanguageListContainer::NAME));
               prec->ctx->insert (request->operationContext.get (SubscriptionInstanceContainer::NAME));
               prec->ctx->insert (request->operationContext.get (SubscriptionFilterConditionContainer::NAME));

               prec->enabled = true;

               prec->handler = new EnableIndicationsResponseHandler (0,
                                                                     0,
                                                                     req_provider,
                                                                     _indicationCallback,
                                                                     _responseChunkCallback);

               DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: Adding to provTab "<<providerName<<PEGASUS_STD(endl));

               provTab.insert (providerName, prec);
           }
        }

        {
           AutoMutex lock (prec->mutex);

           prec->count++;
        }

        // Add a selection record for JNI CIMOMHandle deliverEvent calls
        indSelectRecord *srec = new indSelectRecord ();

        {
           srec->query         = request->query;
           srec->queryLanguage = sub_cntr.getQueryLanguage ();
           srec->propertyList  = request->propertyList;

           CIMOMHandleQueryContext *qContext = new CIMOMHandleQueryContext (CIMNamespaceName (request->nameSpace.getString ()),
                                                                            *pr._cimom_handle);
           srec->qContext = qContext;

           CIMObjectPath        sPath = request->subscriptionInstance.getPath ();
           Array<CIMKeyBinding> kb;

           // Technically we only need Name and Handler for uniqueness
           kb = sPath.getKeyBindings ();

           // Add an entry for every provider.
           kb.append (CIMKeyBinding ("Provider",
                                     pr.getName (),
                                     CIMKeyBinding::STRING));

           sPath.setKeyBindings (kb);

           AutoMutex lock (mutexSelxTab);

           DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: Adding to selxTab "<<sPath.toString ()<<PEGASUS_STD(endl));

           selxTab.insert (sPath.toString (), srec);

           DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: For selxTab "<<sPath.toString ()<<", srec = "<<PEGASUS_STD(hex)<<(long)srec<<PEGASUS_STD(dec)<<", qContext = "<<PEGASUS_STD(hex)<<(long)qContext<<PEGASUS_STD(dec)<<PEGASUS_STD(endl));
        }

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.createSubscriptionRequest: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: Calling provider createSubscriptionRequest: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (pidc,
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public void activateFilter (org.pegasus.jmpi.SelectExp     filter,
           //                             java.lang.String               eventType,
           //                             org.pegasus.jmpi.CIMObjectPath classPath,
           //                             boolean                        firstActivation)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID ((jclass)pr.jProviderClass,
                                  "activateFilter",
                                  "(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: found METHOD_EVENTPROVIDER."<<PEGASUS_STD(endl));
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public void activateFilter (org.pegasus.jmpi.OperationContext oc,
           //                             org.pegasus.jmpi.SelectExp        filter,
           //                             java.lang.String                  eventType,
           //                             org.pegasus.jmpi.CIMObjectPath    classPath,
           //                             boolean                           firstActivation)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID ((jclass)pr.jProviderClass,
                                  "activateFilter",
                                  "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: found METHOD_EVENTPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_EVENTPROVIDER:
        {
            WQLSelectStatement *stmt       = newSelectExp (srec->query,
                                                           srec->queryLanguage);
            jlong               jStmtRef   = DEBUG_ConvertCToJava (WQLSelectStatement *, jlong, stmt);
            jobject             jSelectExp = env->NewObject(jv->SelectExpClassRef,jv->SelectExpNewJ,jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop     = new CIMObjectPath (System::getHostName(),
                                                        request->nameSpace,
                                                        request->classNames[0]);
            jlong          jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject        jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fNewPrec);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_EVENTPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            WQLSelectStatement *stmt       = newSelectExp (srec->query,
                                                           srec->queryLanguage);
            jlong               jStmtRef   = DEBUG_ConvertCToJava (WQLSelectStatement *, jlong, stmt);
            jobject             jSelectExp = env->NewObject(jv->SelectExpClassRef,jv->SelectExpNewJ,jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop     = new CIMObjectPath (System::getHostName(),
                                                        request->nameSpace,
                                                        request->classNames[0]);
            jlong          jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject        jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fNewPrec);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleCreateSubscriptionRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDeleteSubscriptionRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleDeleteSubscriptionRequest");

    HandlerIntroInd(DeleteSubscription,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_EVENTPROVIDER,
       METHOD_EVENTPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION           eMethodFound = METHOD_UNKNOWN;
    JNIEnv                  *env          = NULL;
    bool                     fFreePrec    = false;
    indProvRecord           *prec         = NULL;
    indSelectRecord         *srec         = NULL;

    try {
        String              fileName,
                            providerName,
                            providerLocation;
        CIMInstance         req_provider,
                            req_providerModule;
        ProviderIdContainer pidc               = (ProviderIdContainer)request->operationContext.get(ProviderIdContainer::NAME);

        req_provider       = pidc.getProvider ();
        req_providerModule = pidc.getModule ();

        LocateIndicationProviderNames (req_provider,
                                       req_providerModule,
                                       providerName,
                                       providerLocation);

        fileName = resolveFileName (providerLocation);

        Logger::put (Logger::STANDARD_LOG,
                     System::CIMSERVER,
                     Logger::TRACE,
                     "JMPIProviderManager::handleDeleteSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
                     System::getHostName(),
                     request->nameSpace.getString(),
                     providerName);

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: hostname = "
                             <<System::getHostName()
                             <<", namespace = "
                             <<request->nameSpace.getString()
                             <<", providername = "
                             <<providerName
                             <<", fileName = "
                             <<fileName
                             <<PEGASUS_STD(endl));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (fileName,
                                                                         providerName,
                                                                         String::EMPTY);

        JMPIProvider &pr = ph.GetProvider ();

        {
           AutoMutex lock (mutexProvTab);

           provTab.lookup (providerName, prec);
        }

        {
           AutoMutex lock (prec->mutex);

           if (--prec->count <= 0)
           {
               DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: Removing provTab "<<providerName<<PEGASUS_STD(endl));

               provTab.remove (providerName);

               fFreePrec = true;
           }
        }

        {
           CIMObjectPath        sPath = request->subscriptionInstance.getPath ();
           Array<CIMKeyBinding> kb;

           // Technically we only need Name and Handler for uniqueness
           kb = sPath.getKeyBindings ();

           // Add an entry for every provider.
           kb.append (CIMKeyBinding ("Provider",
                                     pr.getName (),
                                     CIMKeyBinding::STRING));

           sPath.setKeyBindings (kb);

           String sPathString = sPath.toString ();

           AutoMutex lock (mutexSelxTab);

           DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: Removing selxTab "<<sPathString<<PEGASUS_STD(endl));

           if (!selxTab.lookup (sPathString, srec))
           {
               PEGASUS_ASSERT(0);
           }

           DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: For selxTab "<<sPathString<<", srec = "<<PEGASUS_STD(hex)<<(long)srec<<PEGASUS_STD(dec)<<", qContext = "<<PEGASUS_STD(hex)<<(long)srec->qContext<<PEGASUS_STD(dec)<<PEGASUS_STD(endl));

           selxTab.remove (sPathString);
        }

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, "Calling provider.deleteSubscriptionRequest: " + pr.getName());

        DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: Calling provider deleteSubscriptionRequest: "<<pr.getName()<<PEGASUS_STD(endl));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                           MessageLoaderParms("ProviderManager.JMPI.INIT_JVM_FAILED",
                                                              "Could not initialize the JVM (Java Virtual Machine) runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (request->operationContext.get (ProviderIdContainer::NAME),
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           // public void deActivateFilter (org.pegasus.jmpi.SelectExp    filter,
           //                              java.lang.String               eventType,
           //                              org.pegasus.jmpi.CIMObjectPath classPath,
           //                              boolean                        lastActivation)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "deActivateFilter",
                                 "(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: found METHOD_EVENTPROVIDER."<<PEGASUS_STD(endl));
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           // public void deActivateFilter (org.pegasus.jmpi.OperationContext oc,
           //                               org.pegasus.jmpi.SelectExp        filter,
           //                               java.lang.String                  eventType,
           //                               org.pegasus.jmpi.CIMObjectPath    classPath,
           //                               boolean                           lastActivation)
           //        throws org.pegasus.jmpi.CIMException
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "deActivateFilter",
                                 "(Lorg/pegasus/jmpi/OperationContext;Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER2;
               DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: found METHOD_EVENTPROVIDER2."<<PEGASUS_STD(endl));
           }
        }

        if (id == NULL)
        {
           DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: No method found!"<<PEGASUS_STD(endl));

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED,
                                          MessageLoaderParms ("ProviderManager.JMPI.METHOD_NOT_FOUND",
                                                              "Could not find a method for the provider based on InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_EVENTPROVIDER:
        {
            WQLSelectStatement *stmt       = newSelectExp (srec->query,
                                                           srec->queryLanguage);
            jlong               jStmtRef   = DEBUG_ConvertCToJava (WQLSelectStatement *, jlong, stmt);
            jobject             jSelectExp = env->NewObject(jv->SelectExpClassRef,jv->SelectExpNewJ,jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop     = new CIMObjectPath (System::getHostName(),
                                                        request->nameSpace,
                                                        request->classNames[0]);
            jlong          jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject        jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fFreePrec);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_EVENTPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava (OperationContext*, jlong, &request->operationContext);
            jobject joc    = env->NewObject(jv->OperationContextClassRef,jv->OperationContextNewJ,jocRef);

            WQLSelectStatement *stmt       = newSelectExp (srec->query,
                                                           srec->queryLanguage);
            jlong               jStmtRef   = DEBUG_ConvertCToJava (WQLSelectStatement *, jlong, stmt);
            jobject             jSelectExp = env->NewObject(jv->SelectExpClassRef,jv->SelectExpNewJ,jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop     = new CIMObjectPath (System::getHostName(),
                                                        request->nameSpace,
                                                        request->classNames[0]);
            jlong          jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject        jcop    = env->NewObject(jv->CIMObjectPathClassRef,jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fFreePrec);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod (joc, JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_UNKNOWN:
        {
            DDD (PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleDeleteSubscriptionRequest: should not be here!"<<PEGASUS_STD(endl));
            break;
        }
        }

        //
        //  Decrement count of current subscriptions for this provider
        //
        pr.decrementSubscriptionsAndTestIfZero ();
    }
    HandlerCatch(handler);

    if (srec)
    {
       delete srec->qContext;
    }
    delete srec;

    if (fFreePrec)
    {
       delete prec->ctx;
       delete prec->handler;
       delete prec;
    }

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDisableModuleRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleDisableModuleRequest");

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

    Array<Uint16> operationalStatus;
    // Assume success.
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    // Unload providers
    //
    Array<CIMInstance> _pInstances = request->providers;

    CIMDisableModuleResponseMessage * response =
        new CIMDisableModuleResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop(),
        operationalStatus);

    PEGASUS_ASSERT(response != 0);

    //
    //  Set HTTP method in response from request
    //
    response->setHttpMethod (request->getHttpMethod ());

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnableModuleRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleEnableModuleRequest");

    CIMEnableModuleRequestMessage * request =
        dynamic_cast<CIMEnableModuleRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    Array<Uint16> operationalStatus;
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);

    CIMEnableModuleResponseMessage * response =
        new CIMEnableModuleResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop(),
        operationalStatus);

    PEGASUS_ASSERT(response != 0);

    //  Set HTTP method in response from request
    response->setHttpMethod (request->getHttpMethod ());

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleStopAllProvidersRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleStopAllProvidersRequest");

    CIMStopAllProvidersRequestMessage * request =
        dynamic_cast<CIMStopAllProvidersRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    CIMStopAllProvidersResponseMessage * response =
        new CIMStopAllProvidersResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop());

    PEGASUS_ASSERT(response != 0);

    //  Set HTTP method in response from request
    response->setHttpMethod (request->getHttpMethod ());

    // tell the provider manager to shutdown all the providers
    providerManager.shutdownAllProviders();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleInitializeProviderRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleInitializeProviderRequest");

    HandlerIntroInit(InitializeProvider,message,request,response,handler);

    try
    {
        // resolve provider name
        ProviderName name = _resolveProviderName(
           request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider (name.getPhysicalName(),
                                                                         name.getLogicalName(),
                                                                         String::EMPTY);

    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleSubscriptionInitCompleteRequest (const Message * message)
{
    PEG_METHOD_ENTER (TRC_PROVIDERMANAGER,
     "JMPIProviderManager::handleSubscriptionInitCompleteRequest");

    CIMSubscriptionInitCompleteRequestMessage * request =
        dynamic_cast <CIMSubscriptionInitCompleteRequestMessage *>
            (const_cast <Message *> (message));

    PEGASUS_ASSERT (request != 0);

    CIMSubscriptionInitCompleteResponseMessage * response =
        dynamic_cast <CIMSubscriptionInitCompleteResponseMessage *>
            (request->buildResponse ());

    PEGASUS_ASSERT (response != 0);

    //
    //  Set indicator
    //
    _subscriptionInitComplete = true;

    //
    //  For each provider that has at least one subscription, call
    //  provider's enableIndications method
    //
    Array <JMPIProvider *> enableProviders;

    enableProviders = providerManager.getIndicationProvidersToEnable ();

    Uint32 numProviders = enableProviders.size ();

    DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSubscriptionInitCompleteRequest: numProviders = "<<numProviders<<PEGASUS_STD(endl));

#if 0
    for (Uint32 i = 0; i < numProviders; i++)
    {
        try
        {
            CIMInstance provider;

            provider = enableProviders[i]->getProviderInstance ();

            DDD(PEGASUS_STD(cout)<<"--- JMPIProviderManager::handleSubscriptionInitCompleteRequest: name = "<<enableProviders[i]->getName ()<<PEGASUS_STD(endl));

            //
            //  Get cached or load new provider module
            //
            JMPIProvider::OpProviderHolder  ph   = providerManager.getProvider (enableProviders[i]->getModule ()->getFileName (),
                                                                                enableProviders[i]->getName ());
            indProvRecord                  *prec = NULL;

            {
               AutoMutex lock (mutexProvTab);

               provTab.lookup (enableProviders[i]->getName (), prec);
            }
        }
        catch (CIMException & e)
        {
            PEG_TRACE_STRING (TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "CIMException: " + e.getMessage ());
        }
        catch (Exception & e)
        {
            PEG_TRACE_STRING (TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "Exception: " + e.getMessage ());
        }
        catch(...)
        {
            PEG_TRACE_CSTRING (TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "Unknown error in handleSubscriptionInitCompleteRequest");
        }
    }
#endif

    PEG_METHOD_EXIT ();
    return (response);
}

Message * JMPIProviderManager::handleUnsupportedRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIProviderManager::handleUnsupportedRequest");

    CIMRequestMessage* request =
        dynamic_cast<CIMRequestMessage *>(const_cast<Message *>(message));
    PEGASUS_ASSERT(request != 0 );

    CIMResponseMessage* response = request->buildResponse();
    response->cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

    PEG_METHOD_EXIT();
    return response;
}

ProviderName JMPIProviderManager::_resolveProviderName(
    const ProviderIdContainer & providerId)
{
    String providerName;
    String fileName;
    String moduleName;
    CIMValue genericValue;

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty("Name")).getValue();
    genericValue.get(moduleName);

    genericValue = providerId.getProvider().getProperty(
        providerId.getProvider().findProperty("Name")).getValue();
    genericValue.get(providerName);

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty("Location")).getValue();
    genericValue.get(fileName);
    fileName = resolveFileName(fileName);

    return ProviderName(moduleName, providerName, fileName);
}

String JMPIProviderManager::resolveFileName(String fileName)
{
    String name = ConfigManager::getHomedPath(ConfigManager::getInstance()->getCurrentValue("providerDir"));
    // physfilename = everything up to the delimiter pointing at class start
    // in case there is no delimiter anymore, it takes the entire filename
    String physfilename = fileName.subString(0, fileName.find(":"));
    // look in all(multiple) homed pathes for the physical file
    name = FileSystem::getAbsoluteFileName(name, physfilename);
    // construct back the fully specified jar:<classname> provider name
    name = FileSystem::extractFilePath(name) + fileName;
    return name;
}

PEGASUS_NAMESPACE_END
