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

#include <Pegasus/Common/Config.h>
#include <cstdio>
#include "XmlWriter.h"
#include "CIMParamValueRep.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN

CIMParamValueRep::CIMParamValueRep(
    String parameterName,
    CIMValue value,
    Boolean isTyped)
    : _parameterName(parameterName), _value(value), _isTyped(isTyped)
{
    // ensure parameterName is not null
    if (parameterName.size() == 0)
    {
        throw UninitializedObjectException();
    }
}

CIMParamValueRep::~CIMParamValueRep()
{
}

//------------------------------------------------------------------------------
//
//     <!ELEMENT PARAMVALUE (VALUE|VALUE.REFERENCE|VALUE.ARRAY|VALUE.REFARRAY)?>
//     <!ATTLIST PARAMVALUE
//         %CIMName;
//         %EmbeddedObject; #IMPLIED
//         %ParamType;>
//
//------------------------------------------------------------------------------
void CIMParamValueRep::toXml(Buffer& out) const
{
    out << STRLIT("<PARAMVALUE NAME=\"") << _parameterName;
    out.append('"');

    CIMType type = _value.getType();

    if (_isTyped)
    {
        // If the property type is CIMObject, then
        //   encode the property in CIM-XML as a string with the EMBEDDEDOBJECT
        //   attribute (there is not currently a CIM-XML "object" datatype)
        // else
        //   output the real type
        if (type == CIMTYPE_OBJECT)
        {
            out << STRLIT(" PARAMTYPE=\"string\" EMBEDDEDOBJECT=\"object\"");
        }
#ifdef PEGASUS_EMBEDDED_INSTANCE_SUPPORT
        else if (type == CIMTYPE_INSTANCE)
        {
            out << STRLIT(" PARAMTYPE=\"string\"");
            out << STRLIT(" EMBEDDEDOBJECT=\"instance\"");
        }
#endif // PEGASUS_EMBEDDED_INSTANCE_SUPPORT
        else
        {
            out << STRLIT(" PARAMTYPE=\"") << cimTypeToString (type);
            out.append('"');
        }
    }

    out << STRLIT(">\n");
    XmlWriter::appendValueElement(out, _value);

    out << STRLIT("</PARAMVALUE>\n");
}

CIMParamValueRep::CIMParamValueRep()
{
}

CIMParamValueRep::CIMParamValueRep(const CIMParamValueRep& x) :
    Sharable(),
    _parameterName(x._parameterName),
    _value(x._value),
    _isTyped(x._isTyped)
{
}

void CIMParamValueRep::setParameterName(String& parameterName)
{
    // ensure parameterName is not null
    if (parameterName.size() == 0)
    {
        throw UninitializedObjectException();
    }

    _parameterName = parameterName;
}

void CIMParamValueRep::setValue(CIMValue& value)
{
    _value = value;
}

void CIMParamValueRep::setIsTyped(Boolean isTyped)
{
    _isTyped = isTyped;
}

PEGASUS_NAMESPACE_END
