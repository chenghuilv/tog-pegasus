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
// Author: Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
// Modified By: Heather Sterling, IBM (hsterl@us.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CIMClientRep.h"

// l10n
#include <Pegasus/Common/MessageLoader.h>

#include <iostream>
#include <fstream>
#ifdef PEGASUS_PLATFORM_WIN32_IX86_MSVC
# include <windows.h>
#else
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// CIMClientRep
//
///////////////////////////////////////////////////////////////////////////////
static Boolean verifyServerCertificate(SSLCertificateInfo &certInfo)
{
    //ATTN-NB-03-05132002: Add code to handle server certificate verification.
    return true;
}

CIMClientRep::CIMClientRep(Uint32 timeoutMilliseconds)
    :
    MessageQueue(PEGASUS_QUEUENAME_CLIENT),
    _httpConnection(0),
    _timeoutMilliseconds(timeoutMilliseconds),
    _connected(false),
    _responseDecoder(0),
    _requestEncoder(0),
    _connectSSLContext(0)
{
    //
    // Create Monitor and HTTPConnector
    //
    _monitor = new Monitor();
    _httpConnector = new HTTPConnector(_monitor);

// l10n
    requestAcceptLanguages = AcceptLanguages::EMPTY;
    requestContentLanguages = ContentLanguages::EMPTY;
}

CIMClientRep::~CIMClientRep()
{
   disconnect();
   delete _httpConnector;
   delete _monitor;
}

void CIMClientRep::handleEnqueue()
{

}

Uint32 _getShowType(String& s)
{
    String log = "log";
    String con = "con";
    String both = "both";
    if (s == log)
        return 2;
    if (s == con)
        return 1;
    if (s == both)
        return 3;
    return 0;
}

void CIMClientRep::_connect()
{
    //
    // Test for Display optons of the form
    // Use Env variable PEGASUS_CLIENT_TRACE= <intrace> : <outtrace
    // intrace = "con" | "log" | "both"
    // outtrace = intrace
    // ex set PEGASUS_CLIENT_TRACE=BOTH:BOTH traces input and output
    // to console and log
    // Keywords are case insensitive.
    // PEP 90
    //
    Uint32 showOutput = 0;
    Uint32 showInput = 0;
#ifdef PEGASUS_CLIENT_TRACE_ENABLE
    String input;
    if (char * envVar = getenv("PEGASUS_CLIENT_TRACE"))
    {
        input = envVar;
        input.toLower();
        String io = String::EMPTY;
        Uint32 pos = input.find(':');
        if (pos == PEG_NOT_FOUND)
            pos = 0;
        else
            io = input.subString(0,pos);

        // some compilers do not allow temporaries to be passed to a
        // reference argument - so break into 2 lines
        String out = input.subString(pos + 1);
        showOutput = _getShowType(out);

        showInput = _getShowType(io);
    }
#endif

    //
    // Create response decoder:
    //
    _responseDecoder = new CIMOperationResponseDecoder(
        this, _requestEncoder, &_authenticator, showInput);

    //
    // Attempt to establish a connection:
    //
    try
    {
        _httpConnection = _httpConnector->connect(_connectHost,
                                                  _connectPortNumber,
                                                  _connectSSLContext,
                                                  _responseDecoder);
    }
    catch (CannotCreateSocketException& e)
    {
        delete _responseDecoder;
        throw e;
    }
    catch (CannotConnectException& e)
    {
        delete _responseDecoder;
        throw e;
    }
    catch (InvalidLocatorException& e)
    {
        delete _responseDecoder;
        throw e;
    }

    //
    // Create request encoder:
    //
    _requestEncoder = new CIMOperationRequestEncoder(
        _httpConnection, &_authenticator, showOutput);

    _responseDecoder->setEncoderQueue(_requestEncoder);

    _connected = true;
}

