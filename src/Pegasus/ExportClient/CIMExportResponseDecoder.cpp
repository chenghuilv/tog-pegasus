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

#include <Pegasus/Common/Config.h>
#include <iostream>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Exception.h>
#include "CIMExportResponseDecoder.h"

// l10n
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportResponseDecoder::CIMExportResponseDecoder(
   MessageQueue* outputQueue,
   MessageQueue* encoderQueue,
   ClientAuthenticator* authenticator)
   :
   MessageQueue(PEGASUS_QUEUENAME_EXPORTRESPDECODER),
   _outputQueue(outputQueue),
   _encoderQueue(encoderQueue),
   _authenticator(authenticator)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::CIMExportResponseDecoder()");
    PEG_METHOD_EXIT();
}

CIMExportResponseDecoder::~CIMExportResponseDecoder()
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::~CIMExportResponseDecoder()");
    _outputQueue.release();
    _encoderQueue.release();
    _authenticator.release();
    PEG_METHOD_EXIT();
}

void  CIMExportResponseDecoder::setEncoderQueue(MessageQueue* encoderQueue)
{
   PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::setEncoderQueue()");
   _encoderQueue.release();
   _encoderQueue.reset(encoderQueue);
   PEG_METHOD_EXIT();
}


void CIMExportResponseDecoder::handleEnqueue()
{
   PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::handleEnqueue()");
   Message* message = dequeue();

   PEGASUS_ASSERT(message != 0);
   
   switch (message->getType())
   {
      case HTTP_MESSAGE:
      {
	 HTTPMessage* httpMessage = (HTTPMessage*)message;
	 _handleHTTPMessage(httpMessage);
	 break;
      }
      
      default:
	 PEGASUS_ASSERT(0);
	 break;
   }

   delete message;
   PEG_METHOD_EXIT();
}

