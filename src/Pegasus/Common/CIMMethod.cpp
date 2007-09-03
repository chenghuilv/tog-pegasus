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

#include "CIMMethod.h"
#include "CIMMethodRep.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMMethod
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

///////////////////////////////////////////////////////////////////////////////
//
// CIMMethod
//
///////////////////////////////////////////////////////////////////////////////

CIMMethod::CIMMethod()
    : _rep(0)
{
}

CIMMethod::CIMMethod(const CIMMethod& x)
{
    Inc(_rep = x._rep);
}

CIMMethod::CIMMethod(
    const CIMName& name,
    CIMType type,
    const CIMName& classOrigin,
    Boolean propagated)
{
    _rep = new CIMMethodRep(name, type, classOrigin, propagated);
}

CIMMethod::CIMMethod(CIMMethodRep* rep)
    : _rep(rep)
{
}

CIMMethod::CIMMethod(const CIMConstMethod& x)
{
    Inc(_rep = x._rep);
}

CIMMethod::~CIMMethod()
{
    Dec(_rep);
}

CIMMethod& CIMMethod::operator=(const CIMMethod& x)
{
    if (x._rep != _rep)
    {
        Dec(_rep);
        Inc(_rep = x._rep);
    }
    return *this;
}

const CIMName& CIMMethod::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

void CIMMethod::setName(const CIMName& name)
{
    CheckRep(_rep);
    _rep->setName(name);
}

CIMType CIMMethod::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

void CIMMethod::setType(CIMType type)
{
    CheckRep(_rep);
    _rep->setType(type);
}

const CIMName& CIMMethod::getClassOrigin() const
{
    CheckRep(_rep);
    return _rep->getClassOrigin();
}

void CIMMethod::setClassOrigin(const CIMName& classOrigin)
{
    CheckRep(_rep);
    _rep->setClassOrigin(classOrigin);
}

Boolean CIMMethod::getPropagated() const
{
    CheckRep(_rep);
    return _rep->getPropagated();
}

void CIMMethod::setPropagated(Boolean propagated)
{
    CheckRep(_rep);
    _rep->setPropagated(propagated);
}

CIMMethod& CIMMethod::addQualifier(const CIMQualifier& x)
{
    CheckRep(_rep);
    _rep->addQualifier(x);
    return *this;
}

Uint32 CIMMethod::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMQualifier CIMMethod::getQualifier(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

CIMConstQualifier CIMMethod::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

void CIMMethod::removeQualifier(Uint32 index)
{
    CheckRep(_rep);
    _rep->removeQualifier(index);
}

Uint32 CIMMethod::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

CIMMethod& CIMMethod::addParameter(const CIMParameter& x)
{
    CheckRep(_rep);
    _rep->addParameter(x);
    return *this;
}

Uint32 CIMMethod::findParameter(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findParameter(name);
}

CIMParameter CIMMethod::getParameter(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getParameter(index);
}

CIMConstParameter CIMMethod::getParameter(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getParameter(index);
}

void CIMMethod::removeParameter (Uint32 index)
{
    CheckRep(_rep);
    _rep->removeParameter (index);
}

Uint32 CIMMethod::getParameterCount() const
{
    CheckRep(_rep);
    return _rep->getParameterCount();
}

Boolean CIMMethod::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMMethod::identical(const CIMConstMethod& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMMethod CIMMethod::clone() const
{
    return CIMMethod(_rep->clone());
}

void CIMMethod::_checkRep() const
{
    if (!_rep)
        throw UninitializedObjectException();
}


///////////////////////////////////////////////////////////////////////////////
//
// CIMConstMethod
//
///////////////////////////////////////////////////////////////////////////////

CIMConstMethod::CIMConstMethod()
    : _rep(0)
{
}

CIMConstMethod::CIMConstMethod(const CIMConstMethod& x)
{
    Inc(_rep = x._rep);
}

CIMConstMethod::CIMConstMethod(const CIMMethod& x)
{
    Inc(_rep = x._rep);
}

CIMConstMethod::CIMConstMethod(
    const CIMName& name,
    CIMType type,
    const CIMName& classOrigin,
    Boolean propagated)
{
    _rep = new CIMMethodRep(name, type, classOrigin, propagated);
}

CIMConstMethod::~CIMConstMethod()
{
    Dec(_rep);
}

CIMConstMethod& CIMConstMethod::operator=(const CIMConstMethod& x)
{
    if (x._rep != _rep)
    {
        Dec(_rep);
        Inc(_rep = x._rep);
    }
    return *this;
}

CIMConstMethod& CIMConstMethod::operator=(const CIMMethod& x)
{
    if (x._rep != _rep)
    {
        Dec(_rep);
        Inc(_rep = x._rep);
    }
    return *this;
}

const CIMName& CIMConstMethod::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

CIMType CIMConstMethod::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

const CIMName& CIMConstMethod::getClassOrigin() const
{
    CheckRep(_rep);
    return _rep->getClassOrigin();
}

Boolean CIMConstMethod::getPropagated() const
{
    CheckRep(_rep);
    return _rep->getPropagated();
}

Uint32 CIMConstMethod::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMConstQualifier CIMConstMethod::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

Uint32 CIMConstMethod::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Uint32 CIMConstMethod::findParameter(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findParameter(name);
}

CIMConstParameter CIMConstMethod::getParameter(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getParameter(index);
}

Uint32 CIMConstMethod::getParameterCount() const
{
    CheckRep(_rep);
    return _rep->getParameterCount();
}

Boolean CIMConstMethod::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMConstMethod::identical(const CIMConstMethod& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMMethod CIMConstMethod::clone() const
{
    return CIMMethod(_rep->clone());
}

void CIMConstMethod::_checkRep() const
{
    if (!_rep)
        throw UninitializedObjectException();
}

PEGASUS_NAMESPACE_END
