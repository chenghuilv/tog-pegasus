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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/TCPChannel.h>
#include <Pegasus/Common/Selector.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Protocol/Handler.h>
#include "CIMClient.h"

#include <iostream>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//#define DDD(X) X
#define DDD(X) // X
////////////////////////////////////////////////////////////////////////////////
//
// GetClassResult
//
////////////////////////////////////////////////////////////////////////////////

struct GetClassResult
{
    CIMStatusCode code;
    CIMClass cimClass;
};

////////////////////////////////////////////////////////////////////////////////
//
// GetInstanceResult
//
////////////////////////////////////////////////////////////////////////////////

struct GetInstanceResult
{
    CIMStatusCode code;
    CIMInstance cimInstance;
};

//STUB{
////////////////////////////////////////////////////////////////////////////////
//
// EnumerateClassNamesResult
//
////////////////////////////////////////////////////////////////////////////////

struct EnumerateClassNamesResult
{
    CIMStatusCode code;
    Array<String> classNames;
};
//STUB}

////////////////////////////////////////////////////////////////////////////////
//
// ReferencesResult
//
////////////////////////////////////////////////////////////////////////////////

struct ReferencesResult
{
    CIMStatusCode code;
    Array<CIMObjectWithPath> objectWithPathArray;
};

////////////////////////////////////////////////////////////////////////////////
//
// ReferenceNamesResult
//
////////////////////////////////////////////////////////////////////////////////

struct ReferenceNamesResult
{
    CIMStatusCode code;
    Array<CIMReference> objectPaths;
};

////////////////////////////////////////////////////////////////////////////////
//
// AssociatorNamesResult
//
////////////////////////////////////////////////////////////////////////////////

struct AssociatorNamesResult
{
    CIMStatusCode code;
    Array<CIMReference> objectPaths;
};

////////////////////////////////////////////////////////////////////////////////
//
// AssociatorsResult
//
////////////////////////////////////////////////////////////////////////////////

struct AssociatorsResult
{
    CIMStatusCode code;
    Array<CIMObjectWithPath> objectWithPathArray;
};

////////////////////////////////////////////////////////////////////////////////
//
// CreateInstanceResult
//
////////////////////////////////////////////////////////////////////////////////

struct CreateInstanceResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// EnumerateInstanceNamesResult
//
////////////////////////////////////////////////////////////////////////////////

struct EnumerateInstanceNamesResult
{
    CIMStatusCode code;
    Array<CIMReference> instanceNames;
};

////////////////////////////////////////////////////////////////////////////////
//
// DeleteQualifierResult
//
////////////////////////////////////////////////////////////////////////////////

struct DeleteQualifierResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// GetQualifierResult
//
////////////////////////////////////////////////////////////////////////////////

struct GetQualifierResult
{
    CIMStatusCode code;
    CIMQualifierDecl qualifierDecl;
};

////////////////////////////////////////////////////////////////////////////////
//
// SetQualifierResult
//
////////////////////////////////////////////////////////////////////////////////

struct SetQualifierResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// EnumerateQualifiersResult
//
////////////////////////////////////////////////////////////////////////////////

struct EnumerateQualifiersResult
{
    CIMStatusCode code;
    Array<CIMQualifierDecl> qualifierDecls;
};

////////////////////////////////////////////////////////////////////////////////
//
// EnumerateClassesResult
//
////////////////////////////////////////////////////////////////////////////////

struct EnumerateClassesResult
{
    CIMStatusCode code;
    Array<CIMClass> classDecls;
};

////////////////////////////////////////////////////////////////////////////////
//
// CreateClassResult
//
////////////////////////////////////////////////////////////////////////////////

struct CreateClassResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// ModifyClassResult
//
////////////////////////////////////////////////////////////////////////////////

struct ModifyClassResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// ModifyInstanceResult
//
////////////////////////////////////////////////////////////////////////////////

struct ModifyInstanceResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// DeleteClassResult
//
////////////////////////////////////////////////////////////////////////////////

struct DeleteClassResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// DeleteInstanceResult
//
////////////////////////////////////////////////////////////////////////////////

struct DeleteInstanceResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// GetPropertyResult
//
////////////////////////////////////////////////////////////////////////////////
// ATTN: The following is not correct or complete yet
struct GetPropertyResult
{
    CIMStatusCode code;
    CIMValue cimValue;
};

////////////////////////////////////////////////////////////////////////////////
//
// SetPropertyResult
//
////////////////////////////////////////////////////////////////////////////////
// ATTN: The following is not correct or complete yet.
struct SetPropertyResult
{
    CIMStatusCode code;
};

