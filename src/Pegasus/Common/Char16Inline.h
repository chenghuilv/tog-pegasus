//%2005////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
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

#ifndef Pegasus_Char16Inline_h
#define Pegasus_Char16Inline_h

PEGASUS_NAMESPACE_BEGIN

PEGASUS_CHAR16_INLINE Char16::Char16() : _code(0)
{
}

PEGASUS_CHAR16_INLINE Char16::Char16(Uint16 x) : _code(x)
{
}

PEGASUS_CHAR16_INLINE Char16::Char16(const Char16& x) : _code(x._code)
{
}

PEGASUS_CHAR16_INLINE Char16::~Char16()
{
}

PEGASUS_CHAR16_INLINE Char16& Char16::operator=(Uint16 x)
{
    _code = x;
    return *this;
}

PEGASUS_CHAR16_INLINE Char16& Char16::operator=(const Char16& x)
{
    _code = x._code;
    return *this;
}

PEGASUS_CHAR16_INLINE Char16::operator Uint16() const
{
    return _code;
}

PEGASUS_CHAR16_INLINE Boolean operator==(const Char16& x, const Char16& y)
{
    return Uint16(x) == Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator==(const Char16& x, char y)
{
    return Uint16(x) == Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator==(char x, const Char16& y)
{
    return Uint16(x) == Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator!=(const Char16& x, const Char16& y)
{
    return Uint16(x) != Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator!=(const Char16& x, char y)
{
    return Uint16(x) != Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator!=(char x, const Char16& y)
{
    return Uint16(x) != Uint16(y);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Char16Inline_h */
