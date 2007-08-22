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

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/HTTPMessage.h>

#include <Pegasus/Config/ConfigManager.h>

#include "LocalAuthenticationHandler.h"
#include "BasicAuthenticationHandler.h"
#include "AuthenticationManager.h"

#include <Pegasus/Common/AutoPtr.h>

#ifdef PEGASUS_KERBEROS_AUTHENTICATION
#include "KerberosAuthenticationHandler.h"
#endif


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//
// Constructor
//
AuthenticationManager::AuthenticationManager()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::AuthenticationManager()");

    //
    // get authentication handlers
    //
    _localAuthHandler = _getLocalAuthHandler();

    _httpAuthHandler = _getHttpAuthHandler();

    PEG_METHOD_EXIT();
}

//
// Destructor
//
AuthenticationManager::~AuthenticationManager()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::~AuthenticationManager()");

    //
    // delete authentication handlers
    //
    delete _localAuthHandler;
    delete _httpAuthHandler;

    PEG_METHOD_EXIT();
}

Boolean AuthenticationManager::isRemotePrivilegedUserAccessAllowed(
        String & userName)
{
    //
    // Reject access if the user is privileged and remote privileged user
    // access is not enabled.
    //
    if (!ConfigManager::parseBooleanValue(ConfigManager::getInstance()->
            getCurrentValue("enableRemotePrivilegedUserAccess"))
        && System::isPrivilegedUser(userName))
    {
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL2,
            "Authentication failed for user '%s' because "
            "enableRemotePrivilegedUserAccess is not set to 'true'.",
            (const char*) userName.getCString()));
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            "Security.Authentication.BasicAuthenticationHandler."
                "PRIVILEGED_ACCESS_DISABLED",
            "Authentication failed for user '$0' because "
                "enableRemotePrivilegedUserAccess is not set to 'true'.",
            userName);
        return false;
    }
    return true;
}

//
// Perform http authentication
//
Boolean AuthenticationManager::performHttpAuthentication(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::performHttpAuthentication()");

    String authType;
    String cookie;

    Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
        "AuthenticationManager:: performHttpAuthentication - "
            "Authority Header: $0",
        authHeader);

    //
    // Parse the HTTP authentication header for authentication information
    //
    if ( !HTTPMessage::parseHttpAuthHeader(authHeader, authType, cookie) )
    {
        PEG_METHOD_EXIT();
        return false;
    }

    Boolean authenticated = false;

    //
    // Check the authenticationinformation and do the authentication
    //
    if ( String::equalNoCase(authType, "Basic") &&
         String::equalNoCase(_httpAuthType, "Basic") )
    {
        authenticated = _httpAuthHandler->authenticate(cookie, authInfo);
    }
#ifdef PEGASUS_KERBEROS_AUTHENTICATION
    else if ( String::equalNoCase(authType, "Negotiate") &&
              String::equalNoCase(_httpAuthType, "Kerberos") )
    {
        authenticated = _httpAuthHandler->authenticate(cookie, authInfo);
    }
#endif
    // FUTURE: Add code to check for "Digest" when digest
    // authentication is implemented.

    if ( authenticated )
    {
        authInfo->setAuthType(authType);
    }

    PEG_METHOD_EXIT();

    return authenticated;
}

//
// Perform pegasus sepcific local authentication
//
Boolean AuthenticationManager::performPegasusAuthentication(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::performPegasusAuthentication()");

    Boolean authenticated = false;

    String authType;
    String userName;
    String cookie;

    Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
        "AuthenticationManager:: performPegasusAuthentication - "
            "Authority Header: $0",
        authHeader);

    //
    // Parse the pegasus authentication header authentication information
    //
    if ( !HTTPMessage::parseLocalAuthHeader(authHeader,
              authType, userName, cookie) )
    {
        PEG_METHOD_EXIT();
        return false;
    }

    // The HTTPAuthenticatorDelegator ensures only local authentication
    // requests get here.
    PEGASUS_ASSERT(authType == "Local");

    authenticated =
        _localAuthHandler->authenticate(cookie, authInfo);

    if ( authenticated )
    {
        authInfo->setAuthType(authType);
    }

    PEG_METHOD_EXIT();

    return authenticated;
}

