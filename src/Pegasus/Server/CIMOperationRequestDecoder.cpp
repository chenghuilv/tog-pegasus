//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 BMC Software, Hewlett-Packard Company, IBM,
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
// Modified By: Yi Zhou, Hewlett-Packard Company (yi_zhou@hp.com)
//              Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//              Jenny Yu, Hewlett-Packard Company (jenny_yu@hp.com)
//              Sushma Fernandes, Hewlett-Packard Company 
//              (sushma_fernandes@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/XmlConstants.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include "CIMOperationRequestDecoder.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMOperationRequestDecoder::CIMOperationRequestDecoder(
   MessageQueueService* outputQueue,
   Uint32 returnQueueId)
   :  Base("CIMOpRequestDecoder", MessageQueue::getNextQueueId()),
      _outputQueue(outputQueue),
      _returnQueueId(returnQueueId),
      _serverTerminating(false)

{
}

CIMOperationRequestDecoder::~CIMOperationRequestDecoder()
{

}

void CIMOperationRequestDecoder::sendResponse(
   Uint32 queueId, 
   Array<Sint8>& message)
{
   MessageQueue* queue = MessageQueue::lookup(queueId);

   if (queue)
   {
      HTTPMessage* httpMessage = new HTTPMessage(message);
      queue->enqueue(httpMessage);
   }
}

void CIMOperationRequestDecoder::sendIMethodError(
   Uint32 queueId, 
   const String& messageId,
   const String& iMethodName,
   CIMStatusCode code,
   const String& description) 
{
    Array<Sint8> message;
    message = XmlWriter::formatSimpleIMethodErrorRspMessage(
        iMethodName,
        messageId,
        code,
        description);

    sendResponse(queueId, message);
}

void CIMOperationRequestDecoder::sendMethodError(
   Uint32 queueId, 
   const String& messageId,
   const String& methodName,
   CIMStatusCode code,
   const String& description) 
{
    Array<Sint8> message;
    message = XmlWriter::formatSimpleMethodErrorRspMessage(
        methodName,
        messageId,
        code,
        description);
    
    sendResponse(queueId, message);
}

void CIMOperationRequestDecoder::handleEnqueue(Message *message)
{

   if (!message)
      return;

   switch (message->getType())
   {
      case HTTP_MESSAGE:
	 handleHTTPMessage((HTTPMessage*)message);
	 break;
   }

   delete message;
}


void CIMOperationRequestDecoder::handleEnqueue()
{
   Message* message = dequeue();
   if(message)
      handleEnqueue(message);
}

const char* CIMOperationRequestDecoder::getQueueName() const
{
   return "CIMOperationRequestDecoder";
}

//------------------------------------------------------------------------------
//
// From the HTTP/1.1 Specification (RFC 2626):
//
// Both types of message consist of a start-line, zero or more header fields 
// (also known as "headers"), an empty line (i.e., a line with nothing 
// preceding the CRLF) indicating the end of the header fields, and possibly 
// a message-body.
//
// Example CIM request:
//
//     M-POST /cimom HTTP/1.1 
//     HOST: www.erewhon.com 
//     Content-Type: application/xml; charset="utf-8" 
//     Content-Length: xxxx 
//     Man: http://www.dmtf.org/cim/operation ; ns=73 
//     73-CIMOperation: MethodCall 
//     73-CIMMethod: EnumerateInstances 
//     73-CIMObject: root/cimv2 
// 
//------------------------------------------------------------------------------

