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

#ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"httpPort",            "5988"},
    {"httpsPort",           "5989"},
    {"home",                ""},
    {"daemon",              "true"},
    {"install",             "false"},
    {"remove",              "false"},
    {"slp",                 "false"},
    {"enableAuthentication", "true"},
    {"sslClientVerificationMode", "disabled"},
    {"httpAuthType",        "Basic"},
    {"repositoryIsDefaultInstanceProvider", "false"},
#endif
#if defined(PEGASUS_USE_RELEASE_DIRS) && !defined(PEGASUS_OS_LSB)
    {"traceFilePath",       "/var/cache/pegasus/cimserver.trc"},
    {"logdir",              "/var/log/pegasus"},
    {"tempLocalAuthDir",    "/var/cache/pegasus/localauth"},
    {"passwordFilePath",    "/etc/pegasus/cimserver.passwd"},
    {"sslCertificateFilePath", "/etc/pegasus/server.pem"},
    {"sslKeyFilePath",      "/etc/pegasus/file.pem"},
    {"sslTrustStore",      "/etc/pegasus/client.pem"},
    {"exportSSLTrustStore", "/etc/pegasus/indication_trust.pem"},
    {"repositoryDir",       "/var/lib/pegasus/repository"},
    {"providerDir",         "/usr/lib/pegasus/providers"},
#endif
#if defined(PEGASUS_OS_LSB)
    {"traceFilePath",       "/var/opt/lsb-pegasus/cache/cimserver.trc"},
    {"logdir",              "/var/opt/lsb-pegasus/log"},
    {"tempLocalAuthDir",    "/var/opt/lsb-pegasus/cache/localauth"},
    {"passwordFilePath",    "/etc/opt/lsb-pegasus/cimserver.passwd"},
    {"sslCertificateFilePath", "/etc/opt/lsb-pegasus/server.pem"},
    {"sslKeyFilePath",      "/etc/opt/lsb-pegasus/file.pem"},
    {"sslTrustStore",      "/etc/opt/lsb-pegasus/client.pem"},
    {"exportSSLTrustStore", "/etc/opt/lsb-pegasus/indication_trust.pem"},
    {"repositoryDir",       "/var/opt/lsb-pegasus/repository"},
    {"providerDir",         "/opt/lsb-pegasus/providers"},
#endif
#if !defined(PEGASUS_USE_RELEASE_CONFIG_OPTIONS) && !defined(PEGASUS_USE_RELEASE_DIRS)
    {"bogus", "MyBogusValue"} // Remove this line if others are added
#endif