void CIMExportResponseDecoder::_handleHTTPMessage(HTTPMessage* httpMessage)
{
   PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::handleHTTPMessage()");
   //
   // Parse the HTTP message:
   //

   String startLine;
   Array<HTTPHeader> headers;
   char* content;
   Uint32 contentLength;
   String  connectClose;
   Boolean cimReconnect=false;

   if (httpMessage->message.size() == 0)
   {

     // l10n
     
     // CIMClientMalformedHTTPException* malformedHTTPException =
     // new CIMClientMalformedHTTPException("Empty HTTP response message.");
     
     MessageLoaderParms mlParms("ExportClient.CIMExportResponseDecoder.EMPTY_RESPONSE", "Empty HTTP response message.");
     String mlString(MessageLoader::getMessage(mlParms));
     
     AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
       new CIMClientMalformedHTTPException(mlString));
     
     AutoPtr<ClientExceptionMessage> response(
       new ClientExceptionMessage(malformedHTTPException.get()));

     malformedHTTPException.release();
     
     _outputQueue->enqueue(response.release());
     PEG_METHOD_EXIT();
     return;
   }

   httpMessage->parse(startLine, headers, contentLength);
    //
    // Check for Connection: Close
    //
    if (HTTPMessage::lookupHeader(headers, "Connection", connectClose, false))
    {
        if (String::equalNoCase(connectClose, "Close"))
        {
            //reconnect and then resend next request.
            cimReconnect=true;
        }
    Tracer::trace(TRC_XML_IO, Tracer::LEVEL2, "%s",
                  httpMessage->message.getData());
    }


    //
    // Get the status line info
    //

    String httpVersion;
    Uint32 statusCode;
    String reasonPhrase;

    Boolean parsableMessage = HTTPMessage::parseStatusLine(
        startLine, httpVersion, statusCode, reasonPhrase);
    if (!parsableMessage)
    {

      // l10n

      // CIMClientMalformedHTTPException* malformedHTTPException = new
      //   CIMClientMalformedHTTPException("Malformed HTTP response message.");

      MessageLoaderParms mlParms("ExportClient.CIMExportResponseDecoder.MALFORMED_RESPONSE", "Malformed HTTP response message.");
      String mlString(MessageLoader::getMessage(mlParms));
     
      AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(new
         CIMClientMalformedHTTPException(mlString));

      AutoPtr<ClientExceptionMessage> response(
         new ClientExceptionMessage(malformedHTTPException.get()));

      malformedHTTPException.release();

      response->setCloseConnect(cimReconnect);
      _outputQueue->enqueue(response.release());
      PEG_METHOD_EXIT();
      return;
    }

   try
   {
       if (_authenticator->checkResponseHeaderForChallenge(headers))
       {
          //
          // Get the original request, put that in the encoder's queue for
          // re-sending with authentication challenge response.
          //

          Message* reqMessage = _authenticator->releaseRequestMessage();

          if (cimReconnect == true)
          {
              reqMessage->setCloseConnect(cimReconnect);
              _outputQueue->enqueue(reqMessage);
          }
          else
          {
              _encoderQueue->enqueue(reqMessage);
          }


          PEG_METHOD_EXIT();
          return;
       }
       else
       {
          //
          // Received a valid/error response from the server.
          // We do not need the original request message anymore, hence delete
          // the request message by getting the handle from the ClientAuthenticator.
          //
          Message* reqMessage = _authenticator->releaseRequestMessage();
          delete reqMessage;
       }
    }
    catch(InvalidAuthHeader& e)
    {
        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
            new CIMClientMalformedHTTPException(e.getMessage()));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));

        malformedHTTPException.release();

        response->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(response.release());
        PEG_METHOD_EXIT();
        return;
    }

    // We have the response.  If authentication failed, we will generate a
    // CIMClientHTTPErrorException below with the "401 Unauthorized" status
    // in the (re-challenge) response.

    //
    // Check for a success (200 OK) response
    //

    if (statusCode != HTTP_STATUSCODE_OK)
    {
        String cimError;
        String pegasusError;

        HTTPMessage::lookupHeader(headers, "CIMError", cimError);
        HTTPMessage::lookupHeader(headers, PEGASUS_HTTPHEADERTAG_ERRORDETAIL, pegasusError);
        try
        {
            pegasusError = XmlReader::decodeURICharacters(pegasusError);
        }
	catch (const ParseError&)
        {
            // Ignore this exception.  We're more interested in having the
            // message in encoded form than knowing that the format is invalid.
        }

        AutoPtr<CIMClientHTTPErrorException> httpError(
            new CIMClientHTTPErrorException(statusCode, reasonPhrase,
                                            cimError, pegasusError));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(httpError.get()));

        httpError.release();

        response->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(response.release());
        PEG_METHOD_EXIT();
        return;
    }

   //
   // Search for "CIMOperation" header:
   //

   String cimExport;

   if (!HTTPMessage::lookupHeader(headers, "CIMExport", cimExport, true))
   {
      //
      //  Missing CIMExport HTTP header
      //
      MessageLoaderParms mlParms(
          "ExportClient.CIMExportResponseDecoder.MISSING_CIMEXP_HEADER",
          "Missing CIMExport HTTP header");
      String mlString(MessageLoader::getMessage(mlParms));

      AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(new
         CIMClientMalformedHTTPException(mlString));

      AutoPtr<ClientExceptionMessage> response(
         new ClientExceptionMessage(malformedHTTPException.get()));

      malformedHTTPException.release();

      response->setCloseConnect(cimReconnect);
      _outputQueue->enqueue(response.release());
      PEG_METHOD_EXIT();
      return;
   }

   //
   // Search for "Content-Type" header:
   //

   String cimContentType;

   if (!HTTPMessage::lookupHeader(
	  headers, "Content-Type", cimContentType, true))
   {
      AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(new
            CIMClientMalformedHTTPException("Missing CIMContentType HTTP header"));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));

        malformedHTTPException.release();

        response->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(response.release());
      PEG_METHOD_EXIT();
      return;
   }


   //
   // Zero-terminate the message:
   //

   httpMessage->message.append('\0');

   // Calculate the beginning of the content from the message size and
   // the content length.  Subtract 1 to take into account the null
   // character we just added to the end of the message.

   content = (char *) httpMessage->message.getData() +
      httpMessage->message.size() - contentLength - 1;

   //
   // If it is a method response, then dispatch it to the handler:
   //

   if (!String::equalNoCase(cimExport, "MethodResponse"))
   {
      //
      //  Expected CIMExport HTTP header value MethodResponse
      //
      MessageLoaderParms mlParms(
          "ExportClient.CIMExportResponseDecoder.EXPECTED_METHODRESPONSE",
          "Received CIMExport HTTP header value \"$0\", "
              "expected \"MethodResponse\"", cimExport);
      String mlString(MessageLoader::getMessage(mlParms));

      AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
	     new CIMClientMalformedHTTPException(mlString));

      AutoPtr<ClientExceptionMessage> response(
	     new ClientExceptionMessage(malformedHTTPException.get()));

      malformedHTTPException.release();

      response->setCloseConnect(cimReconnect);
      _outputQueue->enqueue(response.release());
      PEG_METHOD_EXIT();
      return;
   }

   _handleMethodResponse(content,cimReconnect);
   PEG_METHOD_EXIT();
}