void CIMOperationRequestDecoder::handleHTTPMessage(HTTPMessage* httpMessage)
{
   // Save queueId:

   Uint32 queueId = httpMessage->queueId;

   // Save userName and authType:

   String userName;
   String authType = String::EMPTY;

   if ( httpMessage->authInfo->isAuthenticated() )
   {
      userName = httpMessage->authInfo->getAuthenticatedUser();
      authType = httpMessage->authInfo->getAuthType();
   }

   // Parse the HTTP message:

   String startLine;
   Array<HTTPHeader> headers;
   Sint8* content;
   Uint32 contentLength;

   PEG_FUNC_ENTER(TRC_DISPATCHER,"CIMOperationRequestDecoder::"
		  "handleHTTPMessage()");

   httpMessage->parse(startLine, headers, contentLength);

   // Parse the request line:

   String methodName;
   String requestUri;
   String httpVersion;

   Tracer::trace(TRC_XML_IO, Tracer::LEVEL2, "%s",
		 httpMessage->message.getData());

   HTTPMessage::parseRequestLine(
      startLine, methodName, requestUri, httpVersion);

   // Process M-POST and POST messages:


   if (methodName == "M-POST" || methodName == "POST")
   {
      // Search for "CIMOperation" header:

      String cimOperation;

      if (!HTTPMessage::lookupHeader(
	     headers, "*CIMOperation", cimOperation, true))
      {
	 // ATTN: error discarded at this time!
	 return;
      }

      // Zero-terminate the message:

      httpMessage->message.append('\0');

      // Calculate the beginning of the content from the message size and
      // the content length.  Subtract 1 to take into account the null
      // character we just added to the end of the message.

      content = (Sint8*) httpMessage->message.getData() +
	 httpMessage->message.size() - contentLength - 1;

      // If it is a method call, then dispatch it to be handled:

      if (!String::equalNoCase(cimOperation, "MethodCall"))
      {
	 // ATTN: error discarded at this time!
	 return;
      }

      handleMethodCall(queueId, content, authType, userName);
   }
    
   PEG_FUNC_EXIT(TRC_DISPATCHER,"CIMOperationRequestDecoder::"
		 "handleHTTPMessage()");
    
}


