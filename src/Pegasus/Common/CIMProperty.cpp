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

#include "CIMPropertyRep.h"
#include "CIMProperty.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMProperty
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T


////////////////////////////////////////////////////////////////////////////////
//
// CIMProperty
//
////////////////////////////////////////////////////////////////////////////////

CIMProperty::CIMProperty()
    : _rep(0)
{
}

CIMProperty::CIMProperty(const CIMProperty& x)
{
    Inc(_rep = x._rep);
}

CIMProperty::CIMProperty(
    const CIMName& name,
    const CIMValue& value,
    Uint32 arraySize,
    const CIMName& referenceClassName,
    const CIMName& classOrigin,
    Boolean propagated)
{
    _rep = new CIMPropertyRep(name, value,
        arraySize, referenceClassName, classOrigin, propagated);
}

CIMProperty::CIMProperty(CIMPropertyRep* rep)
    : _rep(rep)
{
}

CIMProperty::~CIMProperty()
{
    Dec(_rep);
}

CIMProperty& CIMProperty::operator=(const CIMProperty& x)
{
    if (x._rep != _rep)
    {
        Dec(_rep);
        Inc(_rep = x._rep);
    }
    return *this;
}

const CIMName& CIMProperty::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

void CIMProperty::setName(const CIMName& name)
{
    CheckRep(_rep);
    _rep->setName(name);
}

const CIMValue& CIMProperty::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

CIMType CIMProperty::getType() const
{
    CheckRep(_rep);
    return _rep->getValue().getType();
}

Boolean CIMProperty::isArray() const
{
    CheckRep(_rep);
    return _rep->getValue().isArray();
}

void CIMProperty::setValue(const CIMValue& value)
{
    CheckRep(_rep);
    _rep->setValue(value);
}

Uint32 CIMProperty::getArraySize() const
{
    CheckRep(_rep);
    return _rep->getArraySize();
}

const CIMName& CIMProperty::getReferenceClassName() const
{
    CheckRep(_rep);
    return _rep->getReferenceClassName();
}

const CIMName& CIMProperty::getClassOrigin() const
{
    CheckRep(_rep);
    return _rep->getClassOrigin();
}

void CIMProperty::setClassOrigin(const CIMName& classOrigin)
{
    CheckRep(_rep);
    _rep->setClassOrigin(classOrigin);
}

Boolean CIMProperty::getPropagated() const
{
    CheckRep(_rep);
    return _rep->getPropagated();
}

void CIMProperty::setPropagated(Boolean propagated)
{
    CheckRep(_rep);
    _rep->setPropagated(propagated);
}

CIMProperty& CIMProperty::addQualifier(const CIMQualifier& x)
{
    CheckRep(_rep);
    _rep->addQualifier(x);
    return *this;
}

Uint32 CIMProperty::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMQualifier CIMProperty::getQualifier(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

CIMConstQualifier CIMProperty::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

void CIMProperty::removeQualifier(Uint32 index)
{
    CheckRep(_rep);
    _rep->removeQualifier(index);
}

Uint32 CIMProperty::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Boolean CIMProperty::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMProperty::identical(const CIMConstProperty& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMProperty CIMProperty::clone() const
{
    return CIMProperty(_rep->clone());
}

void CIMProperty::_checkRep() const
{
    if (!_rep)
        throw UninitializedObjectException();
}

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstProperty
//
////////////////////////////////////////////////////////////////////////////////

CIMConstProperty::CIMConstProperty()
    : _rep(0)
{
}

CIMConstProperty::CIMConstProperty(const CIMConstProperty& x)
{
    Inc(_rep = x._rep);
}

CIMConstProperty::CIMConstProperty(const CIMProperty& x)
{
    Inc(_rep = x._rep);
}

CIMConstProperty::CIMConstProperty(
    const CIMName& name,
    const CIMValue& value,
    Uint32 arraySize,
    const CIMName& referenceClassName,
    const CIMName& classOrigin,
    Boolean propagated)
{
    _rep = new CIMPropertyRep(name, value,
        arraySize, referenceClassName, classOrigin, propagated);
}

CIMConstProperty::~CIMConstProperty()
{
    Dec(_rep);
}

CIMConstProperty& CIMConstProperty::operator=(const CIMConstProperty& x)
{
    if (x._rep != _rep)
    {
        Dec(_rep);
        Inc(_rep = x._rep);
    }
    return *this;
}

CIMConstProperty& CIMConstProperty::operator=(const CIMProperty& x)
{
    if (x._rep != _rep)
    {
        Dec(_rep);
        Inc(_rep = x._rep);
    }
    return *this;
}

const CIMName& CIMConstProperty::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

const CIMValue& CIMConstProperty::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

CIMType CIMConstProperty::getType() const
{
    CheckRep(_rep);
    return _rep->getValue().getType();
}

Boolean CIMConstProperty::isArray() const
{
    CheckRep(_rep);
    return _rep->getValue().isArray();
}

Uint32 CIMConstProperty::getArraySize() const
{
    CheckRep(_rep);
    return _rep->getArraySize();
}

const CIMName& CIMConstProperty::getReferenceClassName() const
{
    CheckRep(_rep);
    return _rep->getReferenceClassName();
}

const CIMName& CIMConstProperty::getClassOrigin() const
{
    CheckRep(_rep);
    return _rep->getClassOrigin();
}

Boolean CIMConstProperty::getPropagated() const
{
    CheckRep(_rep);
    return _rep->getPropagated();
}

Uint32 CIMConstProperty::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMConstQualifier CIMConstProperty::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

Uint32 CIMConstProperty::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Boolean CIMConstProperty::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMConstProperty::identical(const CIMConstProperty& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMProperty CIMConstProperty::clone() const
{
    return CIMProperty(_rep->clone());
}

void CIMConstProperty::_checkRep() const
{
    if (!_rep)
        throw UninitializedObjectException();
}

PEGASUS_NAMESPACE_END
