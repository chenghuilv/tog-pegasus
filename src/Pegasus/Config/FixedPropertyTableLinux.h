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
// Modified By: Heather Sterling, IBM (hsterl@us.ibm.com)
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
    {"enableSSLClientVerification", "false"},
    {"httpAuthType",        "Basic"},
    {"repositoryIsDefaultInstanceProvider", "false"},
    {"usePAMAuthentication", "true"},
#endif
#ifdef PEGASUS_USE_RELEASE_DIRS
    {"traceFilePath",       "/var/cache/pegasus/cimserver.trc"},
    {"logdir",              "/var/log/pegasus"},
    {"tempLocalAuthDir",    "/var/cache/pegasus/localauth"},
    {"passwordFilePath",    "/etc/pegasus/cimserver.passwd"},
    {"sslCertificateFilePath", "/etc/pegasus/server.pem"},
    {"sslKeyFilePath",      "/etc/pegasus/file.pem"},
    {"sslTrustFilePath",      "/etc/pegasus/client.pem"},
    {"repositoryDir",       "/var/lib/pegasus/repository"},
    {"providerDir",         "/usr/lib/pegasus/providers:/usr/lib/openwbem/cimpiproviders:/usr/lib/cmpi"},
#endif
#if !defined(PEGASUS_USE_RELEASE_CONFIG_OPTIONS) && !defined(PEGASUS_USE_RELEASE_DIRS)
    {"bogus", "MyBogusValue"} // Remove this line if others are added
#endif
