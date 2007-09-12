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

#ifndef Pegasus_DefaultPropertyTablePase_h
#define Pegasus_DefaultPropertyTablePase_h

    {"httpPort", "", IS_STATIC, 0, 0, IS_VISIBLE},
    {"httpsPort", "", IS_STATIC, 0, 0, IS_VISIBLE},
    {"enableHttpConnection", "true", IS_STATIC, 0, 0, IS_VISIBLE},
    {"enableHttpsConnection", "true", IS_STATIC, 0, 0, IS_VISIBLE},
    {"daemon", "false", IS_STATIC, 0, 0, IS_VISIBLE},
    {"enableBinaryRepository", "false", IS_STATIC, 0, 0, IS_VISIBLE},
    {"enableAssociationTraversal", "true", IS_STATIC, 0, 0, IS_VISIBLE},
    {"enableIndicationService", "true", IS_STATIC, 0, 0, IS_VISIBLE},
    {"forceProviderProcesses", "true", IS_STATIC, 0, 0, IS_VISIBLE},
    {"maxProviderProcesses","0", IS_STATIC, 0, 0, IS_VISIBLE},
# ifdef PEGASUS_ENABLE_AUDIT_LOGGER
    {"enableAuditLog", "false", IS_DYNAMIC, 0, 0, IS_VISIBLE},
# endif
    {"home", "/QOpenSys/QIBM/UserData/UME/Pegasus", IS_STATIC, 
        0, 0, IS_VISIBLE},
    {"slp", "true", IS_STATIC, 0, 0, IS_VISIBLE},
    
#endif /* Pegasus_DefaultPropertyTablePase_h */