////////////////////////////////////////////////////////////////////////////////
//
// ClientHandler
//
////////////////////////////////////////////////////////////////////////////////

class ClientHandler : public Handler
{
public:

    ClientHandler(Selector* selector)
	: _getClassResult(0), _blocked(false), _selector(selector)
    {

    }

    const char* getHostName() const;

    virtual int handleMessage();

    int handleMethodResponse();

    int handleGetClassResponse(XmlParser& parser, const String& messageId);

    int handleGetInstanceResponse(XmlParser& parser, const String& messageId);

    //STUB{
    int handleEnumerateClassNamesResponse(
	XmlParser& parser, 
	const String& messageId);
    //STUB}

    int handleReferencesResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleReferenceNamesResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleAssociatorNamesResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleAssociatorsResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleCreateInstanceResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleEnumerateInstanceNamesResponse(
	XmlParser& parser,
	const String& messageId);

    int handleDeleteQualifierResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleGetQualifierResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleSetQualifierResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleEnumerateQualifiersResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleEnumerateClassesResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleCreateClassResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleModifyClassResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleModifyInstanceResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleDeleteClassResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleDeleteInstanceResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleGetPropertyResponse(
	XmlParser& parser, 
	const String& messageId);

    int handleSetPropertyResponse(
	XmlParser& parser, 
	const String& messageId);

    Boolean waitForResponse(
	Uint32 timeOutMilliseconds);

    union
    {
	GetClassResult* _getClassResult;
	GetInstanceResult* _getInstanceResult;
	//STUB{
	EnumerateClassNamesResult* _enumerateClassNamesResult;
	//STUB}
	ReferencesResult* _referencesResult;
	ReferenceNamesResult* _referenceNamesResult;
	AssociatorNamesResult* _associatorNamesResult;
	AssociatorsResult* _associatorsResult;
	CreateInstanceResult* _createInstanceResult;
	EnumerateInstanceNamesResult* _enumerateInstanceNamesResult;
	DeleteQualifierResult* _deleteQualifierResult;
	GetQualifierResult* _getQualifierResult;
	SetQualifierResult* _setQualifierResult;
	EnumerateQualifiersResult* _enumerateQualifiersResult;
	EnumerateClassesResult* _enumerateClassesResult;
	CreateClassResult* _createClassResult;
	ModifyClassResult* _modifyClassResult;
	ModifyInstanceResult* _modifyInstanceResult;
	DeleteClassResult* _deleteClassResult;
	DeleteInstanceResult* _deleteInstanceResult;
	GetPropertyResult* _getPropertyResult;
	SetPropertyResult* _setPropertyResult;
    };

private:
    Boolean _blocked;
    char _hostNameTmp[256];
    Selector* _selector;
};

int ClientHandler::handleMessage()
{
    // cout << "ClientHandler::handleMessage()" << endl;

    Handler::handleMessage();

    const char* cimOperation = getFieldValueSubString("CIMOperation:");

    if (cimOperation && strcmp(cimOperation, "MethodResponse") == 0)
    {
	try
	{
	    handleMethodResponse();
	}
	catch (Exception& e)
	{
	    PEGASUS_STD(cerr) << "Error: "
		<< e.getMessage() << PEGASUS_STD(endl);
	}
    }

    return 0;
}