void CIMOperationRequestDecoder::handleMethodCall(
   Uint32 queueId,
   Sint8* content,
   String authType,
   String userName)
{
   Message* request;

   // Create a parser:

   XmlParser parser(content);
   XmlEntry entry;
   String messageId;
   const char* cimMethodName = "";

   try
   {
      // Expect <?xml ...>

      XmlReader::expectXmlDeclaration(parser, entry);

      // Expect <CIM ...>

      XmlReader::testCimStartTag(parser);

      // Expect <MESSAGE ...>

      const char* protocolVersion = 0;

      if (!XmlReader::getMessageStartTag(
	     parser, messageId, protocolVersion))
      {
	 throw XmlValidationError(
	    parser.getLine(), "expected MESSAGE element");
      }

      if (strcmp(protocolVersion, "1.0") != 0)
      {
	 throw XmlSemanticError(parser.getLine(), 
				"Expected MESSAGE.PROTOCOLVERSION to be \"1.0\"");
      }
   }
   catch (Exception&)
   {
      // ATTN: no error can be sent back to the client yet since
      // errors require a message-id (which was in the process of
      // being obtained above).
      return;
   }

   try
   {
      // Expect <SIMPLEREQ ...>

      XmlReader::expectStartTag(parser, entry, "SIMPLEREQ");

      // Expect <IMETHODCALL ...>

      if (XmlReader::getIMethodCallStartTag(parser, cimMethodName))
      {
	 // Expect <LOCALNAMESPACEPATH ...>

	 String nameSpace;

	 if (!XmlReader::getLocalNameSpacePathElement(parser, nameSpace))
	 {
	    throw XmlValidationError(parser.getLine(), 
				     "expected LOCALNAMESPACEPATH element");
	 }

	 // Delegate to appropriate method to handle:

	 //
	 // if CIMOM is shutting down, return error response
	 //
	 // ATTN:  Need to define a new CIM Error.
	 //
	 if (_serverTerminating)
	 {
	    String description = "CIMServer is shutting down.  ";
	    description.append("Request cannot be processed: ");
	    description += cimMethodName;

	    sendIMethodError(
	       queueId,
	       messageId,
	       cimMethodName,
	       CIM_ERR_FAILED,
	       description);

	    return;
	 }

	 if (CompareNoCase(cimMethodName, "GetClass") == 0)
	    request = decodeGetClassRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "GetInstance") == 0)
	    request = decodeGetInstanceRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "EnumerateClassNames") == 0)
	    request = decodeEnumerateClassNamesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "References") == 0)
	    request = decodeReferencesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "ReferenceNames") == 0)
	    request = decodeReferenceNamesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "AssociatorNames") == 0)
	    request = decodeAssociatorNamesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "Associators") == 0)
	    request = decodeAssociatorsRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "CreateInstance") == 0)
	    request = decodeCreateInstanceRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "EnumerateInstanceNames") == 0)
	    request = decodeEnumerateInstanceNamesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "DeleteQualifier") == 0)
	    request = decodeDeleteQualifierRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "GetQualifier") == 0)
	    request = decodeGetQualifierRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "SetQualifier") == 0)
	    request = decodeSetQualifierRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "EnumerateQualifiers") == 0)
	    request = decodeEnumerateQualifiersRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "EnumerateClasses") == 0)
	    request = decodeEnumerateClassesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "EnumerateClassNames") == 0)
	    request = decodeEnumerateClassNamesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "EnumerateInstances") == 0)
	    request = decodeEnumerateInstancesRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "CreateClass") == 0)
	    request = decodeCreateClassRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "ModifyClass") == 0)
	    request = decodeModifyClassRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "ModifyInstance") == 0)
	    request = decodeModifyInstanceRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "DeleteClass") == 0)
	    request = decodeDeleteClassRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "DeleteInstance") == 0)
	    request = decodeDeleteInstanceRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "GetProperty") == 0)
	    request = decodeGetPropertyRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else if (CompareNoCase(cimMethodName, "SetProperty") == 0)
	    request = decodeSetPropertyRequest(
	       queueId, parser, messageId, nameSpace, authType, userName);
	 else
	 {
	    String description = "Unknown intrinsic method: ";
	    description += cimMethodName;

	    sendIMethodError(
	       queueId, 
	       messageId,
	       cimMethodName,
	       CIM_ERR_FAILED,
	       description);

	    return;
	 }

	 // Expect </IMETHODCALL>

	 XmlReader::expectEndTag(parser, "IMETHODCALL");
      }
      // Expect <METHODCALL ...>
      else if (XmlReader::getMethodCallStartTag(parser, cimMethodName))
      {
	 CIMReference reference;
	 XmlEntry        entry;

	 //
	 // Check for <LOCALINSTANCEPATHELEMENT ...>
	 //
	 if ( XmlReader::testStartTag (parser, entry,
				       XML_ELEMENT_LOCALINSTANCEPATH))
	 {
	    parser.putBack(entry);
	    if (!XmlReader::getLocalInstancePathElement(parser, reference))
	    {
	       throw XmlValidationError(parser.getLine(),
					"expected LOCALINSTANCEPATH element");
	    }
	 }
	 //
	 // Check for <LOCALCLASSPATHELEMENT ...>
	 //
	 else if ( XmlReader::testStartTag( parser, entry,
					    XML_ELEMENT_LOCALCLASSPATH))
	 {
	    parser.putBack(entry);
	    if (!XmlReader::getLocalClassPathElement(parser, reference))
	    {
	       throw XmlValidationError(parser.getLine(),
					"expected LOCALCLASSPATH element");
	    }
	 }
	 else
	 {
	    throw XmlValidationError(parser.getLine(),
				     MISSING_ELEMENT_LOCALPATH);
	 }

	 if (cimMethodName != NULL)
	 {
	    //
	    // if CIMOM is shutting down, return error response
	    //
	    // ATTN:  Need to define a new CIM Error.
	    //
	    if (_serverTerminating)
	    {
	       String description = "CIMServer is shutting down.  ";
	       description.append("Request cannot be processed: ");
	       description += cimMethodName;

	       sendMethodError(
		  queueId,
		  messageId,
		  cimMethodName,
		  CIM_ERR_FAILED,
		  description);

	       return;
	    }
	    else
	    {
	       // Delegate to appropriate method to handle:

	       request = decodeInvokeMethodRequest(
		  queueId, 
		  parser, 
		  messageId, 
		  reference, 
		  cimMethodName,
		  authType,
		  userName);
	    }
	 }
	 else
	 {
	    // ATTN: cimMethodName is null in this case
	    String description = "Unknown extrinsic method: ";
	    description += cimMethodName;

	    sendMethodError(
	       queueId, 
	       messageId,
	       cimMethodName,
	       CIM_ERR_FAILED,
	       description);

	    return;
	 }
	 // Expect </METHODCALL>

	 XmlReader::expectEndTag(parser, "METHODCALL");
      }
      else
      {
	 throw XmlValidationError(parser.getLine(), 
				  "expected IMETHODCALL element");
      }

      // Expect </SIMPLEREQ>

      XmlReader::expectEndTag(parser, "SIMPLEREQ");

      // Expect </MESSAGE>

      XmlReader::expectEndTag(parser, "MESSAGE");

      // Expect </CIM>

      XmlReader::expectEndTag(parser, "CIM");
   }
   catch (CIMException& e)
   {
      sendIMethodError(
	 queueId, 
	 messageId,
	 cimMethodName,
	 e.getCode(),
	 e.getMessage());
      return;
   }
   catch (Exception& e)
   {
      sendIMethodError(
	 queueId, 
	 messageId,
	 cimMethodName,
	 CIM_ERR_FAILED,
	 e.getMessage());
      return;
   }

   _outputQueue->enqueue(request);
}

