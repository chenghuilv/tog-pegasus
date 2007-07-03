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

#include "Network.h"
#include "Config.h"
#include <iostream>
#include "Constants.h"
#include "Socket.h"
#include <Pegasus/Common/MessageLoader.h>
#include "Socket.h"
#include "TLS.h"
#include "HTTPConnector.h"
#include "HTTPConnection.h"
#include "HostAddress.h"

#ifdef PEGASUS_OS_OS400
# include "EBCDIC_OS400.h"
#endif

#ifdef PEGASUS_OS_ZOS
# include <resolv.h>  // MAXHOSTNAMELEN
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

class bsd_socket_rep;

////////////////////////////////////////////////////////////////////////////////
//
// Local routines:
//
////////////////////////////////////////////////////////////////////////////////

static Boolean _MakeAddress(
    const char* hostname,
    int port,
    sockaddr_in& address,
    void **addr_info)
{
    if (!hostname)
        return false;

#ifdef PEGASUS_ENABLE_IPV6
    struct sockaddr_in6 serveraddr;
    struct addrinfo  hints;
    struct addrinfo *result;
    int rc = 0;

    memset(&hints, 0x00, sizeof(hints));
    hints.ai_family   = AF_UNSPEC ;
    hints.ai_socktype = SOCK_STREAM;

    // Giving hint as AI_NUMERICHOST to the getaddrinfo function avoids
    // name resolution done by getaddrinfo at DNS. This will improve
    // performance.

    // Check for valid IPv4 address.
    if (1 == HostAddress::convertTextToBinary(AF_INET, hostname, &serveraddr))
    {
        hints.ai_family = AF_INET;
        hints.ai_flags |= AI_NUMERICHOST;
    } // check for valid IPv6 address
    else if (1 == HostAddress::convertTextToBinary(AF_INET6, hostname,
        &serveraddr))
    {
        hints.ai_family = AF_INET6;
        hints.ai_flags |= AI_NUMERICHOST;
    }

    char portStr[20];
    sprintf(portStr,"%d",port);
    while ((rc = getaddrinfo(hostname, portStr, &hints, &result)) == EAI_AGAIN)
        ;
    if (rc)
    {
        return false;
    }
    *addr_info = result;
#else
#ifdef PEGASUS_OS_OS400
    char ebcdicHost[256];
    if (strlen(hostname) < 256)
    strcpy(ebcdicHost, hostname);
    else
    return false;
    AtoE(ebcdicHost);
#endif

////////////////////////////////////////////////////////////////////////////////
// This code used to check if the first character of "hostname" was alphabetic
// to indicate hostname instead of IP address. But RFC 1123, section 2.1,
// relaxed this requirement to alphabetic character *or* digit. So bug 1462
// changed the flow here to call inet_addr first to check for a valid IP
// address in dotted decimal notation. If it's not a valid IP address, then
// try to validate it as a hostname.
// RFC 1123 states: The host SHOULD check the string syntactically for a
// dotted-decimal number before looking it up in the Domain Name System.
// Hence the call to inet_addr() first.
////////////////////////////////////////////////////////////////////////////////

#ifdef PEGASUS_OS_OS400
    unsigned long tmp_addr = inet_addr(ebcdicHost);
#else
    unsigned long tmp_addr = inet_addr((char *)hostname);
#endif

    struct hostent* hostEntry;

// Note: 0xFFFFFFFF is actually a valid IP address (255.255.255.255).
//       A better solution would be to use inet_aton() or equivalent, as
//       inet_addr() is now considered "obsolete".

    if (tmp_addr == 0xFFFFFFFF)  // if hostname is not an IP address
    {
#if defined(PEGASUS_OS_LINUX)
        char hostEntryBuffer[8192];
        struct hostent hostEntryStruct;
        int hostEntryErrno;

        gethostbyname_r(
            hostname,
            &hostEntryStruct,
            hostEntryBuffer,
            sizeof(hostEntryBuffer),
            &hostEntry,
            &hostEntryErrno);
#elif defined(PEGASUS_OS_SOLARIS)
        char hostEntryBuffer[8192];
        struct hostent hostEntryStruct;
        int hostEntryErrno;

        hostEntry = gethostbyname_r(
            (char *)hostname,
            &hostEntryStruct,
            hostEntryBuffer,
            sizeof(hostEntryBuffer),
            &hostEntryErrno);
#elif defined(PEGASUS_OS_OS400)
        hostEntry = gethostbyname(ebcdicHost);
#elif defined(PEGASUS_OS_ZOS)
        if (String::equalNoCase("localhost",String(hostname)))
        {
            char hostName[PEGASUS_MAXHOSTNAMELEN + 1];
            gethostname( hostName, sizeof( hostName ) );
            hostName[sizeof(hostName)-1] = 0;
            hostEntry = gethostbyname(hostName);
        }
        else
        {
            hostEntry = gethostbyname((char *)hostname);
        }
#else
        hostEntry = gethostbyname((char *)hostname);
#endif
        if (!hostEntry)
        {
            return false;
        }

        memset(&address, 0, sizeof(address));
        memcpy(&address.sin_addr, hostEntry->h_addr, hostEntry->h_length);
        address.sin_family = hostEntry->h_addrtype;
        address.sin_port = htons(port);
    }
    else    // else hostname *is* a dotted-decimal IP address
    {
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = tmp_addr;
        address.sin_port = htons(port);
    }
#endif // PEGASUS_ENABLE_IPV6

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// HTTPConnectorRep
//
////////////////////////////////////////////////////////////////////////////////

struct HTTPConnectorRep
{
    Array<HTTPConnection*> connections;
};

////////////////////////////////////////////////////////////////////////////////
//
// HTTPConnector
//
////////////////////////////////////////////////////////////////////////////////

HTTPConnector::HTTPConnector(Monitor* monitor)
    : Base(PEGASUS_QUEUENAME_HTTPCONNECTOR),
      _monitor(monitor), _entry_index(-1)
{
    _rep = new HTTPConnectorRep;
    Socket::initializeInterface();
}

HTTPConnector::~HTTPConnector()
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnector::~HTTPConnector()");
    delete _rep;
    Socket::uninitializeInterface();
    PEG_METHOD_EXIT();
}