int ClientHandler::handleMethodResponse()
{
    //--------------------------------------------------------------------------
    // Create a parser:
    //--------------------------------------------------------------------------

    _message.append('\0');
    XmlParser parser((char*)getContent());
    XmlEntry entry;

    //--------------------------------------------------------------------------
    // Expect "<?xml ...>":
    //--------------------------------------------------------------------------

    XmlReader::expectXmlDeclaration(parser, entry);

    //--------------------------------------------------------------------------
    // <!ELEMENT CIM (MESSAGE|DECLARATION)>
    // <!ATTRLIST CIM
    //     CIMVERSION CDATA #REQUIRED
    //     DTDVERSION CDATA #REQUIRED>
    //--------------------------------------------------------------------------

    XmlReader::testCimStartTag(parser);

    //--------------------------------------------------------------------------
    // <!ELEMENT MESSAGE (SIMPLEREQ|MULTIREQ|SIMPLERSP|MULTIRSP)>
    // <!ATTLIST MESSAGE
    //     ID CDATA #REQUIRED
    //     PROTOCOLVERSION CDATA #REQUIRED>
    //
    // ATTN: Handle MULTIREQ, SIMPLERSP, and MULTIRSP.
    //--------------------------------------------------------------------------

    String messageId;
    const char* protocolVersion = 0;

    if (!XmlReader::getMessageStartTag(parser, messageId, protocolVersion))
	throw XmlValidationError(parser.getLine(), "expected MESSAGE element");

    if (strcmp(protocolVersion, "1.0") != 0)
    {
	throw XmlSemanticError(parser.getLine(),
	    "Expected MESSAGE.PROTOCOLVERSION to be \"1.0\"");
    }

    //--------------------------------------------------------------------------
    // <!ELEMENT SIMPLERSP (IMETHODRESPONSE|METHODRESPONSE)>
    //--------------------------------------------------------------------------

    XmlReader::expectStartTag(parser, entry, "SIMPLERSP");

    //--------------------------------------------------------------------------
    // <!ELEMENT IMETHODRESPONSE (ERROR|IRETURNVALUE?)>
    // <!ATTLIST IMETHODRESPONSE %CIMName;>
    //--------------------------------------------------------------------------

    const char* iMethodResponseName = 0;

    if (!XmlReader::getIMethodResponseStartTag(parser, iMethodResponseName))
	throw XmlValidationError(parser.getLine(),
	    "expected IMETHODRESPONSE element");

    //--------------------------------------------------------------------------
    // Dispatch the method:
    //--------------------------------------------------------------------------

    if (strcmp(iMethodResponseName, "GetClass") == 0)
	handleGetClassResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "GetInstance") == 0)
	handleGetInstanceResponse(parser, messageId);
    //STUB{
    else if (strcmp(iMethodResponseName, "EnumerateClassNames") == 0)
	handleEnumerateClassNamesResponse(parser, messageId);
    //STUB}
    else if (strcmp(iMethodResponseName, "References") == 0)
	handleReferencesResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "ReferenceNames") == 0)
	handleReferenceNamesResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "AssociatorNames") == 0)
	handleAssociatorNamesResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "Associators") == 0)
	handleAssociatorsResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "CreateInstance") == 0)
	handleCreateInstanceResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "EnumerateInstanceNames") == 0)
	handleEnumerateInstanceNamesResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "DeleteQualifier") == 0)
	handleDeleteQualifierResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "GetQualifier") == 0)
	handleGetQualifierResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "SetQualifier") == 0)
	handleSetQualifierResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "EnumerateQualifiers") == 0)
	handleEnumerateQualifiersResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "EnumerateClasses") == 0)
	handleEnumerateClassesResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "CreateClass") == 0)
	handleCreateClassResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "ModifyClass") == 0)
	handleModifyClassResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "ModifyInstance") == 0)
	handleModifyInstanceResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "DeleteClass") == 0)
	handleDeleteClassResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "DeleteInstance") == 0)
	handleDeleteInstanceResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "GetProperty") == 0)
	handleGetPropertyResponse(parser, messageId);
    else if (strcmp(iMethodResponseName, "SetProperty") == 0)
	handleSetPropertyResponse(parser, messageId);

    //--------------------------------------------------------------------------
    // Handle end tags:
    //--------------------------------------------------------------------------

    XmlReader::expectEndTag(parser, "IMETHODRESPONSE");
    XmlReader::expectEndTag(parser, "SIMPLERSP");
    XmlReader::expectEndTag(parser, "MESSAGE");
    XmlReader::expectEndTag(parser, "CIM");

    return 0;
}

int ClientHandler::handleGetClassResponse(
    XmlParser& parser, 
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_getClassResult = new GetClassResult;
	_getClassResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	CIMClass cimClass;

	if (!XmlReader::getClassElement(parser, cimClass))
	    throw XmlValidationError(parser.getLine(),"expected CLASS element");

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_getClassResult = new GetClassResult;
	_getClassResult->code = CIM_ERR_SUCCESS;
	_getClassResult->cimClass = cimClass;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleGetInstanceResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_getInstanceResult = new GetInstanceResult;
	_getInstanceResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	CIMInstance cimInstance;

	if (!XmlReader::getInstanceElement(parser, cimInstance))
	{
	    throw XmlValidationError(
		parser.getLine(), "expected INSTANCE element");
	}

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_getInstanceResult = new GetInstanceResult;
	_getInstanceResult->code = CIM_ERR_SUCCESS;
	_getInstanceResult->cimInstance = cimInstance;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

//STUB{

int ClientHandler::handleEnumerateClassNamesResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_enumerateClassNamesResult = new EnumerateClassNamesResult;
	_enumerateClassNamesResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	Array<String> classNames;
	String className;

	while (XmlReader::getClassNameElement(parser, className, false))
	    classNames.append(className);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_enumerateClassNamesResult = new EnumerateClassNamesResult;
	_enumerateClassNamesResult->code = CIM_ERR_SUCCESS;
	_enumerateClassNamesResult->classNames = classNames;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}