void CIMExportResponseDecoder::_handleMethodResponse(char* content,Boolean cimReconnect)
{
   PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::_handleMethodResponse()");
   AutoPtr<Message> response;

   //
   // Create and initialize XML parser:
   //

   XmlParser parser((char*)content);
   XmlEntry entry;

   try
   {
      //
      // Process <?xml ... >
      //

      const char* xmlVersion = 0;
      const char* xmlEncoding = 0;

      XmlReader::getXmlDeclaration(parser, xmlVersion, xmlEncoding);

      //
      // Process <CIM ... >
      //

      const char* cimVersion = 0;
      const char* dtdVersion = 0;

      XmlReader::getCimStartTag(parser, cimVersion, dtdVersion);

      //
      // Expect <MESSAGE ... >
      //

      String messageId;
      String protocolVersion;

      if (!XmlReader::getMessageStartTag(parser, messageId, protocolVersion)) {

	// l10n

	// throw XmlValidationError(
	// parser.getLine(), "expected MESSAGE element");
	
	MessageLoaderParms mlParms("ExportClient.CIMExportResponseDecoder.EXPECTED_MESSAGE_ELEMENT", "expected MESSAGE element");
	String mlString(MessageLoader::getMessage(mlParms));
	
        PEG_METHOD_EXIT();
	throw XmlValidationError(parser.getLine(), mlString);

      }

      if (!String::equalNoCase(protocolVersion, "1.0"))
      {

	// l10n

	// CIMClientResponseException* responseException =
	//     new CIMClientResponseException(
	//          String("Received unsupported protocol version \"") +
	//          protocolVersion + "\", expected \"1.0\"");

	MessageLoaderParms mlParms("ExportClient.CIMExportResponseDecoder.UNSUPPORTED_PROTOCOL", "Received unsupported protocol version \"$0\", expected \"$1\"", protocolVersion, "1.0");
	String mlString(MessageLoader::getMessage(mlParms));
	
	AutoPtr<CIMClientResponseException> responseException(
	  new CIMClientResponseException(mlString));

         AutoPtr<ClientExceptionMessage> response(
                new ClientExceptionMessage(responseException.get()));

         responseException.release();

         response->setCloseConnect(cimReconnect);
         _outputQueue->enqueue(response.release());
         PEG_METHOD_EXIT();
	 return;
      }

      //
      // Expect <SIMPLEEXPRSP ... >
      //

      XmlReader::expectStartTag(parser, entry, "SIMPLEEXPRSP");

      //
      // Expect <EXPMETHODRESPONSE ... >
      //

      const char* expMethodResponseName = 0;
      Boolean isEmptyTag = false;

      if (XmlReader::getEMethodResponseStartTag(
              parser, expMethodResponseName, isEmptyTag))
      {
          if (System::strcasecmp(expMethodResponseName, "ExportIndication") ==
              0)
          {
              response.reset(_decodeExportIndicationResponse(
                  parser, messageId, isEmptyTag));
          }
          else
	  {
            //
            //  Unrecognized ExpMethodResponse name attribute
            //
            MessageLoaderParms mlParms(
                "ExportClient.CIMExportResponseDecoder.UNRECOGNIZED_EXPMETHRSP",
                "Unrecognized ExpMethodResponse name \"$0\"",
                expMethodResponseName);
	    String mlString(MessageLoader::getMessage(mlParms));
	    
            PEG_METHOD_EXIT();
	    throw XmlValidationError(parser.getLine(), mlString);
           }

            //
            // Handle end tag:
            //

            if (!isEmptyTag)
            {
                XmlReader::expectEndTag(parser, "EXPMETHODRESPONSE");
            }
        }
        else
        {
          //
          //  Expected ExpMethodResponse element
          //
          MessageLoaderParms mlParms(
              "ExportClient.CIMExportResponseDecoder.
                  EXPECTED_EXPMETHODRESPONSE_ELEMENT",
              "expected EXPMETHODRESPONSE element");
	  String mlString(MessageLoader::getMessage(mlParms));
	  
          PEG_METHOD_EXIT();
	  throw XmlValidationError(parser.getLine(), mlString);
        }

        //
        // Handle end tags:
        //
	XmlReader::expectEndTag(parser, "SIMPLEEXPRSP");
        XmlReader::expectEndTag(parser, "MESSAGE");
        XmlReader::expectEndTag(parser, "CIM");
    }
    catch (XmlException& x)
    {
        response.reset(new ClientExceptionMessage(
            new CIMClientXmlException(x.getMessage())));
    }
    catch (Exception& x)
    {
        // Might get MalformedObjectNameException, InvalidNameException, etc.

        response.reset(new ClientExceptionMessage(
            new CIMClientResponseException(x.getMessage())));
    }

// l10n
// 	Note: Ignore any ContentLanguage set in the export response

    response->setCloseConnect(cimReconnect);
    _outputQueue->enqueue(response.release());
    PEG_METHOD_EXIT();
}

CIMExportIndicationResponseMessage* CIMExportResponseDecoder::_decodeExportIndicationResponse(
    XmlParser& parser,
    const String& messageId,
    Boolean isEmptyImethodresponseTag)
{
   PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportResponseDecoder::_decodeExportIndicationResponse()");
   XmlEntry entry;
   CIMException cimException;

   if (!isEmptyImethodresponseTag)
   {
      if (XmlReader::getErrorElement(parser, cimException))
      {
         PEG_METHOD_EXIT();
         return(new CIMExportIndicationResponseMessage(
            messageId,
            cimException,
            QueueIdStack()));
      }

      if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
      {
         if (entry.type != XmlEntry::EMPTY_TAG)
         {
            XmlReader::expectEndTag(parser, "IRETURNVALUE");
         }
      }
   }

   PEG_METHOD_EXIT();
   return(new CIMExportIndicationResponseMessage(
      messageId,
      cimException,
      QueueIdStack()));
}

PEGASUS_NAMESPACE_END
