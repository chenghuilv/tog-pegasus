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
// Author: Bob Blair (bblair@bmc.com)
//
// $Log: cimmofParser.cpp,v $
// Revision 1.6  2001/03/05 17:00:47  bob
// Catch up with the change from CimException to CIMException
//
// Revision 1.5  2001/03/04 22:18:00  bob
// Cleanup, support for reference, message moving, start of instance support
//
// Revision 1.4  2001/02/26 04:33:30  mike
// Fixed many places where cim names were be compared with operator==(String,String).
// Changed all of these to use CIMName::equal()
//
// Revision 1.3  2001/02/18 19:02:17  mike
// Fixed CIM debacle
//
// Revision 1.2  2001/02/18 02:45:49  mike
// Added bison.simple to distribution so that there
// would be no dependency on an external directory.
//
// Revision 1.1  2001/02/16 23:59:09  bob
// Initial checkin
//
//
//END_HISTORY
//
// implementation of valueFactory 
//
//
//
// Implementation of methods of cimmofParser class
//
//
#include "cimmofParser.h"
#include <cstring>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMScope.h>
#include <iostream>
#include "valueFactory.h"
#include "cimmofMessages.h"

//
// These routines are in the lexer.  They are there because
// there is no need for class cimmofParser to know the details
// of YY_BUFFER_STATE and its associated methods.
//
extern int get_yy_buf_size_wrapper();
extern void *get_cimmof__current_buffer_wrapper();
extern int switch_to_buffer_wrapper(void *buffstate);
extern void *create_cimmof_buffer_wrapper(const FILE *f, int size);

const char ParseError::MSG[] = "";
const char LexerError::MSG[] = "";

cimmofParser *cimmofParser::_instance = 0;

cimmofParser::cimmofParser(): 
  parser(),  _cmdline(0), _repository(0) {
}

cimmofParser::~cimmofParser() {
  if (_repository)
    delete _repository;
}

cimmofParser *
cimmofParser::Instance() {
  if (!_instance) {
    _instance = new cimmofParser();
  }
  return _instance;
}


  //------------------------------------------------------------------
  // Methods for manipulating the members added in this specialization
  //------------------------------------------------------------------

//---------------------------------------------------------------------
// allow someone to set/get our compiler options object reference
//---------------------------------------------------------------------
void
cimmofParser::setCompilerOptions(const mofCompilerOptions *co) {
  _cmdline = co;
  const string path = co->get_namespacePath();
  setDefaultNamespacePath(path.c_str());
}

const mofCompilerOptions *
cimmofParser::getCompilerOptions() const {
  return _cmdline;
}

//---------------------------------------------------------------------
// Set/get the repository we will be using.  The path should be in
// the command line
//---------------------------------------------------------------------
bool
cimmofParser::setRepository(void) {
  String message;
  cimmofMessages::arglist arglist;
  const String &s = getDefaultNamespacePath();
  if (_cmdline) {
    String rep = _cmdline->get_repository_name();
    if (rep != "") {
      try {
      _repository = new cimmofRepository(rep);
      } catch(Exception &e) {
	arglist.push_back(rep);
	arglist.push_back(e.getMessage());
	cimmofMessages::getMessage(message,
				  cimmofMessages::REPOSITORY_CREATE_ERROR,
				  arglist);
	elog(message);
	return false;
      } 
      try {
        _repository->createNameSpace(s);
      } catch(AlreadyExists &) {
	// OK, that's what we expect
      } catch(Exception &e) {
	arglist.push_back(s);
	arglist.push_back(e.getMessage());
	cimmofMessages::getMessage(message, 
				   cimmofMessages::NAMESPACE_CREATE_ERROR,
				   arglist);
	elog(message);
	return false;
      }
    } else {
      cimmofMessages::getMessage(message, 
				 cimmofMessages::SETREPOSITORY_BLANK_NAME);
      elog(message);
    }
  } else {
    cimmofMessages::getMessage(message, 
		       cimmofMessages::SETREPOSITORY_NO_COMPILER_OPTIONS);
    elog(message);
  }
  return (_repository ? true : false);
}

