//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
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
// Author: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//
// Modified By: Yi Zhou, Hewlett-Packard Company (yi_zhou@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "snmpDeliverTrap_stub.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

void snmpDeliverTrap_stub::deliverTrap(
        const String& trapOid,
        const String& securityName, 
        const String& targetHost, 
        const Uint16& targetHostFormat, 
        const String& otherTargetHostFormat, 
        const Uint32& portNumber,
        const Uint16& snmpVersion, 
        const String& engineID,
        Array<String>& vbOids,
        Array<String>& vbTypes,
        Array<String>& vbValues)
{
    //ATTN: Code to implement SNMP specific delivery mechanism.
}

PEGASUS_NAMESPACE_END