void HTTPConnector::handleEnqueue(Message *message)
{
    if (!message)
        return;

    switch (message->getType())
    {
        // It might be useful to catch socket messages later to implement
        // asynchronous establishment of connections.

        case SOCKET_MESSAGE:
            break;

        case CLOSE_CONNECTION_MESSAGE:
        {
            CloseConnectionMessage* closeConnectionMessage =
                (CloseConnectionMessage*)message;

            for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
            {
                HTTPConnection* connection = _rep->connections[i];
                SocketHandle socket = connection->getSocket();

                if (socket == closeConnectionMessage->socket)
                {
                    _monitor->unsolicitSocketMessages(socket);
                    _rep->connections.remove(i);
                    delete connection;
                    break;
                }
            }
        }

        default:
            // ATTN: need unexpected message error!
            break;
    }

    delete message;
}


void HTTPConnector::handleEnqueue()
{
    Message* message = dequeue();

    if (!message)
        return;

    handleEnqueue(message);
}

HTTPConnection* HTTPConnector::connect(
    const String& host,
    const Uint32 portNumber,
    SSLContext * sslContext,
    MessageQueue* outputMessageQueue)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,"HTTPConnector::connect()");

    SocketHandle socket;

#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    if (host == String::EMPTY)
    {
        // Set up the domain socket for a local connection

        sockaddr_un address;
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, PEGASUS_LOCAL_DOMAIN_SOCKET_PATH);
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
        AtoE(address.sun_path);