const cimmofRepository *
cimmofParser::getRepository() const {
  return _repository;
}

//------------------------------------------------------------------
// Set up the default and override namespace path in the repository
//------------------------------------------------------------------
void 
cimmofParser::setDefaultNamespacePath(const String &path) {
  if (String::equal(_defaultNamespacePath, ""))  // it can only be set once
    _defaultNamespacePath = path;
}

void
cimmofParser::setCurrentNamespacePath(const String &path) {
  _currentNamespacePath = path;
}

//------------------------------------------------------------------
// Return the namespace path members
//------------------------------------------------------------------
const String &
cimmofParser::getDefaultNamespacePath() const {
  return _defaultNamespacePath;
}

const String &
cimmofParser::getCurrentNamespacePath() const {
  return _currentNamespacePath;
}

const String &
cimmofParser::getNamespacePath() const {
  if (String::equal(_currentNamespacePath, "")) {
    return _defaultNamespacePath;
  }
  return _currentNamespacePath;
}

  //------------------------------------------------------------------
  // Methods that implement or override base class methods
  //------------------------------------------------------------------

//-------------------------------------------------------------------
// Methods for setting the parser's input buffer either from a saved
// buffer state or from an open file handle
//-------------------------------------------------------------------
int
cimmofParser::setInputBuffer(const FILE *f) {
  void *buf = create_cimmof_buffer_wrapper(f, get_buffer_size());
  if (buf)
    return setInputBuffer(buf);
  else
    return -1;
}

int
cimmofParser::setInputBuffer(void *buffstate)
{
  return switch_to_buffer_wrapper(buffstate);
};

//--------------------------------------------------------------------
// Handle include files from either the file name or an open handle
//--------------------------------------------------------------------
int
cimmofParser::enterInlineInclude(const String &filename) {
  int ret = 1;
  FILE *f = 0;
  char *fqname = (char *)filename.allocateCString(0, true);

  f = fopen(fqname, "r");
  if (!f) {
    if (_cmdline) {
      const vector<string> &include_paths = _cmdline->get_include_paths();
      for (unsigned int i = 0; i < include_paths.size(); i++) {
	string s = include_paths[i] + "/" + fqname;
	if ( (f = fopen(s.c_str(), "r")) ) {
	  _includefile = s;
	  break;
	}	 
      } 
    } else {  // incorrect call:  cmdline should have been set
      return ret;
    }
  }
  if (f) {
     ret = enterInlineInclude((const FILE *)f);
  } else {
    // FIXME:  need to throw an exception 
    cerr << "Could not open include file " << fqname << endl;
  }
  return ret;
}
    
int
cimmofParser::enterInlineInclude(const FILE *f) {
  if (f) {
    set_buffer_size(get_yy_buf_size_wrapper());
    void *buf = get_cimmof__current_buffer_wrapper();
    bufstate *bs = new bufstate;
    bs->buffer_state = buf;
    bs->filename = get_current_filename();
    bs->lineno = get_lineno();
    push_statebuff(bs);
    set_current_filename(_includefile);
    set_lineno(0);
    return setInputBuffer(f);
  }
  return 1;
}

//--------------------------------------------------------------------
// Handle the parser telling us he's reached end-of-file
//--------------------------------------------------------------------
int
cimmofParser::wrapCurrentBuffer()
{ return wrap(); }

//--------------------------------------------------------------------
// Tell the parser to start on the buffer that's been set
//--------------------------------------------------------------------
int
cimmofParser::parse()
{ return cimmof_parse(); }