//STUB}

int ClientHandler::handleReferencesResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code = CIM_ERR_SUCCESS;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_referencesResult = new ReferencesResult;
	_referencesResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	Array<CIMObjectWithPath> objectWithPathArray;
	CIMObjectWithPath tmp;

	while (XmlReader::getObjectWithPath(parser, tmp))
	    objectWithPathArray.append(tmp);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_referencesResult = new ReferencesResult;
	_referencesResult->code = code;
	_referencesResult->objectWithPathArray = objectWithPathArray;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleReferenceNamesResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_referenceNamesResult = new ReferenceNamesResult;
	_referenceNamesResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	CIMReference objectPath;
	Array<CIMReference> objectPaths;

	while (XmlReader::getObjectPathElement(parser, objectPath))
	    objectPaths.append(objectPath);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_referenceNamesResult = new ReferenceNamesResult;
	_referenceNamesResult->code = CIM_ERR_SUCCESS;
	_referenceNamesResult->objectPaths = objectPaths;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleAssociatorNamesResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_associatorNamesResult = new AssociatorNamesResult;
	_associatorNamesResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	CIMReference objectPath;
	Array<CIMReference> objectPaths;

	while (XmlReader::getObjectPathElement(parser, objectPath))
	    objectPaths.append(objectPath);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_associatorNamesResult = new AssociatorNamesResult;
	_associatorNamesResult->code = CIM_ERR_SUCCESS;
	_associatorNamesResult->objectPaths = objectPaths;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleAssociatorsResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code = CIM_ERR_SUCCESS;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_associatorsResult = new AssociatorsResult;
	_associatorsResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	Array<CIMObjectWithPath> objectWithPathArray;
	CIMObjectWithPath tmp;

	while (XmlReader::getObjectWithPath(parser, tmp))
	    objectWithPathArray.append(tmp);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_associatorsResult = new AssociatorsResult;
	_associatorsResult->code = code;
	_associatorsResult->objectWithPathArray = objectWithPathArray;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleCreateInstanceResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_createInstanceResult = new CreateInstanceResult;
	_createInstanceResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_createInstanceResult = new CreateInstanceResult;
	_createInstanceResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleEnumerateInstanceNamesResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_enumerateInstanceNamesResult = new EnumerateInstanceNamesResult;
	_enumerateInstanceNamesResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	Array<CIMReference> instanceNames;
	String className;
	Array<KeyBinding> keyBindings;

	while (XmlReader::getInstanceNameElement(
	    parser, className, keyBindings))
	{
	    CIMReference r(
		String::EMPTY,
		String::EMPTY,
		className,
		keyBindings);
	    instanceNames.append(r);
	}

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_enumerateInstanceNamesResult = new EnumerateInstanceNamesResult;
	_enumerateInstanceNamesResult->code = CIM_ERR_SUCCESS;
	_enumerateInstanceNamesResult->instanceNames = instanceNames;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleDeleteQualifierResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_deleteQualifierResult = new DeleteQualifierResult;
	_deleteQualifierResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_deleteQualifierResult = new DeleteQualifierResult;
	_deleteQualifierResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleGetQualifierResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_getQualifierResult = new GetQualifierResult;
	_getQualifierResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	CIMQualifierDecl qualifierDecl;
	XmlReader::getQualifierDeclElement(parser, qualifierDecl);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_getQualifierResult = new GetQualifierResult;
	_getQualifierResult->code = CIM_ERR_SUCCESS;
	_getQualifierResult->qualifierDecl = qualifierDecl;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleSetQualifierResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_setQualifierResult = new SetQualifierResult;
	_setQualifierResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_setQualifierResult = new SetQualifierResult;
	_setQualifierResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleEnumerateQualifiersResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_enumerateQualifiersResult = new EnumerateQualifiersResult;
	_enumerateQualifiersResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	Array<CIMQualifierDecl> qualifierDecls;
	CIMQualifierDecl qualifierDecl;

	while (XmlReader::getQualifierDeclElement(parser, qualifierDecl))
	    qualifierDecls.append(qualifierDecl);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_enumerateQualifiersResult = new EnumerateQualifiersResult;
	_enumerateQualifiersResult->code = CIM_ERR_SUCCESS;
	_enumerateQualifiersResult->qualifierDecls = qualifierDecls;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleEnumerateClassesResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_enumerateClassesResult = new EnumerateClassesResult;
	_enumerateClassesResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	Array<CIMClass> classDecls;
	CIMClass cimClass;

	while (XmlReader::getClassElement(parser, cimClass))
	    classDecls.append(cimClass);

	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_enumerateClassesResult = new EnumerateClassesResult;
	_enumerateClassesResult->code = CIM_ERR_SUCCESS;
	_enumerateClassesResult->classDecls = classDecls;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleCreateClassResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_createClassResult = new CreateClassResult;
	_createClassResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_createClassResult = new CreateClassResult;
	_createClassResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleModifyClassResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_modifyClassResult = new ModifyClassResult;
	_modifyClassResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_modifyClassResult = new ModifyClassResult;
	_modifyClassResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleModifyInstanceResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_modifyInstanceResult = new ModifyInstanceResult;
	_modifyInstanceResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_modifyInstanceResult = new ModifyInstanceResult;
	_modifyInstanceResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleDeleteClassResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_deleteClassResult = new DeleteClassResult;
	_deleteClassResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_deleteClassResult = new DeleteClassResult;
	_deleteClassResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleDeleteInstanceResponse(
    XmlParser& parser,
    const String& messageId)
{
    XmlEntry entry;
    CIMStatusCode code;
    const char* description = 0;

    if (XmlReader::getErrorElement(parser, code, description))
    {
	_deleteInstanceResult = new DeleteInstanceResult;
	_deleteInstanceResult->code = code;
	_blocked = false;
	return 0;
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
	XmlReader::testEndTag(parser, "IRETURNVALUE");

	_deleteInstanceResult = new DeleteInstanceResult;
	_deleteInstanceResult->code = CIM_ERR_SUCCESS;
	_blocked = false;
	return 0;
    }
    else
    {
	throw XmlValidationError(parser.getLine(),
	    "expected ERROR or IRETURNVALUE element");
    }

    PEGASUS_UNREACHABLE( return 0; )
}