CIMCreateClassRequestMessage* CIMOperationRequestDecoder::decodeCreateClassRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMClass newClass;
   Boolean duplicateParameter = false;
   Boolean gotClass = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "NewClass") == 0)
      {
	 XmlReader::getClassElement(parser, newClass);
	 duplicateParameter = gotClass;
	 gotClass = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotClass)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }
 
   CIMCreateClassRequestMessage* request = new CIMCreateClassRequestMessage(
      messageId,
      nameSpace,
      newClass,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMGetClassRequestMessage* CIMOperationRequestDecoder::decodeGetClassRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String className;
   Boolean localOnly = true;
   Boolean includeQualifiers = true;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ClassName") == 0)
      {
	 /* ATTN-KS: Should be getStringValueElement(
	    parser, qualifierName, true)
	    Note that this could trip up the client
	 */
	 /* ATTN-RK: Section 2.4.1 of the Specification for CIM Operations
	    over HTTP indicates that the CLASSNAME tag is correct.
	 */
	 XmlReader::getClassNameElement(parser, className, true);
	 duplicateParameter = gotClassName;
	 gotClassName = true;
      }
      else if (CompareNoCase(name, "LocalOnly") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, localOnly, true);
	 duplicateParameter = gotLocalOnly;
	 gotLocalOnly = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "IncludeClassOrigin") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
	 duplicateParameter = gotIncludeClassOrigin;
	 gotIncludeClassOrigin = true;
      }
      else if (CompareNoCase(name, "PropertyList") == 0)
      {
	 CIMValue pl;
	 if (XmlReader::getValueArrayElement(parser, CIMType::STRING, pl))
	 {
	    Array<String> propertyListArray;
	    pl.get(propertyListArray);
	    propertyList.set(propertyListArray);
	 }
	 duplicateParameter = gotPropertyList;
	 gotPropertyList = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotClassName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMGetClassRequestMessage* request = new CIMGetClassRequestMessage(
      messageId,
      nameSpace,
      className,
      localOnly,
      includeQualifiers,
      includeClassOrigin,
      propertyList,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMModifyClassRequestMessage* CIMOperationRequestDecoder::decodeModifyClassRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMClass modifiedClass;
   Boolean duplicateParameter = false;
   Boolean gotClass = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ModifiedClass") == 0)
      {
	 XmlReader::getClassElement(parser, modifiedClass);
	 duplicateParameter = gotClass;
	 gotClass = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotClass)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMModifyClassRequestMessage* request = 
      new CIMModifyClassRequestMessage(
	 messageId,
	 nameSpace,
	 modifiedClass,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMEnumerateClassNamesRequestMessage* CIMOperationRequestDecoder::decodeEnumerateClassNamesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String className;
   Boolean deepInheritance = false;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotDeepInheritance = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ClassName") == 0)
      {
	 XmlReader::getClassNameElement(parser, className, true);
	 duplicateParameter = gotClassName;
	 gotClassName = true;
      }
      else if (CompareNoCase(name, "DeepInheritance") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, deepInheritance, true);
	 duplicateParameter = gotDeepInheritance;
	 gotDeepInheritance = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   CIMEnumerateClassNamesRequestMessage* request = 
      new CIMEnumerateClassNamesRequestMessage(
	 messageId,
	 nameSpace,
	 className,
	 deepInheritance,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMEnumerateClassesRequestMessage* CIMOperationRequestDecoder::decodeEnumerateClassesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String className;
   Boolean deepInheritance = false;
   Boolean localOnly = true;
   Boolean includeQualifiers = true;
   Boolean includeClassOrigin = false;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotDeepInheritance = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ClassName") == 0)
      {
	 XmlReader::getClassNameElement(parser, className, true);
	 duplicateParameter = gotClassName;
	 gotClassName = true;
      }
      else if (CompareNoCase(name, "DeepInheritance") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, deepInheritance, true);
	 duplicateParameter = gotDeepInheritance;
	 gotDeepInheritance = true;
      }
      else if (CompareNoCase(name, "LocalOnly") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, localOnly, true);
	 duplicateParameter = gotLocalOnly;
	 gotLocalOnly = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "IncludeClassOrigin") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
	 duplicateParameter = gotIncludeClassOrigin;
	 gotIncludeClassOrigin = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   CIMEnumerateClassesRequestMessage* request = 
      new CIMEnumerateClassesRequestMessage(
	 messageId,
	 nameSpace,
	 className,
	 deepInheritance,
	 localOnly,
	 includeQualifiers,
	 includeClassOrigin,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMDeleteClassRequestMessage* CIMOperationRequestDecoder::decodeDeleteClassRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String className;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ClassName") == 0)
      {
	 XmlReader::getClassNameElement(parser, className);
	 duplicateParameter = gotClassName;
	 gotClassName = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotClassName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMDeleteClassRequestMessage* request = new CIMDeleteClassRequestMessage(
      messageId,
      nameSpace,
      className,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMCreateInstanceRequestMessage* CIMOperationRequestDecoder::decodeCreateInstanceRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMInstance newInstance;
   Boolean duplicateParameter = false;
   Boolean gotInstance = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "NewInstance") == 0)
      {
	 XmlReader::getInstanceElement(parser, newInstance);
	 duplicateParameter = gotInstance;
	 gotInstance = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotInstance)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMCreateInstanceRequestMessage* request = 
      new CIMCreateInstanceRequestMessage(
	 messageId,
	 nameSpace,
	 newInstance,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMGetInstanceRequestMessage* CIMOperationRequestDecoder::decodeGetInstanceRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference instanceName;
   Boolean localOnly = true;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "InstanceName") == 0)
      {
	 XmlReader::getInstanceNameElement(parser, instanceName);
	 duplicateParameter = gotInstanceName;
	 gotInstanceName = true;
      }
      else if (CompareNoCase(name, "LocalOnly") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, localOnly, true);
	 duplicateParameter = gotLocalOnly;
	 gotLocalOnly = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "IncludeClassOrigin") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
	 duplicateParameter = gotIncludeClassOrigin;
	 gotIncludeClassOrigin = true;
      }
      else if (CompareNoCase(name, "PropertyList") == 0)
      {
	 CIMValue pl;
	 if (XmlReader::getValueArrayElement(parser, CIMType::STRING, pl))
	 {
	    Array<String> propertyListArray;
	    pl.get(propertyListArray);
	    propertyList.set(propertyListArray);
	 }
	 duplicateParameter = gotPropertyList;
	 gotPropertyList = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotInstanceName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMGetInstanceRequestMessage* request = new CIMGetInstanceRequestMessage(
      messageId,
      nameSpace,
      instanceName,
      localOnly,
      includeQualifiers,
      includeClassOrigin,
      propertyList,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMModifyInstanceRequestMessage* CIMOperationRequestDecoder::decodeModifyInstanceRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMNamedInstance modifiedInstance;
   Boolean includeQualifiers = true;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotInstance = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotPropertyList = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ModifiedInstance") == 0)
      {
	 XmlReader::getNamedInstanceElement(parser, modifiedInstance);
	 duplicateParameter = gotInstance;
	 gotInstance = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "PropertyList") == 0)
      {
	 CIMValue pl;
	 if (XmlReader::getValueArrayElement(parser, CIMType::STRING, pl))
	 {
	    Array<String> propertyListArray;
	    pl.get(propertyListArray);
	    propertyList.set(propertyListArray);
	 }
	 duplicateParameter = gotPropertyList;
	 gotPropertyList = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotInstance)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMModifyInstanceRequestMessage* request = 
      new CIMModifyInstanceRequestMessage(
	 messageId,
	 nameSpace,
	 modifiedInstance,
	 includeQualifiers,
	 propertyList,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);


   return(request);
}