//----------------------------------------------------------------------
// Override the default parser error routine to enable I18n
//----------------------------------------------------------------------
void
cimmofParser::log_parse_error(char *token, char *errmsg) const {
  String message;
  char buf[40];  // itoa can't overflow
  sprintf(buf, "%d", get_lineno());
  cimmofMessages::arglist arglist;
  arglist.push_back(get_current_filename().c_str());
  arglist.push_back(buf);
  arglist.push_back(errmsg);
  arglist.push_back(token);
  cimmofMessages::getMessage(message, cimmofMessages::PARSER_SYNTAX_ERROR,
			     arglist);
  elog(message);
  maybeThrowLexerError(message);
}

  //------------------------------------------------------------------
  // Do various representation transformations.
  //------------------------------------------------------------------

// -----------------------------------------------------------------
// Convert a string representing an octal integer to a string        
// representing the corresponding decimal integer                    
// FIXME:  Need to support 64-bit integers
// FIXME:  Needs to support non-ascii strings
// ------------------------------------------------------------------
char *
cimmofParser::oct_to_dec(const String &octrep) const {
  unsigned long oval = 0;
  char buf[40];  // can't overrrun on an itoa of a long
  for (int i = octrep.getLength() - 1; i >= 0; --i) {
     oval *= 8;
     switch(octrep[i]) {
     case '1': oval += 1; break;
     case '2': oval += 2; break;
     case '3': oval += 3; break;
     case '4': oval += 4; break;
     case '5': oval += 5; break;
     case '6': oval += 6; break;
     case '7': oval += 7; break; 
     }
  }
  sprintf(buf, "%ld", oval);
  return strdup(buf);
}

// ----------------------------------------------------------------- 
// Convert a string representing a hexadecimal integer to a string   
// representing the corresponding decimal integer                    
// FIXME:  Need to support 64-bit integers
// FIXME:  Needs to support non-ascii strings
// ------------------------------------------------------------------
char *
cimmofParser::hex_to_dec(const String &hexrep) const {
  unsigned long hval = 0;
  char buf[40];  // can't overrrun on an itoa of a long
  for (int i = hexrep.getLength() - 1; i >= 2; --i) {
     hval *= 16;
     switch(hexrep[i]) {
     case '1': hval += 1; break;
     case '2': hval += 2; break;
     case '3': hval += 3; break;
     case '4': hval += 4; break;
     case '5': hval += 5; break;
     case '6': hval += 6; break;
     case '7': hval += 7; break; 
     case '8': hval += 8; break;
     case '9': hval += 9; break;
     case 'a': case 'A':
       hval += 10;
       break;
     case 'b': case 'B':
       hval += 11;
       break;
     case 'c': case 'C':
       hval += 12;
       break;
     case 'd': case 'D':
       hval += 13;
       break;
     case 'e': case 'E':
       hval += 14;
       break;
     case 'f': case 'F':
       hval += 15;
       break;
     }
  }
  sprintf(buf, "%ld", hval);
  //  ltoa(hval, buf, 10);
  return strdup(buf);
}

// ----------------------------------------------------------------- 
// Convert a string representing a binary integer to a string        
// representing the corresponding decimal integer                    
// FIXME:  Need to support 64-bit integers
// ------------------------------------------------------------------
char *
cimmofParser::binary_to_dec(const String &binrep) const {
  unsigned long bval = 0;
  char buf[40];  // can't overrrun on an itoa of a long
  for (int i = binrep.getLength() - 1; i >= 0; --i) {
     bval *= 2;
     bval += binrep[i] == '1' ? 1 : 0;
  }
  sprintf(buf, "%ld", bval);
  return strdup(buf);
}

  //------------------------------------------------------------------
  // Handle the processing of CIM-specific constructs
  //------------------------------------------------------------------

//--------------------------------------------------------------------
// Take the compiler-local action specified by the #pragma (directive).
// Note that the Include #pragma is handled elsewhere.
//--------------------------------------------------------------------
void
cimmofParser::processPragma(const String &name, const String &value) {
  // The valid names are:
  // instancelocale
  // locale
  // namespace
  // nonlocal
  // nonlocaltype
  // source
  // sourcetype

  // FIXME:  We have to be able to do something about the namespaces
  // at least.
}