#endif

        socket = Socket::createSocket(AF_UNIX, SOCK_STREAM, 0);
        if (socket == PEGASUS_INVALID_SOCKET)
        {
            PEG_METHOD_EXIT();
            throw CannotCreateSocketException();
        }
            

        // Connect the socket to the address:

        if (::connect(socket,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) < 0)
        {
            MessageLoaderParms parms(
                "Common.HTTPConnector.CONNECTION_FAILED_LOCAL_CIM_SERVER",
                "Cannot connect to local CIM server. Connection failed.");
            Socket::close(socket);
            PEG_METHOD_EXIT();
            throw CannotConnectException(parms);
        }
    }
    else
    {
#endif

        // Make the internet address:
#ifdef PEGASUS_ENABLE_IPV6
        struct addrinfo *addr_info;
#endif
        sockaddr_in address;
        if (!_MakeAddress((const char*)host.getCString(), portNumber, address,
#ifdef PEGASUS_ENABLE_IPV6
             (void**)&addr_info
#else
             0
#endif
             ))        
        {
            char portStr [32];
            sprintf (portStr, "%u", portNumber);
            PEG_METHOD_EXIT();
            throw InvalidLocatorException(host + ":" + portStr);
        }

#ifdef PEGASUS_ENABLE_IPV6
        while (addr_info)
        {   
        // Create the socket:
            socket = Socket::createSocket(addr_info->ai_family, 
                addr_info->ai_socktype, addr_info->ai_protocol);
#else
        socket = Socket::createSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
        if (socket == PEGASUS_INVALID_SOCKET)
        {
            PEG_METHOD_EXIT();
            throw CannotCreateSocketException();
        }
            

        // Conect the socket to the address:
#ifdef PEGASUS_ENABLE_IPV6
            if (::connect(socket,
                reinterpret_cast<sockaddr*>(addr_info->ai_addr),
                addr_info->ai_addrlen) < 0)
#else
        if (::connect(socket,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) < 0)
#endif
            {
#ifdef PEGASUS_ENABLE_IPV6
                addr_info = addr_info->ai_next;
                if (addr_info)
        {
                    Socket::close(socket);
                    continue;
                }
#endif
            char portStr[32];
            sprintf(portStr, "%u", portNumber);
            MessageLoaderParms parms(
                "Common.HTTPConnector.CONNECTION_FAILED_TO",
                "Cannot connect to $0:$1. Connection failed.",
                host,
                portStr);
            Socket::close(socket);
            PEG_METHOD_EXIT();
            throw CannotConnectException(parms);
        }
#ifdef PEGASUS_ENABLE_IPV6
            break;
        }
        freeaddrinfo(addr_info);
#endif

#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    }
#endif

    // Create HTTPConnection object:

    AutoPtr<MP_Socket> mp_socket(new MP_Socket(socket, sslContext, 0));
    if (mp_socket->connect() < 0)
    {
        char portStr[32];
        sprintf(portStr, "%u", portNumber);
        MessageLoaderParms parms(
            "Common.HTTPConnector.CONNECTION_FAILED_TO",
            "Cannot connect to $0:$1. Connection failed.",
            host,
            portStr);
        mp_socket->close();
        PEG_METHOD_EXIT();
        throw CannotConnectException(parms);
    }

    HTTPConnection* connection = new HTTPConnection(
        _monitor, mp_socket, String::EMPTY, this,
        static_cast<MessageQueueService *>(outputMessageQueue));

    // Solicit events on this new connection's socket:

    if (-1 == (_entry_index = _monitor->solicitSocketMessages(
            connection->getSocket(),
            SocketMessage::READ | SocketMessage::EXCEPTION,
            connection->getQueueId(), Monitor::CONNECTOR)))
    {
        (connection->getMPSocket()).close();
    }

    // Save the socket for cleanup later:

    _rep->connections.append(connection);
    PEG_METHOD_EXIT();
    return connection;
}

void HTTPConnector::destroyConnections()
{
    // For each connection created by this object:

    for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
    {
        _deleteConnection(_rep->connections[i]);
    }

    _rep->connections.clear();
}


void HTTPConnector::disconnect(HTTPConnection* currentConnection)
{
    //
    // find and delete the specified connection
    //

    Uint32 index = PEG_NOT_FOUND;
    for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
    {
        if (currentConnection == _rep->connections[i])
        {
            index = i;
            break;
        }
     }

    PEGASUS_ASSERT(index != PEG_NOT_FOUND);

    SocketHandle socket = currentConnection->getSocket();
    _monitor->unsolicitSocketMessages(socket);
    _rep->connections.remove(index);
    delete currentConnection;
}

void HTTPConnector::_deleteConnection(HTTPConnection* httpConnection)
{
    SocketHandle socket = httpConnection->getSocket();

    // Unsolicit SocketMessages:

    _monitor->unsolicitSocketMessages(socket);

    // Destroy the connection (causing it to close):

    delete httpConnection;
}

PEGASUS_NAMESPACE_END
