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

#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/CIMClassRep.h>
#include <Pegasus/Common/CIMInstanceRep.h>
#include <Pegasus/Common/CIMPropertyRep.h>
#include <Pegasus/Common/CIMMethodRep.h>
#include <Pegasus/Common/CIMParameterRep.h>
#include <Pegasus/Common/CIMQualifierRep.h>

PEGASUS_NAMESPACE_BEGIN

void Resolver::resolveClass(
    CIMClass& theClass,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace)
{
    CheckRep(theClass._rep);
    theClass._rep->resolve(declContext, nameSpace);
}

void Resolver::resolveInstance(
    CIMInstance& theInstance,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace,
    Boolean propagateQualifiers)
{
    CheckRep(theInstance._rep);
    CIMConstClass cimClass;
    theInstance._rep->resolve(
        declContext, nameSpace, cimClass, propagateQualifiers);
}

void Resolver::resolveInstance(
    CIMInstance& theInstance,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace,
    CIMConstClass& cimClassOut,
    Boolean propagateQualifiers)
{
    CheckRep(theInstance._rep);
    theInstance._rep->resolve(declContext, nameSpace, cimClassOut,
        propagateQualifiers);
}

void Resolver::resolveProperty(
    CIMProperty& theProperty,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace,
    Boolean isInstancePart,
    const CIMConstProperty& inheritedProperty,
    Boolean propagateQualifiers)
{
    CheckRep(theProperty._rep);
    theProperty._rep->resolve(declContext, nameSpace, isInstancePart,
        inheritedProperty, propagateQualifiers);
}

void Resolver::resolveProperty(
    CIMProperty& theProperty,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace,
    Boolean isInstancePart,
    Boolean propagateQualifiers)
{
    CheckRep(theProperty._rep);
    theProperty._rep->resolve(declContext, nameSpace, isInstancePart,
        propagateQualifiers);
}

void Resolver::resolveMethod(
    CIMMethod& theMethod,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace,
    const CIMConstMethod& inheritedMethod)
{
    CheckRep(theMethod._rep);
    theMethod._rep->resolve(declContext, nameSpace, inheritedMethod);
}

void Resolver::resolveMethod(
    CIMMethod& theMethod,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace)
{
    CheckRep(theMethod._rep);
    theMethod._rep->resolve(declContext, nameSpace);
}

void Resolver::resolveParameter(
    CIMParameter& theParameter,
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace)
{
    CheckRep(theParameter._rep);
    theParameter._rep->resolve(declContext, nameSpace);
}

void Resolver::resolveQualifierFlavor(
    CIMQualifier& theQualifier,
    const CIMFlavor& inheritedFlavor,
    Boolean inherited)
{
    CheckRep(theQualifier._rep);
    theQualifier._rep->resolveFlavor(inheritedFlavor, inherited);
}

PEGASUS_NAMESPACE_END
