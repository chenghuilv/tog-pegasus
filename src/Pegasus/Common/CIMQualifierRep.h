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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_QualifierRep_h
#define Pegasus_QualifierRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/CIMFlavor.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstQualifier;
class CIMQualifier;

class PEGASUS_COMMON_LINKAGE CIMQualifierRep : public Sharable
{
public:

    CIMQualifierRep(
	const CIMName& name, 
	const CIMValue& value, 
	const CIMFlavor & flavor,
	Boolean propagated);

    virtual ~CIMQualifierRep();

    const CIMName& getName() const 
    { 
	return _name; 
    }

    void setName(const CIMName& name);

    CIMType getType() const 
    { 
	return _value.getType(); 
    }

    Boolean isArray() const 
    {
	return _value.isArray();
    }

    const CIMValue& getValue() const 
    { 
	return _value; 
    }

    void setValue(const CIMValue& value);

    const CIMFlavor & getFlavor() const 
    {
	return _flavor; 
    }

    void setFlavor(const CIMFlavor & flavor) 
    {
	_flavor = flavor; 
    }

    void unsetFlavor(const CIMFlavor & flavor) 
    {
        _flavor.removeFlavor (flavor);
    }

    void resolveFlavor (const CIMFlavor & inheritedFlavor, Boolean inherited);

    Boolean getPropagated() const 
    { 
	return _propagated; 
    }

    void setPropagated(Boolean propagated) 
    {
	_propagated = propagated; 
    }

    void toXml(Array<Sint8>& out) const;

    void toMof(Array<Sint8>& out) const;

    Boolean identical(const CIMQualifierRep* x) const;

    CIMQualifierRep* clone() const
    {
	return new CIMQualifierRep(*this);
    }

private:

    CIMQualifierRep();

    // Cloning constructor:

    CIMQualifierRep(const CIMQualifierRep& x);

    // This method is declared and made private so that the compiler does
    // not implicitly define a default copy constructor.
    CIMQualifierRep& operator=(const CIMQualifierRep& x)
    {
        //PEGASUS_ASSERT(0);
        return *this;
    }

    CIMName _name;
    CIMValue _value;
    CIMFlavor _flavor;
    Boolean _propagated;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_QualifierRep_h */