//
// Validate user.
//
Boolean AuthenticationManager::validateUserForHttpAuth (const String& userName)
{
    return _httpAuthHandler->validateUser(userName);
}

//
// Get pegasus/local authentication response header
//
String AuthenticationManager::getPegasusAuthResponseHeader(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::getPegasusAuthResponseHeader()");

    String respHeader;

    String authType;
    String userName;
    String cookie;

    //
    // Parse the pegasus authentication header authentication information
    //
    if ( !HTTPMessage::parseLocalAuthHeader(authHeader, 
              authType, userName, cookie) )
    {
        PEG_METHOD_EXIT();
        return respHeader;
    }

    //
    // User name can not be empty
    //
    if (String::equal(userName, String::EMPTY))
    {
        PEG_METHOD_EXIT();
        return respHeader;
    }

    respHeader =
        _localAuthHandler->getAuthResponseHeader(authType, userName, authInfo);

    PEG_METHOD_EXIT();

    return respHeader;

}

//
// Get HTTP authentication response header
//
#ifdef PEGASUS_KERBEROS_AUTHENTICATION
String AuthenticationManager::getHttpAuthResponseHeader(
    AuthenticationInfo* authInfo)
#else
String AuthenticationManager::getHttpAuthResponseHeader()
#endif
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::getHttpAuthResponseHeader()");

#ifdef PEGASUS_KERBEROS_AUTHENTICATION
    String respHeader = _httpAuthHandler->getAuthResponseHeader(
        String::EMPTY, String::EMPTY, authInfo);
#else
    String respHeader = _httpAuthHandler->getAuthResponseHeader();
#endif

    PEG_METHOD_EXIT();

    return respHeader;
}

//
// Get local authentication handler
//
Authenticator* AuthenticationManager::_getLocalAuthHandler()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::_getLocalAuthHandler()");

    PEG_METHOD_EXIT();
    //
    // create and return a local authentication handler.
    //
    return new LocalAuthenticationHandler();
}


//
// Get Http authentication handler
//
Authenticator* AuthenticationManager::_getHttpAuthHandler()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::_getHttpAuthHandler()");
    AutoPtr<Authenticator> handler;

    //
    // get the configured authentication type
    //
    AutoPtr<ConfigManager> configManager(ConfigManager::getInstance());

    _httpAuthType = configManager->getCurrentValue("httpAuthType");
    configManager.release();
    //
    // create a authentication handler.
    //
    if ( String::equalNoCase(_httpAuthType, "Basic") )
    {
        handler.reset((Authenticator* ) new BasicAuthenticationHandler( ));
    }
#ifdef PEGASUS_KERBEROS_AUTHENTICATION
    else if ( String::equalNoCase(_httpAuthType, "Kerberos") )
    {
        handler.reset((Authenticator*) new KerberosAuthenticationHandler());
        AutoPtr<KerberosAuthenticationHandler> kerberosHandler(
            (KerberosAuthenticationHandler *)handler.get());
        int itFailed = kerberosHandler->initialize();
        kerberosHandler.release();
        if (itFailed)
        {
            if (handler.get())
            {
                handler.reset(0);
            }
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                "Security.Authentication.AuthenticationManager."
                    "AUTHENTICATION_HANDLER_KERBEROS_FAILED_TO_INITIALIZE",
                "CIMOM server authentication handler for Kerberos failed to "
                    "initialize properly.");
            MessageLoaderParms parms(
                "Security.Authentication.AuthenticationManager."
                    "AUTHENTICATION_HANDLER_KERBEROS_FAILED_TO_INITIALIZE",
                "CIMOM server authentication handler for Kerberos failed to "
                    "initialize properly.");
            throw Exception(parms);
        }
    }
#endif
    // FUTURE: uncomment these line when Digest authentication
    // is implemented.
    //
    //else if (String::equalNoCase(_httpAuthType, "Digest"))
    //{
    //    handler = (Authenticator* ) new DigestAuthenticationHandler( );
    //}
    else
    {
        //
        // This should never happen. Gets here only if Security Config
        // property owner has not validated the configured http auth type.
        //
        PEGASUS_ASSERT(0);
    }

    PEG_METHOD_EXIT();
    return handler.release();
}

PEGASUS_NAMESPACE_END
