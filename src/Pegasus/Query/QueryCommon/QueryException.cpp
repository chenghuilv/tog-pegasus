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
// Author: Humberto Rivero (hurivero@us.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "QueryException.h"

PEGASUS_NAMESPACE_BEGIN

const char QueryException::MSG[] = "Query error: $0";
const char QueryException::KEY[] = "QueryCommon.QueryException.QUERY_EXCEPTION";

const char QueryLanguageInvalidException::MSG[] = "Query invalid language error: $0";
const char QueryLanguageInvalidException::KEY[] = "QueryCommon.QueryException.QUERY_LANGUAGE_INVALID_EXCEPTION";

const char QueryParseException::MSG[] = "Query parse error: $0";
const char QueryParseException::KEY[] = "QueryCommon.QueryException.QUERY_PARSE_EXCEPTION";

const char QueryValidationException::MSG[] = "Query validation error: $0";
const char QueryValidationException::KEY[] = "QueryCommon.QueryException.QUERY_VALIDATION_EXCEPTION";

const char QueryRuntimeException::MSG[] = "Query runtime error: $0";
const char QueryRuntimeException::KEY[] = "QueryCommon.QueryException.QUERY_RUNTIME_EXCEPTION";

const char QueryRuntimePropertyException::MSG[] = "Query property error: $0";
const char QueryRuntimePropertyException::KEY[] = "QueryCommon.QueryException.QUERY_RUNTIME_PROPERTY_EXCEPTION";

const char QueryMissingPropertyException::MSG[] = "Query missing property error: $0";
const char QueryMissingPropertyException::KEY[] = "QueryCommon.QueryException.QUERY_MISSINGPROPERTY_EXCEPTION";

const char CQLChainedIdParseException::MSG[] = "CQL chained identifier parse error: $0";
const char CQLChainedIdParseException::KEY[] = "QueryCommon.QueryException.CQL_CHAINED_ID_EXCEPTION";

const char CQLIdentifierParseException::MSG[] = "CQL identifier parse error: $0";
const char CQLIdentifierParseException::KEY[] = "QueryCommon.QueryException.CQL_IDENTIFIER_EXCEPTION";

const char CQLRuntimeException::MSG[] = "CQL runtime error: $0";
const char CQLRuntimeException::KEY[] = "QueryCommon.QueryException.CQL_RUNTIME_EXCEPTION";

const char CQLSyntaxErrorException::MSG[] = "CQL syntax error: $0 around token $1 in position $2 while processing rule $3";
const char CQLSyntaxErrorException::KEY[] = "QueryCommon.QueryException.CQL_SYNTAX_ERROR_EXCEPTION";

const char CQLNullContagionException::MSG[] = "CQL null evaluation error: $0";
const char CQLNullContagionException::KEY[] = "QueryCommon.QueryException.CQL_NULL_CONTAGION_EXCEPTION";

PEGASUS_NAMESPACE_END