//-------------------------------------------------------------------
// When a class declaration production is complete, try to add it to
// the Repository
//-------------------------------------------------------------------
int 
cimmofParser::addClass(CIMClass *classdecl)
{
  int ret = 0;
  String message;
  cimmofMessages::arglist arglist;
  arglist.push_back(classdecl->getClassName());
  if ( _cmdline && _cmdline->trace() ) {
    String header;
    cimmofMessages::getMessage(header, cimmofMessages::ADD_CLASS);
    trace(header,""); 
    if (classdecl)
      classdecl->print(_cmdline->traceos()); 
  }
  if (_cmdline && _cmdline->syntax_only()) {
    return ret; 
  }
  try {
    ret = _repository->addClass(classdecl);
  } catch(AlreadyExists) {
    cimmofMessages::getMessage(message, cimmofMessages::CLASS_EXISTS_WARNING,
			       arglist);
    wlog(message);
  } catch (CIMException &e) {
    if (e.getCode() == CIMException::ALREADY_EXISTS) {
      cimmofMessages::getMessage(message, cimmofMessages::CLASS_EXISTS_WARNING,
				 arglist);
      wlog(message);
    } else {
      arglist.push_back(e.getMessage());
      cimmofMessages::getMessage(message, cimmofMessages::ADD_CLASS_ERROR,
				 arglist);
      elog(message);
      maybeThrowParseError(message);
    } 
  } catch(Exception &e) {
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message, cimmofMessages::ADD_CLASS_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 

  if (_cmdline && _cmdline->trace()) {
    String ok;
    cimmofMessages::getMessage(ok, cimmofMessages::TAB_OK);
    trace(ok, "");
  }
  return ret;
}

//---------------------------------------------------------------------
// When a new class declaration is detected, create the CIMClassDecl
// object
//---------------------------------------------------------------------
CIMClass *
cimmofParser::newClassDecl(const String &name, const String &superclassname)
{
  CIMClass *c = 0;
  try {
    c = new CIMClass(name, superclassname);
  } catch(CIMException &e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(name);
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message, cimmofMessages::NEW_CLASS_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 

  return c;
}

//---------------------------------------------------------------------
// When an instance production is complete, add it to the Repository
//---------------------------------------------------------------------
int 
cimmofParser::addInstance(CIMInstance *instance)
  // FIXME: This hasn't been thought about much
{ 
  int ret = 0;
  if (_cmdline && _cmdline->trace()) {
    String header;
    cimmofMessages::getMessage(header, cimmofMessages::ADD_INSTANCE);
    trace(header, "");
    if (instance)
      instance->print(_cmdline->traceos());
  }
  if (_cmdline && _cmdline->syntax_only()) {
    return ret; 
  }
  // FIXME:  Add try-catch
  ret = _repository->addInstance(instance);
  if (_cmdline && _cmdline->trace()) {
    String ok;
    cimmofMessages::getMessage(ok, cimmofMessages::TAB_OK);
    trace(ok, "");
  }
  return ret;
}

//---------------------------------------------------------------------
// When the start of a Qualifier Declaration is found, create the new
// CIMQualifierDecl object
//---------------------------------------------------------------------
CIMQualifierDecl *
cimmofParser::newQualifierDecl(const String &name, const CIMValue *value,
			       Uint32 scope, Uint32 flavor) {
  CIMQualifierDecl *q = 0;
  try {
    q = new CIMQualifierDecl(name, *value, scope, flavor);
  } catch(Exception &e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(name);
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message,
			       cimmofMessages::NEW_QUALIFIER_DECLARATION_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 

  return q;
}

//---------------------------------------------------------------------
// When a QualifierDeclaration production is complete, add the qualifier
// to the Repository.
//---------------------------------------------------------------------
int 
cimmofParser::addQualifier(CIMQualifierDecl *qualifier)
  // FIXME
{
  int ret  = 0;
  if (_cmdline && _cmdline->trace()) {
    String header;
    cimmofMessages::getMessage(header, cimmofMessages::ADD_QUALIFIER);
    trace(header, "");
    if (qualifier) 
      qualifier->print(_cmdline->traceos()); 
  }
  if (_cmdline && _cmdline->syntax_only()) {
    return ret; 
  }
  try {
    ret = _repository->addQualifier(qualifier);
  } catch(Exception e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(qualifier->getName());
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message, cimmofMessages::ADD_QUALIFIER_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  if (_cmdline && _cmdline->trace()) {
    String ok;
    cimmofMessages::getMessage(ok, cimmofMessages::TAB_OK);
    trace(ok, "");
  }
  return ret;
}

//--------------------------------------------------------------------
// When a qualifier itself (not its declaration) is detected,
// create the CIMQualifier object.
//--------------------------------------------------------------------
CIMQualifier *
cimmofParser::newQualifier(const String &name, const CIMValue &value,
			   Uint32 flavor)
{
  CIMQualifier *q = 0;
  try {
    q = new CIMQualifier(name, value, flavor);
  } catch(Exception &e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(name);
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message, cimmofMessages::NEW_QUALIFIER_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 

  return q;
}

//----------------------------------------------------------------------
// When a property of a class being declared is discovered, creat the
// new CIMProperty object.
//----------------------------------------------------------------------
CIMProperty *
cimmofParser::newProperty(const String &name, const CIMValue &val,
			  const String &referencedObject)
{
  CIMProperty *p = 0; 
  try {
    p = new CIMProperty(name, val, 0, referencedObject);
  } catch(Exception &e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(name);
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message, cimmofMessages::NEW_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 
  return p;
}

//-----------------------------------------------------------------------
// When a property production is complete, apply it to the
// class being declared.
//-----------------------------------------------------------------------
int
cimmofParser::applyProperty(CIMClass &c, CIMProperty &p)
{
  cimmofMessages::arglist arglist;
  arglist.push_back(c.getClassName());
  arglist.push_back(p.getName());
  String message;
  try {
    c.addProperty(p);
  } catch(UnitializedHandle) {
    cimmofMessages::getMessage(message,
			       cimmofMessages::UNINITIALIZED_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } catch(AlreadyExists) {
    cimmofMessages::getMessage(message,
			       cimmofMessages::PROPERTY_ALREADY_EXISTS_WARNING,
			       arglist);
    wlog(message);
  } catch(Exception &e) {
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message, 
			       cimmofMessages::APPLYING_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 

  return 0;
}

//-----------------------------------------------------------------------
// When a property production is complete, apply it to the
// instance being declared.
//-----------------------------------------------------------------------
int
cimmofParser::applyProperty(CIMInstance &i, CIMProperty &p)
{
  cimmofMessages::arglist arglist;
  const String &propertyName = p.getName();
  arglist.push_back(i.getClassName());
  arglist.push_back(propertyName);
  String message;
  try {
    Uint32 pos = i.findProperty(propertyName);
    if (pos == (Uint32)-1) {
      i.addProperty(p);   // Add the property
    } else {
      // FIXME:  There doesn't seem to be a way to change a property value
      // yet.
    }
  } catch (Exception &e) {
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message, 
			       cimmofMessages::APPLY_INSTANCE_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  return 0;
}
    
//----------------------------------------------------------------------
// When a property object's value changes, create a clone of the
// property object with a new value
//----------------------------------------------------------------------
CIMProperty *
cimmofParser::copyPropertyWithNewValue(const CIMProperty &p,
				       const CIMValue &v) const
{
  cimmofMessages::arglist arglist;
  String message;
  CIMProperty *newprop;
  arglist.push_back(p.getName());  
  try {
    newprop = new CIMProperty(p);
    newprop->setValue(v);
  } catch (Exception &e) {
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message, 
			       cimmofMessages::CLONING_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  return newprop;
}
    
//----------------------------------------------------------------------
// When a method of a class being declared is discovered, create the
// new CIMMethod object.
//----------------------------------------------------------------------
CIMMethod *
cimmofParser::newMethod(const String &name, const CIMType type)
{ 
  CIMMethod *m = 0;
  try {
    m = new CIMMethod(name, type);
  } catch(Exception &e) {
    cimmofMessages::arglist arglist;
    String message;
    arglist.push_back(name);
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message, cimmofMessages::NEW_METHOD_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 
  return m;
}

//-----------------------------------------------------------------------
// When a method production is complete, apply it to the
// class being declared.
//-----------------------------------------------------------------------
int
cimmofParser::applyMethod(CIMClass &c, CIMMethod &m) {
  cimmofMessages::arglist arglist;
  String message;
  arglist.push_back(m.getName());
  arglist.push_back(c.getClassName());
  try {
    c.addMethod(m);
  } catch(UnitializedHandle) {
    cimmofMessages::getMessage(message, 
			       cimmofMessages::UNINITIALIZED_PARAMETER_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } catch(AlreadyExists) {
    cimmofMessages::getMessage(message,
			       cimmofMessages::METHOD_ALREADY_EXISTS_WARNING,
			       arglist);
    wlog(message);
  } catch(Exception &e) {
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message,
			       cimmofMessages::APPLY_METHOD_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 

  return 0;
}

CIMParameter *
cimmofParser::newParameter(const String &name, const CIMType type,
			   bool isArray, Uint32 array, const String &objName)
{
  CIMParameter *p = 0;
  try {
    p = new CIMParameter(name, type, isArray, array, objName);
  } catch(Exception &e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(name);
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message, cimmofMessages::NEW_PARAMETER_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 
  return p;
}

int
cimmofParser::applyParameter(CIMMethod &m, CIMParameter &p) {
  try {
    m.addParameter(p);
  } catch(CIMException &e) {
    cimmofMessages::arglist arglist;
    String message;
    arglist.push_back(p.getName());
    arglist.push_back(m.getName());
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message, cimmofMessages::APPLY_PARAMETER_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 
  return 0;
}



CIMValue *
cimmofParser::QualifierValue(const String &qualifierName, const String &valstr)
{
  // FIXME:  Needs try/catch
  CIMQualifierDecl q = _repository->getQualifierDecl(qualifierName);
  CIMValue v = q.getValue();
  Uint32 asize = v.getArraySize();
  if (String::equal(valstr, String::EMPTY)) {
    if (v.getType() == CIMType::BOOLEAN) {
      Boolean b;
      v.get(b);
      v.set(!b);
    }
    return new CIMValue(v);
  } else {
    return valueFactory::createValue(v.getType(),
				     v.isArray() ? (int)asize : -1, 
				     &valstr);
  }
}

CIMProperty *
cimmofParser::PropertyFromInstance(const CIMInstance &instance,
				  const String &propertyName) const
{
  cimmofMessages::arglist arglist;
  arglist.push_back(propertyName);
  String className, message;
  try {
    Uint32 pos = instance.findProperty(propertyName);
    if (pos != (Uint32)-1) {
      //FIXME:  There doesn't seem to be a way to get a copy of an 
      // instance's properties (or to change them if you got one)
      //ConstCIMProperty oldp = instance.getProperty(pos);
      //CIMProperty *p = new CIMProperty(oldp);
      //return p;
    }
  } catch (Exception &e) {
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message,
			       cimmofMessages::GET_INSTANCE_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  // That didn't work.  Try getting the property from the class decl
  try {
    className = instance.getClassName();
  } catch (Exception &e) {
    arglist.push_back(className);
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message,
			       cimmofMessages::FIND_CLASS_OF_INSTANCE_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  // OK, we got the className.  Use it to find the class object.
  try {
    Array<String> propertyList;
    propertyList.append(propertyName);
    const CIMClass &c = _repository->getClass(getNamespacePath(), className,
					      true, false, false,
					      propertyList); 
    Uint32 pos = c.findProperty(propertyName);
    if (pos != (Uint32)-1) {
      // FIXME:  Can't get class's property either.
      // return new CIMProperty(c.getProperty(pos));
    }
  } catch (Exception &e) {
    arglist.push_back(getNamespacePath());
    arglist.push_back(className);
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message,
			       cimmofMessages::GET_CLASS_PROPERTY_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  CIMProperty *p = new CIMProperty();
  p->setName(propertyName);
  return p;
}

CIMValue *
cimmofParser::ValueFromProperty(const CIMProperty &prop) const
{
  String propname;
  try {
    propname = prop.getName();
    const CIMValue &v = prop.getValue();
    return new CIMValue(v);
  } catch (Exception &e) {
    cimmofMessages::arglist arglist;
    String message;
    arglist.push_back(propname);
    arglist.push_back(e.getMessage());
    cimmofMessages::getMessage(message,
			       cimmofMessages::GET_PROPERTY_VALUE_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  }
  return new CIMValue();
}


CIMValue *
cimmofParser::PropertyValueFromInstance(const CIMInstance &instance,
					const String &propertyName)const
{
  CIMProperty *prop = PropertyFromInstance(instance, propertyName);
  CIMValue *value = ValueFromProperty(*prop);
  delete prop;
  return value;
}
  
CIMReference *
cimmofParser::newReference(const Pegasus::objectName &oname)
{
  CIMReference *ref = 0;
  try {
    ref = new CIMReference(oname.host(), oname.handle(), oname.className(),
			   oname.KeyBindings());
  } catch(Exception &e) {
    cimmofMessages::arglist arglist;
    arglist.push_back(oname.className());
    arglist.push_back(e.getMessage());
    String message;
    cimmofMessages::getMessage(message, cimmofMessages::NEW_REFERENCE_ERROR,
			       arglist);
    elog(message);
    maybeThrowParseError(message);
  } 
  return ref;
}

void
cimmofParser::addClassAlias(const String &alias, const CIMClass *cd,
		       bool isInstance)
{
  // FIXME:  As soon as we figure out what Aliases are for, do something
}

void
cimmofParser::addInstanceAlias(const String &alias, const CIMInstance *cd,
		       bool isInstance)
{
  // FIXME:  As soon as we figure out what Aliases are for, do something
}



    //-------------------------------------------------------------------
    //  Class Private methods
    //-------------------------------------------------------------------
//--------------------------------------------------------------------
// Error logging
//--------------------------------------------------------------------
void
cimmofParser::elog(const String &msg) const
{
  if (_cmdline)
    if (!_cmdline->suppress_all_messages() && !_cmdline->suppress_warnings())
      _cmdline->erroros() << msg << endl;
}

//----------------------------------------------------------------------
// Warning logging
//----------------------------------------------------------------------
void
cimmofParser::wlog(const String &msg) const
{
  if (_cmdline)
    if (!_cmdline->suppress_all_messages())
      _cmdline->warningos() << msg << endl;
}

//-------------------------------------------------------------------
// Tracing
//-------------------------------------------------------------------
void
cimmofParser::trace(const String &head, const String &tail) const
{
  if (_cmdline)
    _cmdline->traceos() << head << " " << tail << endl;
}

//--------------------------------------------------------------------
// Exception Control
//--------------------------------------------------------------------
void
cimmofParser::maybeThrowParseError(const String &msg) const
{
  // unless the continue-processing flag is on
  throw ParseError(msg);
}

void
cimmofParser::maybeThrowLexerError(const String &msg) const
{
  // unless we may want to continue (and that's not possible with the
  // lexer written as it is now) (FIXME:),
  throw LexerError(msg);
}
