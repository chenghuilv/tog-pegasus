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
// Author: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/HTTPMessage.h>

#include "HttpConstants.h"
#include "WbemExecClient.h"

#include <iostream>
#ifdef PEGASUS_PLATFORM_WIN32_IX86_MSVC
# include <windows.h>
#else
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// ATTN: Put these in a central location
//
// Wbem service name
//
#define WBEM_SERVICE_NAME          "wbem-http"

//
// Wbem default local port number
//
static const Uint32 WBEM_DEFAULT_PORT =  5988;

WbemExecClient::WbemExecClient(Uint32 timeOutMilliseconds)
    : 
    MessageQueue(PEGASUS_QUEUENAME_WBEMEXECCLIENT),
    _httpConnection(0),
    _timeOutMilliseconds(timeOutMilliseconds),
    _connected(false)
{
    //
    // Create Monitor and HTTPConnector
    //
    _monitor = new Monitor();
    _httpConnector = new HTTPConnector(_monitor);
}

WbemExecClient::~WbemExecClient()
{
   disconnect();
   delete _httpConnector;
   delete _monitor;
}

void WbemExecClient::handleEnqueue()
{

}

void WbemExecClient::_connect(
    const String& address,
    SSLContext* sslContext
) throw(CannotCreateSocket, CannotConnect, InvalidLocator, UnexpectedFailure)
{
    //
    // Attempt to establish a connection:
    //
    //try
    //{
	_httpConnection = _httpConnector->connect(address,
                                                  sslContext,
                                                  this);
    //}
    // Could catch CannotCreateSocket, CannotConnect, InvalidLocator, or
    // UnexpectedFailure
    //catch (Exception& e)
    //{
    //    throw e;
    //}
    
    _connected = true;
}

void WbemExecClient::connect(
    const String& address,
    SSLContext* sslContext,
    const String& userName,
    const String& password
) throw(AlreadyConnected, InvalidLocator, CannotCreateSocket,
        CannotConnect, UnexpectedFailure)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
	throw AlreadyConnected();

    //
    // If the address is empty, reject it
    //
    if (address == String::EMPTY)
	throw InvalidLocator(address);

    //
    // Set authentication information
    //
    _authenticator.clearRequest(true);

    if (userName.size())
    {
        _authenticator.setUserName(userName);
    }

    if (password.size())
    {
        _authenticator.setPassword(password);
    }

    _connect(address, sslContext);
}


void WbemExecClient::connectLocal(SSLContext* sslContext)
    throw(AlreadyConnected, InvalidLocator, CannotCreateSocket,
          CannotConnect, UnexpectedFailure)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
	throw AlreadyConnected();

    String address;

#ifndef PEGASUS_LOCAL_DOMAIN_SOCKET
    //
    // Look up the WBEM port number for the local system
    //
    Uint32 portNum = System::lookupPort(WBEM_SERVICE_NAME, WBEM_DEFAULT_PORT);
    char port[32];
    sprintf(port, "%u", portNum);

    //
    // Build address string using local host name and port number
    //
    address.append(_getLocalHostName());
    address.append(":");
    address.append(port);
#endif

    //
    // Set authentication type
    //
    _authenticator.clearRequest(true);
    _authenticator.setAuthType(ClientAuthenticator::LOCAL);

    _connect(address, sslContext);
}

void WbemExecClient::disconnect()
{
    if (_connected)
    {
        //
        // Close the connection
        //
        _httpConnector->disconnect(_httpConnection);

        _authenticator.clearRequest(true);

        _connected = false;
    }
}


Array<Sint8> WbemExecClient::issueRequest(
    const Array<Sint8>& request
) throw(NotConnected, TimedOut)
{
    if (!_connected)
    {
	throw NotConnected();
    }
    
    HTTPMessage* httpRequest = new HTTPMessage(request);
    
    _authenticator.clearRequest();
    _authenticator.setRequestMessage(httpRequest);

    Boolean finished = false;
    HTTPMessage* httpResponse;
    do
    {
        HTTPMessage* httpRequestCopy =
            new HTTPMessage(*(HTTPMessage*)_authenticator.getRequestMessage());
        _addAuthHeader(httpRequestCopy);

        Message* response = _doRequest(httpRequestCopy);
        PEGASUS_ASSERT(response->getType() == HTTP_MESSAGE);
        httpResponse = (HTTPMessage*)response;

        finished = !_checkNeedToResend(httpResponse);
        if (!finished)
        {
            delete httpResponse;
        }
    } while (!finished);
    
    HTTPMessage* origRequest = (HTTPMessage*)_authenticator.getRequestMessage();
    _authenticator.clearRequest();
    delete origRequest;

    Destroyer<HTTPMessage> destroyer(httpResponse);
    
    return(httpResponse->message);
}

Message* WbemExecClient::_doRequest(HTTPMessage * request) throw(TimedOut)
{
    // ATTN-RK-P2-20020416: We should probably clear out the queue first.
    PEGASUS_ASSERT(getCount() == 0);  // Shouldn't be any messages in our queue

    _httpConnection->enqueue(request);

    Uint64 startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    Uint64 nowMilliseconds = startMilliseconds;
    Uint64 stopMilliseconds = nowMilliseconds + _timeOutMilliseconds;

    while (nowMilliseconds < stopMilliseconds)
    {
	//
	// Wait until the timeout expires or an event occurs:
	//

	_monitor->run(Uint32(stopMilliseconds - nowMilliseconds));

	//
	// Check to see if incoming queue has a message
	//

	Message* response = dequeue();

	if (response)
	{
            return response;
	}

        nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    }

    //
    // Throw timed out exception:
    //

    throw TimedOut();
}

String WbemExecClient::_getLocalHostName()
{
    static String hostname;

    if (!hostname.size())
    {
        hostname.assign(System::getHostName());
    }

    return hostname;
}

void WbemExecClient::_addAuthHeader(HTTPMessage*& httpMessage)
{
    //
    // Add authentication headers to the message
    //
    String authHeader = _authenticator.buildRequestAuthHeader();
    if (authHeader != String::EMPTY)
    {
        //
        // Parse the HTTP message:
        //

        String startLine;
        Array<HTTPHeader> headers;
        Sint8* content;
        Uint32 contentLength;
        httpMessage->parse(startLine, headers, contentLength);

        // Calculate the beginning of the content from the message size and
        // the content length

        content = (Sint8*) httpMessage->message.getData() +
	  httpMessage->message.size() - contentLength;

        Array<Sint8> newMessageBuffer;
        newMessageBuffer << startLine << HTTP_CRLF;
        newMessageBuffer << authHeader << HTTP_CRLF;
        for (Uint32 i=0; i<headers.size(); i++)
        {
            newMessageBuffer << headers[i].first << HEADER_SEPARATOR <<
                HTTP_SP << headers[i].second << HTTP_CRLF;
        }
        newMessageBuffer << HTTP_CRLF;
        newMessageBuffer << content << HTTP_CRLF;

        HTTPMessage* newMessage = new HTTPMessage(newMessageBuffer);
        delete httpMessage;
        httpMessage = newMessage;
    }
}

Boolean WbemExecClient::_checkNeedToResend(HTTPMessage* httpMessage)
{
    //
    // Parse the HTTP message:
    //

    String startLine;
    Array<HTTPHeader> headers;
    Uint32 contentLength;

    httpMessage->parse(startLine, headers, contentLength);

    try
    {
        return _authenticator.checkResponseHeaderForChallenge(headers);
    }
    catch(InvalidAuthHeader&)
    {
        // We're done, send (garbage) response back to the user.
        return false;
    }
}

PEGASUS_NAMESPACE_END
