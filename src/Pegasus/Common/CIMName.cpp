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

#include <cctype>
#include "CIMName.h"
#include "CommonUTF.h"
#include "CharSet.h"

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMName
//
////////////////////////////////////////////////////////////////////////////////

#define PEGASUS_ARRAY_T CIMName
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T


/**
    Checks if a given character string consists of ASCII only and
    legal characters for a CIMName (i.e. letter, numbers and underscore)
    The first character has to be a letter or underscore
    @param str character string to be checked
    @return  0 in case non-legal ASCII character was found
            >0 length of the character string str
*/
static Uint32 _legalASCII(const char* str)
{
    const Uint8* p = (const Uint8*)str;

    if (!CharSet::isAlphaUnder(*p++))
        return 0;

    while (*p)
    {
        if (!CharSet::isAlNumUnder(*p++))
            return 0;
    }

    return Uint32((char*)p - str);
}

CIMName::CIMName(const String& name) : cimName(name)
{
    if (!legal(name))
        throw InvalidNameException(name);
}

CIMName::CIMName(const char* name)
{
    Uint32 size = _legalASCII(name);

    if (size == 0)
    {
        cimName.assign(name);

        if (!legal(cimName))
            throw InvalidNameException(name);        
    }
    else
    {
        AssignASCII(cimName, name, size);
    }
}

CIMName& CIMName::operator=(const String& name)
{
    if (!legal(name))
        throw InvalidNameException(name);

    cimName=name;
    return *this;
}

CIMName& CIMName::operator=(const char* name)
{
    Uint32 size = _legalASCII(name);

    if (size == 0)
    {
        String tmp(name);

        if (!legal(tmp))
            throw InvalidNameException(name);
        
        cimName.assign(tmp);
    }
    else
    {
        AssignASCII(cimName, name, size);
    }
    return *this;
}

Boolean CIMName::legal(const String& name)
{
    // Check first character.

    const Uint16* p = (const Uint16*)name.getChar16Data();
    Uint32 n = name.size();

    if (!(*p < 128 && CharSet::isAlphaUnder(*p)))
    {
        if (!(*p >= 0x0080 && *p <= 0xFFEF))
            return false;
    }

    p++;
    n--;

    // Use loop unrolling to skip over good ASCII 7-bit characters.

    while (n >= 4)
    {
        if (p[0] < 128 && CharSet::isAlNumUnder(p[0]) &&
            p[1] < 128 && CharSet::isAlNumUnder(p[1]) &&
            p[2] < 128 && CharSet::isAlNumUnder(p[2]) &&
            p[3] < 128 && CharSet::isAlNumUnder(p[3]))
        {
            p += 4;
            n -= 4;
            continue;
        }

        break;
    }

    // Process remaining charcters.

    while (n)
    {
        if (!(*p < 128 && CharSet::isAlNumUnder(*p)))
        {
            if (!(*p >= 0x0080 && *p <= 0xFFEF))
                return false;
        }
        p++;
        n--;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// CIMNamespaceName
//
////////////////////////////////////////////////////////////////////////////////

#define PEGASUS_ARRAY_T CIMNamespaceName
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

inline void _check_namespace_name(String& name)
{
    if (!CIMNamespaceName::legal(name))
        throw InvalidNamespaceNameException(name);

    if (name[0] == '/')
        name.remove(0, 1);
}

CIMNamespaceName::CIMNamespaceName(const String& name)
    : cimNamespaceName(name)
{
    _check_namespace_name(cimNamespaceName);
}

CIMNamespaceName::CIMNamespaceName(const char* name)
    : cimNamespaceName(name)
{
    _check_namespace_name(cimNamespaceName);
}

CIMNamespaceName& CIMNamespaceName::operator=(const CIMNamespaceName& name)
{
    cimNamespaceName = name.cimNamespaceName;
    return *this;
}

CIMNamespaceName& CIMNamespaceName::operator=(const String& name)
{
    cimNamespaceName = name;
    _check_namespace_name(cimNamespaceName);
    return *this;
}

CIMNamespaceName& CIMNamespaceName::operator=(const char* name)
{
    cimNamespaceName = name;
    _check_namespace_name(cimNamespaceName);
    return *this;
}

Boolean CIMNamespaceName::legal(const String& name)
{
    Uint32 length = name.size();
    Uint32 index = 0;

    // Skip a leading '/' because the CIM specification is ambiguous
    if (name[0] == '/')
    {
        index++;
    }

    Boolean moreElements = true;

    // Check each namespace element (delimited by '/' characters)
    while (moreElements)
    {
        moreElements = false;

        if (index == length)
        {
            return false;
        }

        Uint16 ch = name[index++];

        // First character must be alphabetic or '_' if ASCII

        if (!(ch < 128 && CharSet::isAlphaUnder(ch)))
        {
            if (!(ch >= 0x0080 && ch <= 0xFFEF))
                return false;
        }

        // Remaining characters must be alphanumeric or '_' if ASCII
        while (index < length)
        {
            ch = name[index++];

            // A '/' indicates another namespace element follows
            if (ch == '/')
            {
                moreElements = true;
                break;
            }

            if (!(ch < 128 && CharSet::isAlNumUnder(ch)))
            {
                if (!(ch >= 0x0080 && ch <= 0xFFEF))
                    return false;
            }
        }
    }

    return true;
}

Boolean operator==(const CIMNamespaceName& name1, const CIMNamespaceName& name2)
{
    return name1.equal(name2);
}

Boolean operator==(const CIMNamespaceName& name1, const char* name2)
{
    return name1.equal(name2);
}

Boolean operator==(const char* name1, const CIMNamespaceName& name2)
{
    return name2.equal(name1);
}

Boolean operator!=(const CIMNamespaceName& name1, const CIMNamespaceName& name2)
{
    return !name1.equal(name2);
}

Boolean operator!=(const CIMNamespaceName& name1, const char* name2)
{
    return !name1.equal(name2);
}

Boolean operator!=(const char* name1, const CIMNamespaceName& name2)
{
    return !name2.equal(name1);
}

PEGASUS_NAMESPACE_END

/*
================================================================================

Optimizations:

    1. Optimized legal().
    2. Implmented "char*" version of operator=().
    3. Added conditional inlining (for Pegaus internal use).
    4. Added comparison functions for "char*".
    5. Implemented "unchecked" version of constructors and assignment operators
       that take String or "char*".
    6. Added loop unrolling to CIMName::legal()

================================================================================
*/