CIMEnumerateInstancesRequestMessage* CIMOperationRequestDecoder::decodeEnumerateInstancesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String className;
   Boolean deepInheritance = false;
   Boolean localOnly = true;
   Boolean includeQualifiers = true;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotDeepInheritance = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ClassName") == 0)
      {
	 XmlReader::getClassNameElement(parser, className, true);
	 duplicateParameter = gotClassName;
	 gotClassName = true;
      }
      else if (CompareNoCase(name, "DeepInheritance") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, deepInheritance, true);
	 duplicateParameter = gotDeepInheritance;
	 gotDeepInheritance = true;
      }
      else if (CompareNoCase(name, "LocalOnly") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, localOnly, true);
	 duplicateParameter = gotLocalOnly;
	 gotLocalOnly = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "IncludeClassOrigin") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
	 duplicateParameter = gotIncludeClassOrigin;
	 gotIncludeClassOrigin = true;
      }
      else if (CompareNoCase(name, "PropertyList") == 0)
      {
	 CIMValue pl;
	 if (XmlReader::getValueArrayElement(parser, CIMType::STRING, pl))
	 {
	    Array<String> propertyListArray;
	    pl.get(propertyListArray);
	    propertyList.set(propertyListArray);
	 }
	 duplicateParameter = gotPropertyList;
	 gotPropertyList = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotClassName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMEnumerateInstancesRequestMessage* request = 
      new CIMEnumerateInstancesRequestMessage(
	 messageId,
	 nameSpace,
	 className,
	 deepInheritance,
	 localOnly,
	 includeQualifiers,
	 includeClassOrigin,
	 propertyList,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMEnumerateInstanceNamesRequestMessage* CIMOperationRequestDecoder::decodeEnumerateInstanceNamesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String className;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ClassName") == 0)
      {
	 XmlReader::getClassNameElement(parser, className, true);
	 duplicateParameter = gotClassName;
	 gotClassName = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotClassName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMEnumerateInstanceNamesRequestMessage* request = 
      new CIMEnumerateInstanceNamesRequestMessage(
	 messageId,
	 nameSpace,
	 className,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMDeleteInstanceRequestMessage* CIMOperationRequestDecoder::decodeDeleteInstanceRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference instanceName;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "InstanceName") == 0)
      {
	 XmlReader::getInstanceNameElement(parser, instanceName);
	 duplicateParameter = gotInstanceName;
	 gotInstanceName = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotInstanceName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMDeleteInstanceRequestMessage* request = new CIMDeleteInstanceRequestMessage(
      messageId,
      nameSpace,
      instanceName,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMSetQualifierRequestMessage* CIMOperationRequestDecoder::decodeSetQualifierRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMQualifierDecl qualifierDeclaration;
   Boolean duplicateParameter = false;
   Boolean gotQualifierDeclaration = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "QualifierDeclaration") == 0)
      {
	 XmlReader::getQualifierDeclElement(parser, qualifierDeclaration);
	 duplicateParameter = gotQualifierDeclaration;
	 gotQualifierDeclaration = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotQualifierDeclaration)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMSetQualifierRequestMessage* request = 
      new CIMSetQualifierRequestMessage(
	 messageId,
	 nameSpace,
	 qualifierDeclaration,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMGetQualifierRequestMessage* CIMOperationRequestDecoder::decodeGetQualifierRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String qualifierName;
   Boolean duplicateParameter = false;
   Boolean gotQualifierName = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "QualifierName") == 0)
      {
	 XmlReader::getClassNameElement(parser, qualifierName, true);
	 duplicateParameter = gotQualifierName;
	 gotQualifierName = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotQualifierName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMGetQualifierRequestMessage* request = 
      new CIMGetQualifierRequestMessage(
	 messageId,
	 nameSpace,
	 qualifierName,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMEnumerateQualifiersRequestMessage* CIMOperationRequestDecoder::decodeEnumerateQualifiersRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      // No IPARAMVALUEs are defined for this operation
      throw CIMException(CIM_ERR_NOT_SUPPORTED);
   }

   CIMEnumerateQualifiersRequestMessage* request = 
      new CIMEnumerateQualifiersRequestMessage(
	 messageId,
	 nameSpace,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMDeleteQualifierRequestMessage* CIMOperationRequestDecoder::decodeDeleteQualifierRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   String qualifierName;
   Boolean duplicateParameter = false;
   Boolean gotQualifierName = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "QualifierName") == 0)
      {
	 XmlReader::getClassNameElement(parser, qualifierName, true);
	 duplicateParameter = gotQualifierName;
	 gotQualifierName = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotQualifierName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMDeleteQualifierRequestMessage* request = 
      new CIMDeleteQualifierRequestMessage(
	 messageId,
	 nameSpace,
	 qualifierName,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMReferenceNamesRequestMessage* CIMOperationRequestDecoder::decodeReferenceNamesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference objectName;
   String resultClass;
   String role;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ObjectName") == 0)
      {
	 XmlReader::getObjectNameElement(parser, objectName);
	 duplicateParameter = gotObjectName;
	 gotObjectName = true;
      }
      else if (CompareNoCase(name, "ResultClass") == 0)
      {
	 XmlReader::getClassNameElement(parser, resultClass, true);
	 duplicateParameter = gotResultClass;
	 gotResultClass = true;
      }
      else if (CompareNoCase(name, "Role") == 0)
      {
	 XmlReader::getStringValueElement(parser, role, true);
	 duplicateParameter = gotRole;
	 gotRole = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotObjectName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMReferenceNamesRequestMessage* request = 
      new CIMReferenceNamesRequestMessage(
	 messageId,
	 nameSpace,
	 objectName,
	 resultClass,
	 role,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMReferencesRequestMessage* CIMOperationRequestDecoder::decodeReferencesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference objectName;
   String resultClass;
   String role;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ObjectName") == 0)
      {
	 XmlReader::getObjectNameElement(parser, objectName);
	 duplicateParameter = gotObjectName;
	 gotObjectName = true;
      }
      else if (CompareNoCase(name, "ResultClass") == 0)
      {
	 XmlReader::getClassNameElement(parser, resultClass, true);
	 duplicateParameter = gotResultClass;
	 gotResultClass = true;
      }
      else if (CompareNoCase(name, "Role") == 0)
      {
	 XmlReader::getStringValueElement(parser, role, true);
	 duplicateParameter = gotRole;
	 gotRole = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "IncludeClassOrigin") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
	 duplicateParameter = gotIncludeClassOrigin;
	 gotIncludeClassOrigin = true;
      }
      else if (CompareNoCase(name, "PropertyList") == 0)
      {
	 CIMValue pl;
	 if (XmlReader::getValueArrayElement(parser, CIMType::STRING, pl))
	 {
	    Array<String> propertyListArray;
	    pl.get(propertyListArray);
	    propertyList.set(propertyListArray);
	 }
	 duplicateParameter = gotPropertyList;
	 gotPropertyList = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotObjectName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMReferencesRequestMessage* request = 
      new CIMReferencesRequestMessage(
	 messageId,
	 nameSpace,
	 objectName,
	 resultClass,
	 role,
	 includeQualifiers,
	 includeClassOrigin,
	 propertyList,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMAssociatorNamesRequestMessage* CIMOperationRequestDecoder::decodeAssociatorNamesRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference objectName;
   String assocClass;
   String resultClass;
   String role;
   String resultRole;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotAssocClass = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean gotResultRole = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ObjectName") == 0)
      {
	 XmlReader::getObjectNameElement(parser, objectName);
	 duplicateParameter = gotObjectName;
	 gotObjectName = true;
      }
      else if (CompareNoCase(name, "AssocClass") == 0)
      {
	 XmlReader::getClassNameElement(parser, assocClass, true);
	 duplicateParameter = gotAssocClass;
	 gotAssocClass = true;
      }
      else if (CompareNoCase(name, "ResultClass") == 0)
      {
	 XmlReader::getClassNameElement(parser, resultClass, true);
	 duplicateParameter = gotResultClass;
	 gotResultClass = true;
      }
      else if (CompareNoCase(name, "Role") == 0)
      {
	 XmlReader::getStringValueElement(parser, role, true);
	 duplicateParameter = gotRole;
	 gotRole = true;
      }
      else if (CompareNoCase(name, "ResultRole") == 0)
      {
	 XmlReader::getStringValueElement(parser, resultRole, true);
	 duplicateParameter = gotResultRole;
	 gotResultRole = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotObjectName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMAssociatorNamesRequestMessage* request = 
      new CIMAssociatorNamesRequestMessage(
	 messageId,
	 nameSpace,
	 objectName,
	 assocClass,
	 resultClass,
	 role,
	 resultRole,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMAssociatorsRequestMessage* CIMOperationRequestDecoder::decodeAssociatorsRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference objectName;
   String assocClass;
   String resultClass;
   String role;
   String resultRole;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotAssocClass = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean gotResultRole = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "ObjectName") == 0)
      {
	 XmlReader::getObjectNameElement(parser, objectName);
	 duplicateParameter = gotObjectName;
	 gotObjectName = true;
      }
      else if (CompareNoCase(name, "AssocClass") == 0)
      {
	 XmlReader::getClassNameElement(parser, assocClass, true);
	 duplicateParameter = gotAssocClass;
	 gotAssocClass = true;
      }
      else if (CompareNoCase(name, "ResultClass") == 0)
      {
	 XmlReader::getClassNameElement(parser, resultClass, true);
	 duplicateParameter = gotResultClass;
	 gotResultClass = true;
      }
      else if (CompareNoCase(name, "Role") == 0)
      {
	 XmlReader::getStringValueElement(parser, role, true);
	 duplicateParameter = gotRole;
	 gotRole = true;
      }
      else if (CompareNoCase(name, "ResultRole") == 0)
      {
	 XmlReader::getStringValueElement(parser, resultRole, true);
	 duplicateParameter = gotResultRole;
	 gotResultRole = true;
      }
      else if (CompareNoCase(name, "IncludeQualifiers") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
	 duplicateParameter = gotIncludeQualifiers;
	 gotIncludeQualifiers = true;
      }
      else if (CompareNoCase(name, "IncludeClassOrigin") == 0)
      {
	 XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
	 duplicateParameter = gotIncludeClassOrigin;
	 gotIncludeClassOrigin = true;
      }
      else if (CompareNoCase(name, "PropertyList") == 0)
      {
	 CIMValue pl;
	 if (XmlReader::getValueArrayElement(parser, CIMType::STRING, pl))
	 {
	    Array<String> propertyListArray;
	    pl.get(propertyListArray);
	    propertyList.set(propertyListArray);
	 }
	 duplicateParameter = gotPropertyList;
	 gotPropertyList = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotObjectName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMAssociatorsRequestMessage* request = 
      new CIMAssociatorsRequestMessage(
	 messageId,
	 nameSpace,
	 objectName,
	 assocClass,
	 resultClass,
	 role,
	 resultRole,
	 includeQualifiers,
	 includeClassOrigin,
	 propertyList,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);

   return(request);
}

CIMGetPropertyRequestMessage* CIMOperationRequestDecoder::decodeGetPropertyRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference instanceName;
   String propertyName;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean gotPropertyName = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "InstanceName") == 0)
      {
	 XmlReader::getInstanceNameElement(parser, instanceName);
	 duplicateParameter = gotInstanceName;
	 gotInstanceName = true;
      }
      else if (CompareNoCase(name, "PropertyName") == 0)
      {
	 XmlReader::getStringValueElement(parser, propertyName, true);
	 duplicateParameter = gotPropertyName;
	 gotPropertyName = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotInstanceName || !gotPropertyName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMGetPropertyRequestMessage* request = new CIMGetPropertyRequestMessage(
      messageId,
      nameSpace,
      instanceName,
      propertyName,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMSetPropertyRequestMessage* CIMOperationRequestDecoder::decodeSetPropertyRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const String& nameSpace,
   const String& authType,
   const String& userName)
{
   CIMReference instanceName;
   String propertyName;
   CIMValue propertyValue;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean gotPropertyName = false;
   Boolean gotNewValue = false;

   for (const char* name; XmlReader::getIParamValueTag(parser, name);)
   {
      if (CompareNoCase(name, "InstanceName") == 0)
      {
	 XmlReader::getInstanceNameElement(parser, instanceName);
	 duplicateParameter = gotInstanceName;
	 gotInstanceName = true;
      }
      else if (CompareNoCase(name, "PropertyName") == 0)
      {
	 XmlReader::getStringValueElement(parser, propertyName, true);
	 duplicateParameter = gotPropertyName;
	 gotPropertyName = true;
      }
      else if (CompareNoCase(name, "NewValue") == 0)
      {
	 if (!XmlReader::getPropertyValue(parser, propertyValue))
	 {
	    propertyValue.setNullValue(CIMType::STRING, false);
	 }
	 duplicateParameter = gotNewValue;
	 gotNewValue = true;
      }
      else
      {
	 throw CIMException(CIM_ERR_NOT_SUPPORTED);
      }

      XmlReader::expectEndTag(parser, "IPARAMVALUE");

      if (duplicateParameter)
      {
	 throw CIMException(CIM_ERR_INVALID_PARAMETER);
      }
   }

   if (!gotInstanceName || !gotPropertyName)
   {
      throw CIMException(CIM_ERR_INVALID_PARAMETER);
   }

   CIMSetPropertyRequestMessage* request = new CIMSetPropertyRequestMessage(
      messageId,
      nameSpace,
      instanceName,
      propertyName,
      propertyValue,
      QueueIdStack(queueId, _returnQueueId),
      authType,
      userName);

   return(request);
}

CIMInvokeMethodRequestMessage* CIMOperationRequestDecoder::decodeInvokeMethodRequest(
   Uint32 queueId,
   XmlParser& parser, 
   const String& messageId,
   const CIMReference& reference,
   const String& cimMethodName,
   const String& authType,
   const String& userName)
{
   CIMParamValue paramValue;
   Array<CIMParamValue> inParameters;
    
   while (XmlReader::getParamValueElement(parser, paramValue))
   {
      inParameters.append(paramValue);
   }

   CIMInvokeMethodRequestMessage* request =     
      new CIMInvokeMethodRequestMessage(
	 messageId, 
	 reference.getNameSpace(), 
	 reference, 
	 cimMethodName.allocateCString(),
	 inParameters,
	 QueueIdStack(queueId, _returnQueueId),
	 authType,
	 userName);


   return(request);
}

void CIMOperationRequestDecoder::setServerTerminating(Boolean flag)
{
   _serverTerminating = flag;
}

PEGASUS_NAMESPACE_END