void CIMClientRep::_disconnect()
{
    if (_connected)
    {
        //
        // destroy response decoder
        //
        if (_responseDecoder)
        {
            delete _responseDecoder;
            _responseDecoder = 0;
        }

        //
        // Close the connection
        //
        if (_httpConnector)
        {
            _httpConnector->disconnect(_httpConnection);
            delete _httpConnection;
            _httpConnection = 0;
        }

        //
        // destroy request encoder
        //
        if (_requestEncoder)
        {
            delete _requestEncoder;
            _requestEncoder = 0;
        }

        if (_connectSSLContext)
        {
            delete _connectSSLContext;
            _connectSSLContext = 0;
        }

        _connected = false;
    }
}

void CIMClientRep::_reconnect()
{
    _disconnect();
    _authenticator.setRequestMessage(0);
    _connect();
}

void CIMClientRep::connect(
    const String& host,
    const Uint32 portNumber,
    const String& userName,
    const String& password
)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // If the host is empty, set hostName to "localhost"
    //
    String hostName = host;
    if (host == String::EMPTY)
    {
        hostName = "localhost";
    }

    //
    // Set authentication information
    //
    _authenticator.clear();

    if (String::equalNoCase(hostName, "localhost")) 
    {
        _authenticator.setAuthType(ClientAuthenticator::LOCAL);
    } 

    if (userName.size())
    {
        _authenticator.setUserName(userName);
    }

    if (password.size())
    {
        _authenticator.setPassword(password);
    }

    _connectSSLContext = 0;
    _connectHost = hostName;
    _connectPortNumber = portNumber;

    _connect();
}


void CIMClientRep::connect(
    const String& host,
    const Uint32 portNumber,
    const SSLContext& sslContext,
    const String& userName,
    const String& password
)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // If the host is empty, set hostName to "localhost"
    //
    String hostName = host;
    if (host == String::EMPTY)
    {
        hostName = "localhost";
    }

    //
    // Set authentication information
    //
    _authenticator.clear();

    if (String::equalNoCase(hostName, "localhost")) 
    {
        _authenticator.setAuthType(ClientAuthenticator::LOCAL);
    } 

    if (userName.size())
    {
        _authenticator.setUserName(userName);
    }

    if (password.size())
    {
        _authenticator.setPassword(password);
    }

    _connectSSLContext = new SSLContext(sslContext);
    _connectHost = hostName;
    _connectPortNumber = portNumber;


    try
    {
        _connect();
    }
    catch (Exception&)
    {
        delete _connectSSLContext;
        _connectSSLContext = 0;
        throw;
    }
}


void CIMClientRep::connectLocal()
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // Set authentication type
    //
    _authenticator.clear();
    _authenticator.setAuthType(ClientAuthenticator::LOCAL);

#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
    _connect();
#else

    try
    {
        //
        // Look up the WBEM HTTP port number for the local system
        //
        _connectPortNumber = System::lookupPort (WBEM_HTTP_SERVICE_NAME,
            WBEM_DEFAULT_HTTP_PORT);

        //
        //  Assign host
        //
        _connectHost.assign(_getLocalHostName());

        _connectSSLContext = 0;

        _connect();
    }
    catch(CannotConnectException &e)
    {
        //
        // Look up the WBEM HTTPS port number for the local system
        //
        _connectPortNumber = System::lookupPort (WBEM_HTTPS_SERVICE_NAME,
            WBEM_DEFAULT_HTTPS_PORT);

        //
        //  Assign host
        //
        _connectHost.assign(_getLocalHostName());

        //
        // Create SSLContext
        //
#ifdef PEGASUS_OS_OS400
#pragma convert(37)
        const char* env = getenv("PEGASUS_HOME");
#pragma convert(0)
        char pegasusHome[256] = {0};
        if (env != NULL && strlen(env) < 256)
        {
            strcpy(pegasusHome, env);
            EtoA(pegasusHome);
        }
#else
        const char* pegasusHome = getenv("PEGASUS_HOME");
#endif

        String certpath = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

        String randFile = String::EMPTY;

#ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
#endif

        try
        {
            _connectSSLContext =
                new SSLContext(certpath, verifyServerCertificate, randFile);
        }
        catch (SSLException &se)
        {
            throw se;
        }

        try
        {
            _connect();
        }
        catch (Exception&)
        {
            delete _connectSSLContext;
            _connectSSLContext = 0;
            throw;
        }
    }
#endif
}


