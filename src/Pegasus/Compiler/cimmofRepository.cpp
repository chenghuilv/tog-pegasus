//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 The Open group, BMC Software, Tivoli Systems, IBM
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
// Author: Bob Blair (bblair@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////


//
// implementation of  cimmofRepository
//
//
//
// This class acts as a buffer between the compiler and the repository
// interface.  Thie main thing it does is registers a non-standard
// DeclContext so we can do local checking of context for new objects
//

#include "cimmofRepository.h"

PEGASUS_USING_PEGASUS;

cimmofRepository::cimmofRepository(const String &path, 
				   compilerCommonDefs::operationType ot) :
  _cimrepository(0), _context(0), _ot(ot)
{
  // don't catch the exceptions that might be thrown.  They should go up.
  if (_ot != compilerCommonDefs::IGNORE_REPOSITORY) {
    _cimrepository = new CIMRepository(Cat(path, "/repository"));
  }
  _context = new compilerDeclContext(_cimrepository, _ot);
  if (_cimrepository)
    _cimrepository->setDeclContext(_context);
}

cimmofRepository::~cimmofRepository() {
  if (_cimrepository)
    delete(_cimrepository);
  if (_context)
    delete(_context);
}

int 
cimmofRepository::addClass(const String &nameSpace, CIMClass *classdecl)
{
  // Don't catch errors: pass them up to the requester
  _context->addClass( nameSpace,  *classdecl);
  return 0;
}


int 
cimmofRepository::addInstance(const String &nameSpace, CIMInstance *instance)
{ 
  // Don't catch errors: pass them up to the requester
  _context->addInstance(nameSpace, *instance);
  return 0;
}

int 
cimmofRepository::addQualifier(const String &nameSpace,
			       CIMQualifierDecl *qualifier)
{ 
  // Don't catch errors: pass them up to the requester
  _context->addQualifierDecl(nameSpace, *qualifier);
  return 0;
}

CIMQualifierDecl
cimmofRepository::getQualifierDecl(const String &nameSpace, const String &name)
{
  // Don't catch errors: pass them up to the requester
  return _context->lookupQualifierDecl(nameSpace, name);
}

CIMClass
cimmofRepository::getClass(const String &nameSpace, const String &classname)
{
  // Don't catch errors: pass them up to the requester
  return _context->lookupClass(nameSpace, classname);
}

void 
cimmofRepository::createNameSpace(const String &nameSpaceName)
{
  if (_cimrepository && _ot != compilerCommonDefs::IGNORE_REPOSITORY)
    _cimrepository->createNameSpace(nameSpaceName);
}
