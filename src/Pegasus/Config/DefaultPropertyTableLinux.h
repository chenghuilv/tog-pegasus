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
// Author: Warren Otsuka, Hewlett-Packard Company (warren.otsuka@hp.com)
//
// Modified By: 
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_DefaultPropertyTableLinux_h
#define Pegasus_DefaultPropertyTableLinux_h


#ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"logLevel", "SEVERE", 0, 0, 0, 1},
    {"httpPort", "5988", 0, 0, 0, 1},
    {"httpsPort", "5989", 0, 0, 0, 1},
    {"enableHttpConnection", "false", 0, 0, 0, 1},
    {"enableHttpsConnection", "true", 0, 0, 0, 1},
    {"home", "", 0, 0, 0, 1},
    {"daemon", "true", 0, 0, 0, 0},
    {"install", "false", 0, 0, 0, 1},
    {"remove", "false", 0, 0, 0, 1},
    {"slp", "false", 0, 0, 0, 1},
    {"enableAssociationTraversal", "true", 0, 0, 0, 1},
    {"enableAuthentication", "true", 0, 0, 0, 1},
    {"enableNamespaceAuthorization", "false", 0, 0, 0, 1},
    {"enableIndicationService", "true", 0, 0, 0, 1},
    {"enableSubscriptionsForNonprivilegedUsers", "false", 0, 0, 0, 1},
    {"enableRemotePrivilegedUserAccess", "true", 0, 0, 0, 1},
    // Removed because unresolved PEP 66 KS{"maximumEnumerationBreadth", "50", 0, 0, 0},
    {"tempLocalAuthDir", PEGASUS_LOCAL_AUTH_DIR, 0, 0, 0, 1},
    {"sslClientVerificationMode", "disabled", 0, 0, 0, 1},
    {"enableProviderProcesses", "false", 0, 0, 0, 1}
#else
    {"logLevel", "INFORMATION", 0, 0, 0, 1},
    {"httpPort", "5988", 0, 0, 0, 1},
    {"httpsPort", "5989", 0, 0, 0, 1},
    {"enableHttpConnection", "true", 0, 0, 0, 1},
    {"enableHttpsConnection", "true", 0, 0, 0, 1},
    {"home", "./", 0, 0, 0, 1},
    {"daemon", "true", 0, 0, 0, 1},
    {"install", "false", 0, 0, 0, 1},
    {"remove", "false", 0, 0, 0, 1},
    {"slp", "false", 0, 0, 0, 1},
    {"enableAssociationTraversal", "true", 0, 0, 0, 1},
    {"enableAuthentication", "false", 0, 0, 0, 1},
    {"enableNamespaceAuthorization", "false", 0, 0, 0, 1},
    {"enableIndicationService", "true", 0, 0, 0, 1},
    {"enableSubscriptionsForNonprivilegedUsers", "true", 0, 0, 0, 1},
    {"enableRemotePrivilegedUserAccess", "true", 0, 0, 0, 1},
    {"maximumEnumerationBreadth", "50", 0, 0, 0, 1},
    {"tempLocalAuthDir", PEGASUS_LOCAL_AUTH_DIR, 0, 0, 0, 1},
    {"sslClientVerificationMode", "disabled", 0, 0, 0, 1},
    {"enableProviderProcesses", "false", 0, 0, 0, 1}
#endif


#endif /* Pegasus_DefaultPropertyTableLinux_h */
