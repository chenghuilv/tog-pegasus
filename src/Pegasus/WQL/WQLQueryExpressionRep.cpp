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
// Author: Adrian Schuur (schuur@de.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "WQLQueryExpressionRep.h"
#include "WQLParser.h"
#include "WQLInstancePropertySource.h"

PEGASUS_NAMESPACE_BEGIN


const CIMPropertyList WQLQueryExpressionRep::getPropertyList() const
{
   if (_stmt==NULL) ((WQLQueryExpressionRep*)this)->_parse();

   return _stmt->getSelectPropertyList();
}

WQLQueryExpressionRep::~WQLQueryExpressionRep() {
   if (_stmt) delete _stmt;
}

void WQLQueryExpressionRep::_parse()
{
   if (_queryLanguage=="WQL") {
      _stmt=new WQLSelectStatement();
      WQLParser::parse(_query, *_stmt);
   }
   else throw
     PEGASUS_CIM_EXCEPTION(CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED, _queryLanguage);
}

Boolean WQLQueryExpressionRep::evaluate(const CIMInstance & inst) const
{
    if (_stmt==NULL) ((WQLQueryExpressionRep*)this)->_parse();

    WQLInstancePropertySource ips(inst);
    try {
       if (_stmt->evaluateWhereClause(&ips)) return true;
    }
    catch (...) {
         return false;
    }
    return false; 
}

void WQLQueryExpressionRep::applyProjection(CIMInstance & ci,
    Boolean allowMissing)
{
   if (_stmt==NULL) ((WQLQueryExpressionRep*)this)->_parse();

   _stmt->applyProjection(ci, allowMissing);
}

PEGASUS_NAMESPACE_END