int ClientHandler::handleGetPropertyResponse(
    XmlParser& parser,
    const String& messageId)
{
    return 0;
}

int ClientHandler::handleSetPropertyResponse(
    XmlParser& parser,
    const String& messageId)
{
    return 0;
}

Boolean ClientHandler::waitForResponse(Uint32 timeOutMilliseconds)
{
    _blocked = true;
    long rem = long(timeOutMilliseconds);

    while (_blocked)
    {
	TimeValue start = TimeValue::getCurrentTime();

	TimeValue tv;
	tv.fromMilliseconds(rem);

	_selector->select(tv.toMilliseconds());

	TimeValue stop = TimeValue::getCurrentTime();

	long diff = stop.toMilliseconds() - start.toMilliseconds();

	if (diff >= rem)
	    break;

	rem -= diff;
    }

    Boolean gotResponse = !_blocked;
    _blocked = false;
    return gotResponse;
}

////////////////////////////////////////////////////////////////////////////////
//
// ClientHandlerFactory
//
////////////////////////////////////////////////////////////////////////////////

class ClientHandlerFactory : public ChannelHandlerFactory
{
public:

    ClientHandlerFactory(Selector* selector) : _selector(selector) { }

    virtual ~ClientHandlerFactory() { }

    virtual ChannelHandler* create() { return new ClientHandler(_selector); }

private:

    Selector* _selector;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMClient
//
////////////////////////////////////////////////////////////////////////////////


CIMClient::CIMClient(
    Selector* selector,
    Uint32 timeOutMilliseconds)
    : _channel(0), _timeOutMilliseconds(timeOutMilliseconds)
{
    _selector = selector;
}

CIMClient::~CIMClient()
{
    // Do not delete the selector here. It is owned by the application
    // and is passed into the constructor.
}

// ATTN-A: rework this to expose connector so that it may be used to
// make more than one client connetion. Also, expose the selector.

void CIMClient::connect(const char* address)
{
    if (_channel)
	throw AlreadyConnected();

    ChannelHandlerFactory* factory = new ClientHandlerFactory(_selector);

    TCPChannelConnector* connector
	= new TCPChannelConnector(factory, _selector);

    // ATTN-A: need connection timeout here:

    _channel = connector->connect(address);

if (!_channel)
	throw FailedToConnect();
}

void CIMClient::get(const char* document) const
{
    if (!_channel)
	throw NotConnected();

    Array<Sint8> message = XmlWriter::formatGetHeader(document);

    _channel->writeN(
	message.getData(), message.size());
}

void CIMClient::runOnce()
{
    const Uint32 MILLISECONDS_TIMEOUT = 5000;
    _selector->select(MILLISECONDS_TIMEOUT);
}

void CIMClient::runForever()
{
    for (;;)
	runOnce();
}

CIMClass CIMClient::getClass(
    const String& nameSpace,
    const String& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const Array<String>& propertyList)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    XmlWriter::appendClassNameParameter(
	params, "ClassName", className);
	