void CIMClientRep::disconnect()
{
    _disconnect();
    _authenticator.clear();
}


// l10n start
AcceptLanguages CIMClientRep::getRequestAcceptLanguages() const
{
    return requestAcceptLanguages;
}

ContentLanguages CIMClientRep::getRequestContentLanguages() const
{
    return requestContentLanguages;
}

ContentLanguages CIMClientRep::getResponseContentLanguages() const
{
    return responseContentLanguages;
}

void CIMClientRep::setRequestAcceptLanguages(AcceptLanguages& langs)
{
    requestAcceptLanguages = langs;
}

void CIMClientRep::setRequestContentLanguages(ContentLanguages& langs)
{
    requestContentLanguages = langs;
}

void CIMClientRep::setRequestDefaultLanguages()
{
    requestAcceptLanguages = AcceptLanguages::getDefaultAcceptLanguages();
}

// l10n end

CIMClass CIMClientRep::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    CIMRequestMessage* request = new CIMGetClassRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_GET_CLASS_RESPONSE_MESSAGE);

    CIMGetClassResponseMessage* response =
        (CIMGetClassResponseMessage*)message;

    Destroyer<CIMGetClassResponseMessage> destroyer(response);

    return(response->cimClass);
}

CIMInstance CIMClientRep::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    CIMRequestMessage* request = new CIMGetInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_GET_INSTANCE_RESPONSE_MESSAGE);

    CIMGetInstanceResponseMessage* response =
        (CIMGetInstanceResponseMessage*)message;

    Destroyer<CIMGetInstanceResponseMessage> destroyer(response);

    return(response->cimInstance);
}

void CIMClientRep::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className
)
{
    CIMRequestMessage* request = new CIMDeleteClassRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_DELETE_CLASS_RESPONSE_MESSAGE);

    CIMDeleteClassResponseMessage* response =
        (CIMDeleteClassResponseMessage*)message;

    Destroyer<CIMDeleteClassResponseMessage> destroyer(response);
}

void CIMClientRep::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName
)
{
    CIMRequestMessage* request = new CIMDeleteInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_DELETE_INSTANCE_RESPONSE_MESSAGE);

    CIMDeleteInstanceResponseMessage* response =
        (CIMDeleteInstanceResponseMessage*)message;

    Destroyer<CIMDeleteInstanceResponseMessage> destroyer(response);
}

void CIMClientRep::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass
)
{
    CIMRequestMessage* request = new CIMCreateClassRequestMessage(
        String::EMPTY,
        nameSpace,
        newClass,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_CREATE_CLASS_RESPONSE_MESSAGE);

    CIMCreateClassResponseMessage* response =
        (CIMCreateClassResponseMessage*)message;

    Destroyer<CIMCreateClassResponseMessage> destroyer(response);
}

CIMObjectPath CIMClientRep::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance
)
{
    CIMRequestMessage* request = new CIMCreateInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        newInstance,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_CREATE_INSTANCE_RESPONSE_MESSAGE);

    CIMCreateInstanceResponseMessage* response =
        (CIMCreateInstanceResponseMessage*)message;

    Destroyer<CIMCreateInstanceResponseMessage> destroyer(response);

    return(response->instanceName);
}

void CIMClientRep::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass
)
{
    CIMRequestMessage* request = new CIMModifyClassRequestMessage(
        String::EMPTY,
        nameSpace,
        modifiedClass,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_MODIFY_CLASS_RESPONSE_MESSAGE);

    CIMModifyClassResponseMessage* response =
        (CIMModifyClassResponseMessage*)message;

    Destroyer<CIMModifyClassResponseMessage> destroyer(response);
}

void CIMClientRep::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList
)
{
    CIMRequestMessage* request = new CIMModifyInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        modifiedInstance,
        includeQualifiers,
        propertyList,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE);

    CIMModifyInstanceResponseMessage* response =
        (CIMModifyInstanceResponseMessage*)message;

    Destroyer<CIMModifyInstanceResponseMessage> destroyer(response);
}

