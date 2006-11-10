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
#include "CIMParameter.h"
#include "CIMParameterRep.h"
#include "Indentor.h"
#include "CIMName.h"
#include "CIMScope.h"
#include "XmlWriter.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN

CIMParameterRep::CIMParameterRep()
{
}

CIMParameterRep::CIMParameterRep(const CIMParameterRep& x) :
    Sharable(),
    _name(x._name),
    _type(x._type),
    _isArray(x._isArray),
    _arraySize(x._arraySize),
    _referenceClassName(x._referenceClassName)
{
    x._qualifiers.cloneTo(_qualifiers);
}

CIMParameterRep::CIMParameterRep(
    const CIMName& name,
    CIMType type,
    Boolean isArray,
    Uint32 arraySize,
    const CIMName& referenceClassName)
    : _name(name), _type(type),
    _isArray(isArray), _arraySize(arraySize),
    _referenceClassName(referenceClassName)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }

    if ((_arraySize != 0) && !_isArray)
    {
        throw TypeMismatchException();
    }

    if (!referenceClassName.isNull())
    {
        if (_type != CIMTYPE_REFERENCE)
        {
            throw TypeMismatchException();
        }
    }
    else
    {
        if (_type == CIMTYPE_REFERENCE)
        {
            throw TypeMismatchException();
        }
    }
}

CIMParameterRep::~CIMParameterRep()
{
}

void CIMParameterRep::setName(const CIMName& name)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }

    _name = name;
}

void CIMParameterRep::removeQualifier(Uint32 index)
{
    if (index >= _qualifiers.getCount())
        throw IndexOutOfBoundsException();

    _qualifiers.removeQualifier (index);
}

void CIMParameterRep::resolve(
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace)
{
    // Validate the qualifiers of the method (according to
    // superClass's method with the same name). This method
    // will throw an exception if the validation fails.

    CIMQualifierList dummy;

    _qualifiers.resolve(
        declContext,
        nameSpace,
        CIMScope::PARAMETER,
        false,
        dummy,
        true);
}

void CIMParameterRep::toXml(Buffer& out) const
{
    if (_isArray)
    {
        if (_type == CIMTYPE_REFERENCE)
        {
            out << STRLIT("<PARAMETER.REFARRAY NAME=\"") << _name;
            out.append('"');

            if (!_referenceClassName.isNull())
            {
                out << STRLIT(" REFERENCECLASS=\"");
                out << _referenceClassName.getString();
                out.append('"');
            }

            if (_arraySize)
            {
                char buffer[32];
                int n = sprintf(buffer, "%d", _arraySize);
                out << STRLIT(" ARRAYSIZE=\"");
                out.append(buffer, n);
                out.append('"');
            }

            out << STRLIT(">\n");

            _qualifiers.toXml(out);

            out << STRLIT("</PARAMETER.REFARRAY>\n");
        }
        else
        {
            out << STRLIT("<PARAMETER.ARRAY");
            out << STRLIT(" NAME=\"") << _name;
            out << STRLIT("\" ");
            out << STRLIT(" TYPE=\"") << cimTypeToString(_type);
            out.append('"');

            if (_arraySize)
            {
                char buffer[32];
                sprintf(buffer, "%d", _arraySize);
                out << STRLIT(" ARRAYSIZE=\"") << buffer;
                out.append('"');
            }

            out << STRLIT(">\n");

            _qualifiers.toXml(out);

            out << STRLIT("</PARAMETER.ARRAY>\n");
        }
    }
    else if (_type == CIMTYPE_REFERENCE)
    {
        out << STRLIT("<PARAMETER.REFERENCE");
        out << STRLIT(" NAME=\"") << _name;
        out.append('"');

        if (!_referenceClassName.isNull())
        {
            out << STRLIT(" REFERENCECLASS=\"");
            out << _referenceClassName.getString();
            out.append('"');
        }
        out << STRLIT(">\n");

        _qualifiers.toXml(out);

        out << STRLIT("</PARAMETER.REFERENCE>\n");
    }
    else
    {
        out << STRLIT("<PARAMETER");
        out << STRLIT(" NAME=\"") << _name;
        out << STRLIT("\" ");
        out << STRLIT(" TYPE=\"") << cimTypeToString(_type);
        out << STRLIT("\">\n");

        _qualifiers.toXml(out);

        out << STRLIT("</PARAMETER>\n");
    }
}

/** toMof - puts the Mof representation of teh Parameter object to
    the output parameter array
    The BNF for this conversion is:
    parameterList    =  parameter *( "," parameter )

        parameter    =  [ qualifierList ] (dataType|objectRef) parameterName
                        [ array ]

        parameterName=  IDENTIFIER

        array        =  "[" [positiveDecimalValue] "]"

    Format on a single line.
*/
void CIMParameterRep::toMof(Buffer& out) const
{
    // Output the qualifiers for the parameter
    _qualifiers.toMof(out);

    if (_qualifiers.getCount())
        out.append(' ');

    // Output the data type and name
    out << cimTypeToString(_type);
    out.append(' ');
    out <<  _name;

    if (_isArray)
    {
        //Output the array indicator "[ [arraysize] ]"
        if (_arraySize)
        {
            char buffer[32];
            int n = sprintf(buffer, "[%d]", _arraySize);
            out.append(buffer, n);
        }
        else
            out << STRLIT("[]");
    }
}


Boolean CIMParameterRep::identical(const CIMParameterRep* x) const
{
    if (!_name.equal (x->_name))
        return false;

    if (_type != x->_type)
        return false;

    if (!_referenceClassName.equal (x->_referenceClassName))
        return false;

    if (!_qualifiers.identical(x->_qualifiers))
        return false;

    return true;
}

PEGASUS_NAMESPACE_END
