//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
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
// Author: Sean Keenan (sean.keenan@hp.com)
//
// Modified By: 
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ConfigVms_h
#define Pegasus_ConfigVms_h

#include <stddef>
#include <stdio.h>

#pragma message disable codcauunr
#pragma message disable unscomzer
#pragma message disable unrintunr
#pragma message disable longextern
#pragma message disable missingreturn

#ifndef PEGASUS_OS_VMS
#define PEGASUS_OS_VMS
#endif

#define PEGASUS_EXPORT /* empty */
#define PEGASUS_IMPORT /* empty */
#define PEGASUS_IOS_BINARY /* empty */
#define PEGASUS_OR_IOS_BINARY /* empty */
#define PEGASUS_COMMON_LINKAGE /* empty */
#define PEGASUS_REPOSITORY_LINKAGE /* empty */
#define PEGASUS_PROTOCOL_LINKAGE /* empty */
#define PEGASUS_SERVER_LINKAGE /* empty */
#define PEGASUS_COMPILER_LINKAGE /* empty */
#define PEGASUS_GETOOPT_LINKAGE /* empty */
#define PEGASUS_PROVIDER_LINKAGE /* empty */
#define PEGASUS_CMDLINE_LINKAGE /* empty */
#define PEGASUS_HANDLER_LINKAGE /* empty */
#define PEGASUS_CIMOM_LINKAGE /* empty */
#define PegasusCreateProvider PEGASUSCREATEPROVIDER
#define PegasusCreateProviderAdapter PEGASUSCREATEPROVIDERADAPTER
#define PegasusCreateProviderManager PEGASUSCREATEPROVIDERMANAGER
#define callme CALLME
#endif  /* Pegasus_ConfigVms_h */