    if (localOnly != true)
	XmlWriter::appendBooleanParameter(
	    params, "LocalOnly", false);

    if (includeQualifiers != true)
	XmlWriter::appendBooleanParameter(
	    params, "IncludeQualifiers", false);

    if (includeClassOrigin != false)
	XmlWriter::appendBooleanParameter(
	    params, "IncludeClassOrigin", true);

    // ATTN-A: inject the real hostname here!

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(), nameSpace, "GetClass", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    GetClassResult* result = _getHandler()->_getClassResult;
    CIMClass cimClass = result->cimClass;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_getClassResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return cimClass;
}

CIMInstance CIMClient::getInstance(
    const String& nameSpace,
    const CIMReference& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const Array<String>& propertyList)
{
    // ATTN: the property list not passed here. Handle this later.
    // same story for getClass().

    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    XmlWriter::appendInstanceNameParameter(
	params, "InstanceName", instanceName);
	
    if (localOnly != true)
	XmlWriter::appendBooleanParameter(
	    params, "LocalOnly", false);

    if (includeQualifiers != false)
	XmlWriter::appendBooleanParameter(
	    params, "IncludeQualifiers", true);

    if (includeClassOrigin != false)
	XmlWriter::appendBooleanParameter(
	    params, "IncludeClassOrigin", true);

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(), nameSpace, "GetInstance", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    GetInstanceResult* result = _getHandler()->_getInstanceResult;
    CIMInstance cimInstance = result->cimInstance;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_getInstanceResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return cimInstance;
}

void CIMClient::deleteClass(
    const String& nameSpace,
    const String& className)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    if (className.size())
	XmlWriter::appendClassNameParameter(params, "ClassName", className);

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "DeleteClass", params);
	
    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    DeleteClassResult* result = _getHandler()->_deleteClassResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_deleteClassResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

void CIMClient::deleteInstance(
    const String& nameSpace,
    const CIMReference& instanceName)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    XmlWriter::appendInstanceNameParameter(
	params, "InstanceName", instanceName);

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(), nameSpace, "DeleteInstance", params);
	
    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    DeleteInstanceResult* result = _getHandler()->_deleteInstanceResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_deleteInstanceResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

void CIMClient::createClass(
    const String& nameSpace,
    const CIMClass& newClass)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;
    XmlWriter::appendClassParameter(params, "NewClass", newClass);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "CreateClass", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    CreateClassResult* result = _getHandler()->_createClassResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_createClassResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

void CIMClient::createInstance(
    const String& nameSpace,
    const CIMInstance& newInstance)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    XmlWriter::appendInstanceParameter(
	params, "NewInstance", newInstance);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "CreateInstance", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    CreateInstanceResult* result = _getHandler()->_createInstanceResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_createInstanceResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

void CIMClient::modifyClass(
    const String& nameSpace,
    const CIMClass& modifiedClass)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;
    XmlWriter::appendClassParameter(params, "ModifiedClass", modifiedClass);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "ModifyClass", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    ModifyClassResult* result = _getHandler()->_modifyClassResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_modifyClassResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

void CIMClient::modifyInstance(
    const String& nameSpace,
    const CIMInstance& modifiedInstance)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;
    XmlWriter::appendInstanceParameter(
	params, "ModifiedInstance", modifiedInstance);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(), nameSpace, "ModifyInstance", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    ModifyInstanceResult* result = _getHandler()->_modifyInstanceResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_modifyInstanceResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

Array<CIMClass> CIMClient::enumerateClasses(
    const String& nameSpace,
    const String& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers ,
    Boolean includeClassOrigin)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    if (className.size())
	XmlWriter::appendClassNameParameter(params, "ClassName", className);
	
    if (deepInheritance != false)
	XmlWriter::appendBooleanParameter(params, "DeepInheritance", true);

    if (localOnly != true)
	XmlWriter::appendBooleanParameter(params, "LocalOnly", false);

    if (includeQualifiers != true)
	XmlWriter::appendBooleanParameter(
	    params, "IncludeQualifiers", false);

    if (includeClassOrigin != false)
	XmlWriter::appendBooleanParameter(
	    params, "IncludeClassOrigin", true);

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "EnumerateClasses", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    EnumerateClassesResult* result = _getHandler()->_enumerateClassesResult;
    Array<CIMClass> classDecls = result->classDecls;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_enumerateClassesResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return classDecls;
}

