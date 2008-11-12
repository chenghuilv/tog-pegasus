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

#ifndef Pegasus_CIMObjectRep_h
#define Pegasus_CIMObjectRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMQualifierList.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/OrderedSet.h>
#include <Pegasus/Common/CIMPropertyRep.h>

PEGASUS_NAMESPACE_BEGIN

/** This class defines the internal representation of the CIMObject class.

    This base class has two implementations: CIMClassRep CIMInstanceRep. The
    CIMObjectRep pointer member of CIMObject points to one of these.

    This class contains what is common to CIMClass and CIMInstance.
*/
class CIMObjectRep 
{
public:

    CIMObjectRep(const CIMObjectPath& className);

    virtual ~CIMObjectRep();

    const CIMName& getClassName() const
    {
        return _reference.getClassName();
    }

    const CIMObjectPath& getPath() const
    {
        return _reference;
    }

    /**
      Sets the object path for the object
      @param  path  CIMObjectPath containing the object path
     */
    void setPath (const CIMObjectPath & path);

    void addQualifier(const CIMQualifier& qualifier)
    {
        _qualifiers.add(qualifier);
    }

    Uint32 findQualifier(const CIMName& name) const
    {
        return _qualifiers.find(name);
    }

    CIMQualifier getQualifier(Uint32 index)
    {
        return _qualifiers.getQualifier(index);
    }

    CIMConstQualifier getQualifier(Uint32 index) const
    {
        return _qualifiers.getQualifier(index);
    }

    Boolean isTrueQualifer(CIMName& name) const
    {
        return _qualifiers.isTrue(name);
    }

    Uint32 getQualifierCount() const
    {
        return _qualifiers.getCount();
    }

    void removeQualifier(Uint32 index)
    {
        _qualifiers.removeQualifier(index);
    }

    virtual void addProperty(const CIMProperty& x);

    Uint32 findProperty(const CIMName& name, Uint32 nameTag) const
    {
        return _properties.find(name, nameTag);
    }

    Uint32 findProperty(const CIMName& name) const
    {
        return _properties.find(name, generateCIMNameTag(name));
    }

    CIMProperty getProperty(Uint32 index)
    {
        return _properties[index];
    }

    CIMConstProperty getProperty(Uint32 index) const
    {
        return ((CIMObjectRep*)this)->getProperty(index);
    }

    void removeProperty(Uint32 index)
    {
        _properties.remove(index);
    }

    Uint32 getPropertyCount() const
    {
        return _properties.size();
    }

    virtual Boolean identical(const CIMObjectRep* x) const;

    virtual CIMObjectRep* clone() const = 0;

    void Inc()
    {
       _refCounter++;
    }

    void Dec()
    {
        if (_refCounter.decAndTestIfZero())
            delete this;
    }

protected:

    CIMObjectRep(const CIMObjectRep& x);

    CIMObjectPath _reference;
    CIMQualifierList _qualifiers;
    typedef OrderedSet<CIMProperty,
                       CIMPropertyRep,
                       PEGASUS_PROPERTY_ORDEREDSET_HASHSIZE> PropertySet;
    PropertySet _properties;

private:

    CIMObjectRep();    // Unimplemented
    CIMObjectRep& operator=(const CIMObjectRep& x);    // Unimplemented

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;

    friend class CIMObject;
    friend class BinaryStreamer;
    friend class CIMBuffer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMObjectRep_h */