Array<CIMClass> CIMClientRep::enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin
)
{
    CIMRequestMessage* request = new CIMEnumerateClassesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE);

    CIMEnumerateClassesResponseMessage* response =
        (CIMEnumerateClassesResponseMessage*)message;

    Destroyer<CIMEnumerateClassesResponseMessage> destroyer(response);

    return(response->cimClasses);
}

Array<CIMName> CIMClientRep::enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance
)
{
    CIMRequestMessage* request = new CIMEnumerateClassNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        deepInheritance,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE);

    CIMEnumerateClassNamesResponseMessage* response =
        (CIMEnumerateClassNamesResponseMessage*)message;

    Destroyer<CIMEnumerateClassNamesResponseMessage> destroyer(response);

    // Temporary code until internal structures use CIMName instead of String
    Array<CIMName> classNameArray;
    classNameArray.reserveCapacity(response->classNames.size());
    for (Uint32 i=0; i<response->classNames.size(); i++)
    {
        classNameArray.append(response->classNames[i]);
    }
    return(classNameArray);
}

Array<CIMInstance> CIMClientRep::enumerateInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    CIMRequestMessage* request = new CIMEnumerateInstancesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE);

    CIMEnumerateInstancesResponseMessage* response =
        (CIMEnumerateInstancesResponseMessage*)message;

    Destroyer<CIMEnumerateInstancesResponseMessage> destroyer(response);

    return(response->cimNamedInstances);
}

Array<CIMObjectPath> CIMClientRep::enumerateInstanceNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className
)
{
    CIMRequestMessage* request = new CIMEnumerateInstanceNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE);

    CIMEnumerateInstanceNamesResponseMessage* response =
        (CIMEnumerateInstanceNamesResponseMessage*)message;

    Destroyer<CIMEnumerateInstanceNamesResponseMessage> destroyer(response);

    return(response->instanceNames);
}

Array<CIMObject> CIMClientRep::execQuery(
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query
)
{
    CIMRequestMessage* request = new CIMExecQueryRequestMessage(
        String::EMPTY,
        nameSpace,
        queryLanguage,
        query,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_EXEC_QUERY_RESPONSE_MESSAGE);

    CIMExecQueryResponseMessage* response =
        (CIMExecQueryResponseMessage*)message;

    Destroyer<CIMExecQueryResponseMessage> destroyer(response);

    return(response->cimObjects);
}

Array<CIMObject> CIMClientRep::associators(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    CIMRequestMessage* request = new CIMAssociatorsRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ASSOCIATORS_RESPONSE_MESSAGE);

    CIMAssociatorsResponseMessage* response =
        (CIMAssociatorsResponseMessage*)message;

    Destroyer<CIMAssociatorsResponseMessage> destroyer(response);

    return(response->cimObjects);
}

Array<CIMObjectPath> CIMClientRep::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole
)
{
    CIMRequestMessage* request = new CIMAssociatorNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE);

    CIMAssociatorNamesResponseMessage* response =
        (CIMAssociatorNamesResponseMessage*)message;

    Destroyer<CIMAssociatorNamesResponseMessage> destroyer(response);

    return(response->objectNames);
}

Array<CIMObject> CIMClientRep::references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    CIMRequestMessage* request = new CIMReferencesRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_REFERENCES_RESPONSE_MESSAGE);

    CIMReferencesResponseMessage* response =
        (CIMReferencesResponseMessage*)message;

    Destroyer<CIMReferencesResponseMessage> destroyer(response);

    return(response->cimObjects);
}

Array<CIMObjectPath> CIMClientRep::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role
)
{
    CIMRequestMessage* request = new CIMReferenceNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        resultClass,
        role,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_REFERENCE_NAMES_RESPONSE_MESSAGE);

    CIMReferenceNamesResponseMessage* response =
        (CIMReferenceNamesResponseMessage*)message;

    Destroyer<CIMReferenceNamesResponseMessage> destroyer(response);

    return(response->objectNames);
}

CIMValue CIMClientRep::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName
)
{
    CIMRequestMessage* request = new CIMGetPropertyRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        propertyName,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_GET_PROPERTY_RESPONSE_MESSAGE);

    CIMGetPropertyResponseMessage* response =
        (CIMGetPropertyResponseMessage*)message;

    Destroyer<CIMGetPropertyResponseMessage> destroyer(response);

    return(response->value);
}

