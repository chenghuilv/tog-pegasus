//BEGIN_LICENSE
//
// Copyright (c) 2000 The Open Group, BMC Software, Tivoli Systems, IBM
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//END_LICENSE
//BEGIN_HISTORY
//
// Author:
//
// $Log: QualifierDecl.h,v $
// Revision 1.4  2001/01/30 23:39:00  karl
// Add doc++ Documentation to header files
//
// Revision 1.3  2001/01/23 01:25:35  mike
// Reworked resolve scheme.
//
// Revision 1.2  2001/01/15 04:31:44  mike
// worked on resolve scheme
//
// Revision 1.1.1.1  2001/01/14 19:53:07  mike
// Pegasus import
//
//
//END_HISTORY

#ifndef Pegasus_QualifierDecl_h
#define Pegasus_QualifierDecl_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/QualifierDeclRep.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// QualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

class ConstQualifierDecl;
class ClassDeclRep;
/** Class QualifierDecl

  NOTE: Clarify difference between qualifier and qualiferdeclaration
  ATTN: Important work required here.
*/
class PEGASUS_COMMON_LINKAGE QualifierDecl
{
public:
    /// Constructor - ATTN:
    QualifierDecl() : _rep(0)
    {

    }
    /// Constructor - ATTN:

    QualifierDecl(const QualifierDecl& x) 
    {
	Inc(_rep = x._rep); 
    }
    /** Constructor
    Throws IllegalName if name argument not legal CIM identifier.
    ATTN:
    */
    
    QualifierDecl(
	const String& name, 
	const Value& value, 
	Uint32 scope,
	Uint32 flavor = Flavor::DEFAULTS,
	Uint32 arraySize = 0)
    {
	_rep = new QualifierDeclRep(name, value, scope, flavor, arraySize);
    }
    /// Destructor
    ~QualifierDecl()
    {
	Dec(_rep);
    }
    /// Operator
    QualifierDecl& operator=(const QualifierDecl& x)
    {
	if (x._rep != _rep)
	{
	    Dec(_rep);
	    Inc(_rep = x._rep);
	}

	return *this;
    }
    /** Method ATTN:

    
    */
    const String& getName() const 
    { 
	_checkRep();
	return _rep->getName(); 
    }

    // Throws IllegalName if name argument not legal CIM identifier.
    /** Method	ATTN:
    
    */
    void setName(const String& name) 
    { 
	_checkRep();
	_rep->setName(name); 
    }
    /** Method ATTN:

    
    */
    Type getType() const 
    { 
	_checkRep();
	return _rep->getType(); 
    }
    /** Method  ATTN:

    
    */
    Boolean isArray() const 
    {
	_checkRep();
	return _rep->isArray();
    }
    /** Method
    
    */
    const Value& getValue() const 
    { 
	_checkRep();
	return _rep->getValue(); 
    }
    /** Method
    
    */
    void setValue(const Value& value) 
    { 
	_checkRep();
	_rep->setValue(value); 
    }
    /** Method
    
    */
    Uint32 getScope() const 
    {
	_checkRep();
	return _rep->getScope();
    }
    /** Method

    */

    Uint32 getFlavor() const 
    {
	_checkRep();
	return _rep->getFlavor();
    }
    /** Method

    */

    Uint32 getArraySize() const 
    {
	_checkRep();
	return _rep->getArraySize();
    }
    /** Method
    
    */

    operator int() const { return _rep != 0; }
    /** Method
    
    */

    void toXml(Array<Sint8>& out) const
    {
	_checkRep();
	_rep->toXml(out);
    }
    /** Method
    
    */

    void print() const
    {
	_checkRep();
	_rep->print();
    }
    /** Method
    
    */

    Boolean identical(const ConstQualifierDecl& x) const;
    /** Method
    
    */

    QualifierDecl clone() const
    {
	return QualifierDecl(_rep->clone());
    }

private:

    QualifierDecl(QualifierDeclRep* rep) : _rep(rep)
    {
    }

    void _checkRep() const
    {
	if (!_rep)
	    throw UnitializedHandle();
    }

    QualifierDeclRep* _rep;
    friend class ConstQualifierDecl;
    friend class ClassDeclRep;
};

////////////////////////////////////////////////////////////////////////////////
//
// ConstQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

class PEGASUS_COMMON_LINKAGE ConstQualifierDecl
{
public:

    ConstQualifierDecl() : _rep(0)
    {

    }

    ConstQualifierDecl(const ConstQualifierDecl& x) 
    {
	Inc(_rep = x._rep); 
    }

    ConstQualifierDecl(const QualifierDecl& x) 
    {
	Inc(_rep = x._rep); 
    }

    // Throws IllegalName if name argument not legal CIM identifier.

    ConstQualifierDecl(
	const String& name, 
	const Value& value, 
	Uint32 scope,
	Uint32 flavor = Flavor::DEFAULTS,
	Uint32 arraySize = 0)
    {
	_rep = new QualifierDeclRep(name, value, scope, flavor, arraySize);
    }

    ~ConstQualifierDecl()
    {
	Dec(_rep);
    }

    ConstQualifierDecl& operator=(const ConstQualifierDecl& x)
    {
	if (x._rep != _rep)
	{
	    Dec(_rep);
	    Inc(_rep = x._rep);
	}

	return *this;
    }

    ConstQualifierDecl& operator=(const QualifierDecl& x)
    {
	if (x._rep != _rep)
	{
	    Dec(_rep);
	    Inc(_rep = x._rep);
	}

	return *this;
    }

    const String& getName() const 
    { 
	_checkRep();
	return _rep->getName(); 
    }

    Type getType() const 
    { 
	_checkRep();
	return _rep->getType(); 
    }

    Boolean isArray() const 
    {
	_checkRep();
	return _rep->isArray();
    }

    const Value& getValue() const 
    { 
	_checkRep();
	return _rep->getValue(); 
    }

    Uint32 getScope() const 
    {
	_checkRep();
	return _rep->getScope();
    }

    const Uint32 getFlavor() const 
    {
	_checkRep();
	return _rep->getFlavor();
    }

    Uint32 getArraySize() const 
    {
	_checkRep();
	return _rep->getArraySize();
    }

    operator int() const { return _rep != 0; }

    void toXml(Array<Sint8>& out) const
    {
	_checkRep();
	_rep->toXml(out);
    }

    void print() const
    {
	_checkRep();
	_rep->print();
    }

    Boolean identical(const ConstQualifierDecl& x) const
    {
	x._checkRep();
	_checkRep();
	return _rep->identical(x._rep);
    }

    QualifierDecl clone() const
    {
	return QualifierDecl(_rep->clone());
    }

private:

    void _checkRep() const
    {
	if (!_rep)
	    throw UnitializedHandle();
    }

    QualifierDeclRep* _rep;
    friend class QualifierDecl;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_QualifierDecl_h */
