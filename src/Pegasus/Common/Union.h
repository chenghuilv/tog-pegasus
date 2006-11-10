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

#ifndef Pegasus_Union_h
#define Pegasus_Union_h

#include <Pegasus/Common/Config.h>

PEGASUS_NAMESPACE_BEGIN

struct StringRep;

/** This union is used to represent the values of properties, qualifiers,
    method return values, and method arguments. All of the types
    defined in CIMType.h are represented by a Union. The
    Union is used as the the basis for the CIMValue implementation.
*/
union Union
{
    Boolean _booleanValue;
    Uint8 _uint8Value;
    Sint8 _sint8Value;
    Uint16 _uint16Value;
    Sint16 _sint16Value;
    Uint32 _uint32Value;
    Sint32 _sint32Value;
    Uint64 _uint64Value;
    Sint64 _sint64Value;
    Real32 _real32Value;
    Real64 _real64Value;
    Uint16 _char16Value;
    char _stringValue[sizeof(void*)];
    char _dateTimeValue[sizeof(void*)];
    char _referenceValue[sizeof(void*)];
    char _objectValue[sizeof(void*)];
#ifdef PEGASUS_EMBEDDED_INSTANCE_SUPPORT
    char _instanceValue[sizeof(void*)];
#endif // PEGASUS_EMBEDDED_INSTANCE_SUPPORT

    char _booleanArray[sizeof(void*)];
    char _uint8Array[sizeof(void*)];
    char _sint8Array[sizeof(void*)];
    char _uint16Array[sizeof(void*)];
    char _sint16Array[sizeof(void*)];
    char _uint32Array[sizeof(void*)];
    char _sint32Array[sizeof(void*)];
    char _uint64Array[sizeof(void*)];
    char _sint64Array[sizeof(void*)];
    char _real32Array[sizeof(void*)];
    char _real64Array[sizeof(void*)];
    char _char16Array[sizeof(void*)];
    char _stringArray[sizeof(void*)];
    char _dateTimeArray[sizeof(void*)];
    char _referenceArray[sizeof(void*)];
    char _objectArray[sizeof(void*)];
#ifdef PEGASUS_EMBEDDED_INSTANCE_SUPPORT
    char _instanceArray[sizeof(void*)];
#endif // PEGASUS_EMBEDDED_INSTANCE_SUPPORT
    void* _voidPtr;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Union_h */