void CIMClientRep::setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue
)
{
    CIMRequestMessage* request = new CIMSetPropertyRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        propertyName,
        newValue,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_SET_PROPERTY_RESPONSE_MESSAGE);

    CIMSetPropertyResponseMessage* response =
        (CIMSetPropertyResponseMessage*)message;

    Destroyer<CIMSetPropertyResponseMessage> destroyer(response);
}

CIMQualifierDecl CIMClientRep::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
)
{
    CIMRequestMessage* request = new CIMGetQualifierRequestMessage(
        String::EMPTY,
        nameSpace,
        qualifierName,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_GET_QUALIFIER_RESPONSE_MESSAGE);

    CIMGetQualifierResponseMessage* response =
        (CIMGetQualifierResponseMessage*)message;

    Destroyer<CIMGetQualifierResponseMessage> destroyer(response);

    return(response->cimQualifierDecl);
}

void CIMClientRep::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration
)
{
    CIMRequestMessage* request = new CIMSetQualifierRequestMessage(
        String::EMPTY,
        nameSpace,
        qualifierDeclaration,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_SET_QUALIFIER_RESPONSE_MESSAGE);

    CIMSetQualifierResponseMessage* response =
        (CIMSetQualifierResponseMessage*)message;

    Destroyer<CIMSetQualifierResponseMessage> destroyer(response);
}

void CIMClientRep::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
)
{
    CIMRequestMessage* request = new CIMDeleteQualifierRequestMessage(
        String::EMPTY,
        nameSpace,
        qualifierName,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE);

    CIMDeleteQualifierResponseMessage* response =
        (CIMDeleteQualifierResponseMessage*)message;

    Destroyer<CIMDeleteQualifierResponseMessage> destroyer(response);
}

Array<CIMQualifierDecl> CIMClientRep::enumerateQualifiers(
    const CIMNamespaceName& nameSpace
)
{
    CIMRequestMessage* request = new CIMEnumerateQualifiersRequestMessage(
        String::EMPTY,
        nameSpace,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE);

    CIMEnumerateQualifiersResponseMessage* response =
        (CIMEnumerateQualifiersResponseMessage*)message;

    Destroyer<CIMEnumerateQualifiersResponseMessage> destroyer(response);

    return(response->qualifierDeclarations);
}

CIMValue CIMClientRep::invokeMethod(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters
)
{
    // ATTN-RK-P2-20020301: Does it make sense to have a nameSpace parameter
    // when the namespace should already be included in the instanceName?
    // ATTN-RK-P3-20020301: Do we need to make sure the caller didn't specify
    // a host name in the instanceName?

    CIMRequestMessage* request = new CIMInvokeMethodRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        methodName,
        inParameters,
        QueueIdStack());

    Message* message = _doRequest(request, CIM_INVOKE_METHOD_RESPONSE_MESSAGE);

    CIMInvokeMethodResponseMessage* response =
        (CIMInvokeMethodResponseMessage*)message;

    Destroyer<CIMInvokeMethodResponseMessage> destroyer(response);

    outParameters = response->outParameters;

    return(response->retValue);
}

