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
// Author: Warren Otsuka (warren.otsuka@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_BUILD_DATE
#define PLATFORM_BUILD_DATE __DATE__
#endif

#define PLATFORM_PRODUCT_NAME    PEGASUS_NAME
#define PLATFORM_PRODUCT_VERSION PEGASUS_VERSION
#define PLATFORM_PRODUCT_STATUS  PEGASUS_STATUS

#ifndef PLATFORM_COMPONENT_NAME
#define PLATFORM_COMPONENT_NAME "component"
#endif

#ifndef PLATFORM_COMPONENT_VERSION 
#define PLATFORM_COMPONENT_VERSION PLATFORM_PRODUCT_VERSION 
#endif

#ifndef PLATFORM_COMPONENT_STATUS
#define PLATFORM_COMPONENT_STATUS PLATFORM_PRODUCT_STATUS
#endif

#ifndef COMPONENT_BUILD_DATE
#define COMPONENT_BUILD_DATE __DATE__
#endif