Array<String> CIMClient::enumerateClassNames(
    const String& nameSpace,
    const String& className,
    Boolean deepInheritance)
{
//STUB{:
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    if (className.size())
	XmlWriter::appendClassNameParameter(params, "ClassName", className);
	
    if (deepInheritance != false)
	XmlWriter::appendBooleanParameter(params, "DeepInheritance", true);

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "EnumerateClassNames", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    EnumerateClassNamesResult* result 
	= _getHandler()->_enumerateClassNamesResult;

    Array<String> classNames = result->classNames;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_enumerateClassNamesResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return classNames;
//STUB}
}

Array<CIMInstance> CIMClient::enumerateInstances(
    const String& nameSpace,
    const String& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const Array<String>& propertyList)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
    return Array<CIMInstance>();
}

Array<CIMReference> CIMClient::enumerateInstanceNames(
    const String& nameSpace,
    const String& className)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    XmlWriter::appendClassNameParameter(params, "ClassName", className);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "EnumerateInstanceNames", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    EnumerateInstanceNamesResult* result
	= _getHandler()->_enumerateInstanceNamesResult;
    Array<CIMReference> instanceNames = result->instanceNames;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_enumerateInstanceNamesResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return instanceNames;
}

Array<CIMInstance> CIMClient::execQuery(
    const String& queryLanguage,
    const String& query)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
    return Array<CIMInstance>();
}

Array<CIMObjectWithPath> CIMClient::associators(
    const String& nameSpace,
    const CIMReference& objectName,
    const String& assocClass,
    const String& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const Array<String>& propertyList)
{
    String messageId = XmlWriter::getNextMessageId();
    Array<Sint8> params;

    // Append "ObjectName" parameter:

    XmlWriter::appendObjectNameParameter(params, "ObjectName", objectName);

    // Append "AssocClass" parameter:

    XmlWriter::appendClassNameParameter(params, "AssocClass", assocClass);

    // Append "ResultClass" parameter:

    XmlWriter::appendClassNameParameter(params, "ResultClass", resultClass);

    // Append "Role" parameter:

    XmlWriter::appendStringParameter(params, "Role", role);

    // Append "ResultRole" parameter:

    XmlWriter::appendStringParameter(params, "ResultRole", resultRole);

    // Append "IncludeQualifiers" paramter:

    if (includeQualifiers != false)
	XmlWriter::appendBooleanParameter(params, "IncludeQualifiers", true);

    if (includeClassOrigin != false)
	XmlWriter::appendBooleanParameter(params, "IncludeClassOrigin", true);

    // Format the message:

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(_getHostName(),
	nameSpace, "Associators", params);

    _channel->writeN(message.getData(), message.size());

    // message.append('\0');
    // cout << message.getData() << endl;

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    AssociatorsResult* result = _getHandler()->_associatorsResult;
    Array<CIMObjectWithPath> objectWithPathArray = result->objectWithPathArray;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_associatorsResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return objectWithPathArray;
}

Array<CIMReference> CIMClient::associatorNames(
    const String& nameSpace,
    const CIMReference& objectName,
    const String& assocClass,
    const String& resultClass,
    const String& role,
    const String& resultRole)
{
    String messageId = XmlWriter::getNextMessageId();
    Array<Sint8> params;

    XmlWriter::appendObjectNameParameter(params, "ObjectName", objectName);

    XmlWriter::appendClassNameParameter(params, "AssocClass", assocClass);

    XmlWriter::appendClassNameParameter(params, "ResultClass", resultClass);

    XmlWriter::appendStringParameter(params, "Role", role);

    XmlWriter::appendStringParameter(params, "ResultRole", resultRole);

    // Format and send the message:

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(_getHostName(),
	nameSpace, "AssociatorNames", params);

    _channel->writeN(message.getData(), message.size());

    // message.append('\0');
    // cout << message.getData() << endl;

    // Wait for response and then process result:

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    AssociatorNamesResult* result = _getHandler()->_associatorNamesResult;
    Array<CIMReference> objectPaths = result->objectPaths;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_associatorNamesResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return objectPaths;
}