Message* CIMClientRep::_doRequest(
    CIMRequestMessage * request,
    const Uint32 expectedResponseMessageType
)
{
    if (!_connected)
    {
        delete request;
        throw NotConnectedException();
    }

    String messageId = XmlWriter::getNextMessageId();
    const_cast<String &>(request->messageId) = messageId;

    _authenticator.setRequestMessage(0);

    // ATTN-RK-P2-20020416: We should probably clear out the queue first.
    PEGASUS_ASSERT(getCount() == 0);  // Shouldn't be any messages in our queue

    //
    //  Set HTTP method in request to POST
    //
    //Bug 478/418 - Change this to do post call, not mpost
    request->setHttpMethod (HTTP_METHOD__POST);

// l10n
    // Set the Accept-Languages and Content-Languages into
    // the request message
    request->acceptLanguages = requestAcceptLanguages;
    request->contentLanguages = requestContentLanguages;

    // Sending a new request, so clear out the response Content-Languages
    responseContentLanguages = ContentLanguages::EMPTY;

    _requestEncoder->enqueue(request);

    Uint64 startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    Uint64 nowMilliseconds = startMilliseconds;
    Uint64 stopMilliseconds = nowMilliseconds + _timeoutMilliseconds;

    while (nowMilliseconds < stopMilliseconds)
    {
        //
        // Wait until the timeout expires or an event occurs:
        //

       _monitor->run(Uint32(stopMilliseconds - nowMilliseconds));

        //
        // Check to see if incoming queue has a message
        //

        Message* response = dequeue();

        if (response)
        {
            // Shouldn't be any more messages in our queue
            PEGASUS_ASSERT(getCount() == 0);

            //
            //  Future:  If M-POST is used and HTTP response is 501 Not
            //  Implemented or 510 Not Extended, retry with POST method
            //

            if (response->getType() == CLIENT_EXCEPTION_MESSAGE)
            {
                Exception* clientException =
                    ((ClientExceptionMessage*)response)->clientException;
                delete response;

                Destroyer<Exception> d(clientException);

                //
                // Determine and throw the specific class of client exception
                //

                CIMClientMalformedHTTPException* malformedHTTPException =
                    dynamic_cast<CIMClientMalformedHTTPException*>(
                        clientException);
                if (malformedHTTPException)
                {
                    throw *malformedHTTPException;
                }

                CIMClientHTTPErrorException* httpErrorException =
                    dynamic_cast<CIMClientHTTPErrorException*>(
                        clientException);
                if (httpErrorException)
                {
                    throw *httpErrorException;
                }

                CIMClientXmlException* xmlException =
                    dynamic_cast<CIMClientXmlException*>(clientException);
                if (xmlException)
                {
                    throw *xmlException;
                }

                CIMClientResponseException* responseException =
                    dynamic_cast<CIMClientResponseException*>(clientException);
                if (responseException)
                {
                    throw *responseException;
                }

                throw *clientException;
            }
            else if (response->getType() == expectedResponseMessageType)
            {
                CIMResponseMessage* cimResponse = (CIMResponseMessage*)response;

                if (cimResponse->messageId != messageId)
                {
                    // l10n

                    // CIMClientResponseException responseException(
                    //   String("Mismatched response message ID:  Got \"") +
                    //    cimResponse->messageId + "\", expected \"" +
                    //    messageId + "\".");

                    MessageLoaderParms mlParms(
                        "Client.CIMClient.MISMATCHED_RESPONSE",
                        "Mismatched response message ID:  Got \"$0\", "
                            "expected \"$1\".",
                        cimResponse->messageId, messageId);
                    String mlString(MessageLoader::getMessage(mlParms));

                    CIMClientResponseException responseException(mlString);

                    delete response;
                    throw responseException;
                }

// l10n
                // Get the Content-Languages from the response
                responseContentLanguages = cimResponse->contentLanguages;

                if (cimResponse->cimException.getCode() != CIM_ERR_SUCCESS)
                {
                    CIMException cimException(
                        cimResponse->cimException.getCode(),
                        cimResponse->cimException.getMessage());
                    delete response;
                    throw cimException;
                }
                return response;
            }
            else
            {
                // l10n

                // CIMClientResponseException responseException(
                //   "Mismatched response message type.");

                MessageLoaderParms mlParms(
                  "Client.CIMOperationResponseDecoder.MISMATCHED_RESPONSE_TYPE",
                  "Mismatched response message type.");
                String mlString(MessageLoader::getMessage(mlParms));

                CIMClientResponseException responseException(mlString);

                delete response;
                throw responseException;
            }
        }

        nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
        pegasus_yield();
    }

    //
    // Reconnect to reset the connection (disregard late response)
    //
    try
    {
        _reconnect();
    }
    catch (...)
    {
    }

    //
    // Throw timed out exception:
    //
    throw ConnectionTimeoutException();
}

String CIMClientRep::_getLocalHostName()
{
    static String hostname = "localhost";

    return hostname;
}

PEGASUS_NAMESPACE_END
