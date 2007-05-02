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

#ifndef Pegasus_ObjectNormalizer_h
#define Pegasus_ObjectNormalizer_h

#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE NormalizerContext
{
public:
    virtual ~NormalizerContext() {};

    virtual CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& name) = 0;

    virtual Array<CIMName> enumerateClassNames(
        const CIMNamespaceName& nameSpace, const CIMName& className,
        bool deepInheritance) = 0;

    virtual NormalizerContext* clone() = 0;
};

// TODO: add documentation
class PEGASUS_COMMON_LINKAGE ObjectNormalizer
{
public:
    ObjectNormalizer();
    ObjectNormalizer(
        const CIMClass& cimClass,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMNamespaceName& nameSpace,
        SharedPtr<NormalizerContext>& context);

    CIMObjectPath processClassObjectPath(
        const CIMObjectPath& cimObjectPath) const;
    CIMObjectPath processInstanceObjectPath(
        const CIMObjectPath& cimObjectPath) const;

    //CIMClass processClass(const CIMClass& cimClass) const;
    CIMInstance processInstance(const CIMInstance& cimInstance) const;
    //CIMIndication processIndication(const CIMIndication& cimIndication);

    static CIMProperty _processProperty(
        CIMConstProperty& referenceProperty,
        CIMConstProperty& cimProperty,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        NormalizerContext * context,
        const CIMNamespaceName& nameSpace);

private:
    CIMClass _cimClass;

    Boolean _includeQualifiers;
    Boolean _includeClassOrigin;
    SharedPtr<NormalizerContext> _context;
    CIMNamespaceName _nameSpace;
};

PEGASUS_NAMESPACE_END

#endif