Array<CIMObjectWithPath> CIMClient::references(
    const String& nameSpace,
    const CIMReference& objectName,
    const String& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const Array<String>& propertyList)
{
    String messageId = XmlWriter::getNextMessageId();
    Array<Sint8> params;

    // Append "ObjectName" parameter:

    XmlWriter::appendObjectNameParameter(params, "ObjectName", objectName);

    // Append "ResultClass" parameter:

    XmlWriter::appendClassNameParameter(params, "ResultClass", resultClass);

    // Append "Role" parameter:

    XmlWriter::appendStringParameter(params, "Role", role);

    // Append "IncludeQualifiers" paramter:

    if (includeQualifiers != false)
	XmlWriter::appendBooleanParameter(params, "IncludeQualifiers", true);

    if (includeClassOrigin != false)
	XmlWriter::appendBooleanParameter(params, "IncludeClassOrigin", true);

    // ATTN: process property list!

    // Format the message:

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(_getHostName(),
	nameSpace, "References", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    ReferencesResult* result = _getHandler()->_referencesResult;
    Array<CIMObjectWithPath> objectWithPathArray = result->objectWithPathArray;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_referencesResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return objectWithPathArray;
}

Array<CIMReference> CIMClient::referenceNames(
    const String& nameSpace,
    const CIMReference& objectName,
    const String& resultClass,
    const String& role)
{
    String messageId = XmlWriter::getNextMessageId();
    Array<Sint8> params;

    XmlWriter::appendObjectNameParameter(params, "ObjectName", objectName);

    XmlWriter::appendClassNameParameter(params, "ResultClass", resultClass);

    XmlWriter::appendStringParameter(params, "Role", role);

    // Format and send the message:

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(_getHostName(),
	nameSpace, "ReferenceNames", params);

    _channel->writeN(message.getData(), message.size());

    // Wait for response and then process result:

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    ReferenceNamesResult* result = _getHandler()->_referenceNamesResult;
    Array<CIMReference> objectPaths = result->objectPaths;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_referenceNamesResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return objectPaths;
}

CIMValue CIMClient::getProperty(
    const String& nameSpace,
    const CIMReference& instanceName,
    const String& propertyName)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
    return CIMValue();
}

void CIMClient::setProperty(
    const String& nameSpace,
    const CIMReference& instanceName,
    const String& propertyName,
    const CIMValue& newValue)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

CIMQualifierDecl CIMClient::getQualifier(
    const String& nameSpace,
    const String& qualifierName)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    // ATTN: no way in the specification to pass a qualifier name within
    // an IPARAMVALUE. Need help to solve this one.

    if (qualifierName.size())
	XmlWriter::appendClassNameParameter(
	    params, "QualifierName", qualifierName);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "GetQualifier", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    GetQualifierResult* result = _getHandler()->_getQualifierResult;
    CIMQualifierDecl qualifierDecl = result->qualifierDecl;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_getQualifierResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return qualifierDecl;
}

void CIMClient::setQualifier(
    const String& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;
    XmlWriter::appendQualifierDeclarationParameter(
	params, "QualifierDeclaration", qualifierDeclaration);

    {
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "SetQualifier", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    SetQualifierResult* result = _getHandler()->_setQualifierResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_setQualifierResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
    }

    int x = 5;
    x++;
}

void CIMClient::deleteQualifier(
    const String& nameSpace,
    const String& qualifierName)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    // ATTN: no way in the specification to pass a qualifier name within
    // an IPARAMVALUE. Need help to solve this one.

    if (qualifierName.size())
	XmlWriter::appendClassNameParameter(
	    params, "QualifierName", qualifierName);
	
    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(),
	nameSpace, "DeleteQualifier", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    DeleteQualifierResult* result = _getHandler()->_deleteQualifierResult;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_deleteQualifierResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);
}

Array<CIMQualifierDecl> CIMClient::enumerateQualifiers(
    const String& nameSpace)
{
    String messageId = XmlWriter::getNextMessageId();

    Array<Sint8> params;

    Array<Sint8> message = XmlWriter::formatSimpleReqMessage(
	_getHostName(), nameSpace, "EnumerateQualifiers", params);

    _channel->writeN(message.getData(), message.size());

    if (!_getHandler()->waitForResponse(_timeOutMilliseconds))
	throw TimedOut();

    EnumerateQualifiersResult* result 
	= _getHandler()->_enumerateQualifiersResult;

    Array<CIMQualifierDecl> qualifierDecls = result->qualifierDecls;
    CIMStatusCode code = result->code;
    delete result;
    _getHandler()->_enumerateQualifiersResult = 0;

    if (code != CIM_ERR_SUCCESS)
	throw CIMException(code);

    return qualifierDecls;
}

CIMValue CIMClient::invokeMethod(
    const String& nameSpace,
    const CIMReference& instanceName,
    const String& methodName,
    const Array<CIMValue>& inParameters,
    Array<CIMValue>& outParameters)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
    return CIMValue();
}

ClientHandler* CIMClient::_getHandler()
{
    return (ClientHandler*)_channel->getChannelHandler();
}

PEGASUS_NAMESPACE_END
