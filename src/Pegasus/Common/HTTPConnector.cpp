//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Modified By: Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
// Modified By: Sushma Fernandes, Hewlett-Packard Company
//                (sushma_fernandes@hp.com)
// Modified By: Dan Gorey, IBM (djgorey@us.ibm.com)
// Modified By: Amit Arora (amita@in.ibm.com) for Bug#1170
//
//%/////////////////////////////////////////////////////////////////////////////

#include "Config.h"
#include <iostream>
#include "Constants.h"
#include "Socket.h"
#include <Pegasus/Common/MessageLoader.h> //l10n

#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <windows.h>
#else
# include <cctype>
# include <cstdlib>
# include <errno.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
# include <unistd.h>
#  include <sys/un.h>
# endif
#endif

#include "Socket.h"
#include "TLS.h"
#include "HTTPConnector.h"
#include "HTTPConnection.h"

#ifdef PEGASUS_OS_OS400
#  include "OS400ConvertChar.h"
#endif

#ifdef PEGASUS_OS_ZOS
#  include <resolv.h>  // MAXHOSTNAMELEN
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
   sockaddr_in& address)
{
   if (!hostname)
      return false;

#ifdef PEGASUS_OS_OS400
    char ebcdicHost[256];
    if (strlen(hostname) < 256)
	strcpy(ebcdicHost, hostname);
    else
	return false;
    AtoE(ebcdicHost);
#endif
	
   struct hostent *entry;
   if (isalpha(hostname[0]))
   {
#ifdef PEGASUS_PLATFORM_SOLARIS_SPARC_CC
#define HOSTENT_BUFF_SIZE        8192
      char      buf[HOSTENT_BUFF_SIZE];
      int       h_errorp;
      struct    hostent hp;

      entry = gethostbyname_r((char *)hostname, &hp, buf,
	  							HOSTENT_BUFF_SIZE, &h_errorp);
#elif defined(PEGASUS_OS_OS400)
      entry = gethostbyname(ebcdicHost);
#elif defined(PEGASUS_OS_ZOS)
	  char hostName[ MAXHOSTNAMELEN + 1 ];
	  if (String::equalNoCase("localhost",String(hostname)))
	  {
		  gethostname( hostName, sizeof( hostName ) );
		  entry = gethostbyname(hostName);
	  } else {
		  entry = gethostbyname((char *)hostname);
	  }
#else
      entry = gethostbyname((char *)hostname);
#endif
      if(!entry)
      {
	return false;
      }

      memset(&address, 0, sizeof(address));
      memcpy(&address.sin_addr, entry->h_addr, entry->h_length);
      address.sin_family = entry->h_addrtype;
      address.sin_port = htons(port);
   }     
   else
   {
#ifdef PEGASUS_OS_OS400
      unsigned long tmp_addr = inet_addr(ebcdicHost);
#else
    #ifdef PEGASUS_OS_ZOS
      unsigned long tmp_addr = inet_addr_ebcdic((char *)hostname);
    #else
      unsigned long tmp_addr = inet_addr((char *)hostname);
    #endif
#endif
      
      if(tmp_addr == 0xFFFFFFFF)
      {
	  return false;
      }
      memset(&address, 0, sizeof(address));
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = tmp_addr;
      address.sin_port = htons(port);
   }

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

struct HTTPConnector2Rep
{
      Array<HTTPConnection2*> connections2;
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
   delete _rep;
   Socket::uninitializeInterface();
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
	 CloseConnectionMessage* closeConnectionMessage 
	    = (CloseConnectionMessage*)message;

	 for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
	 {
	    HTTPConnection* connection = _rep->connections[i];	
	    Sint32 socket = connection->getSocket();

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
   };

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
   Sint32 socket;

#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
   if (host == String::EMPTY)
   {
      // Set up the domain socket for a local connection

      sockaddr_un address;
      address.sun_family = AF_UNIX;
      strcpy(address.sun_path, PEGASUS_LOCAL_DOMAIN_SOCKET_PATH);
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
      AtoE(address.sun_path);
#endif

      socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
      if (socket < 0)
         throw CannotCreateSocketException();

      // Connect the socket to the address:

      if (::connect(socket,
                    reinterpret_cast<sockaddr*>(&address),
                    sizeof(address)) < 0)
      {
	 
      	//l10n
         //throw CannotConnectException("Cannot connect to local CIM server. Connection failed.");
         MessageLoaderParms parms("Common.HTTPConnector.CONNECTION_FAILED_LOCAL_CIM_SERVER",
         						  "Cannot connect to local CIM server. Connection failed.");
         throw CannotConnectException(parms);
      }
   }
   else
   {
#endif

   // Make the internet address:

   sockaddr_in address;

   if (!_MakeAddress((const char*)host.getCString(), portNumber, address))
   {
      char portStr [32];
      sprintf (portStr, "%u", portNumber);
      throw InvalidLocatorException(host + ":" + portStr);
   }


   // Create the socket:

   socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if (socket < 0)
      throw CannotCreateSocketException();

   // Conect the socket to the address:

   if (::connect(socket,
                 reinterpret_cast<sockaddr*>(&address),
                 sizeof(address)) < 0)
   {
      char portStr [32];
      sprintf (portStr, "%u", portNumber);
      //l10n
      //throw CannotConnectException("Cannot connect to " + host + ":" + portStr +". Connection failed.");
      MessageLoaderParms parms("Common.HTTPConnector.CONNECTION_FAILED_TO",
         					   "Cannot connect to $0:$1. Connection failed.",
         					   host,
         					   portStr);
      throw CannotConnectException(parms);
   }

#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
   }
#endif

   // Create HTTPConnection object:

   MP_Socket * mp_socket = new MP_Socket(socket, sslContext);
   if (mp_socket->connect() < 0) {
      char portStr [32];
      sprintf (portStr, "%u", portNumber);
      //l10n
      //throw CannotConnectException("Cannot connect to " + host + ":" + portStr +". Connection failed.");
      MessageLoaderParms parms("Common.HTTPConnector.CONNECTION_FAILED_TO",
         					   "Cannot connect to $0:$1. Connection failed.",
         					   host,
         					   portStr);
      throw CannotConnectException(parms);
   }
    
   HTTPConnection* connection = new HTTPConnection(_monitor, mp_socket,
						   this, static_cast<MessageQueueService *>(outputMessageQueue));

   // Solicit events on this new connection's socket:

   if (-1 == (_entry_index = _monitor->solicitSocketMessages(
	  mp_socket->getSocket(),
	  SocketMessage::READ | SocketMessage::EXCEPTION,
	  connection->getQueueId(), Monitor::CONNECTOR)))
   {
      mp_socket->close();
   }

   // Save the socket for cleanup later:

   _rep->connections.append(connection);

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
    for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
    {
        if (currentConnection == _rep->connections[i])
        {
	   Sint32 socket = _rep->connections[i]->getSocket();
	   _monitor->unsolicitSocketMessages(socket);
	   _rep->connections.remove(i);

            Socket::close(socket);
            return;
        }
    }
}

void HTTPConnector::_deleteConnection(HTTPConnection* httpConnection)
{
    Sint32 socket = httpConnection->getSocket();

    // Unsolicit SocketMessages:

    _monitor->unsolicitSocketMessages(socket);

    // Destroy the connection (causing it to close):

    delete httpConnection;
}


////////////////////////////////////////////////////////////////////////////////
//
// HTTPConnector2
//
////////////////////////////////////////////////////////////////////////////////

HTTPConnector2::HTTPConnector2(monitor_2* monitor)
   : Base(PEGASUS_QUEUENAME_HTTPCONNECTOR),
     _monitor(monitor), _entry_index(-1)
{
   _rep2 = new HTTPConnector2Rep;
   Socket::initializeInterface();
}

HTTPConnector2::~HTTPConnector2()
{
   delete _rep2;
   Socket::uninitializeInterface();
}

void HTTPConnector2::handleEnqueue(Message *message)
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
	 CloseConnectionMessage* closeConnectionMessage 
	    = (CloseConnectionMessage*)message;

	 for (Uint32 i = 0, n = _rep2->connections2.size(); i < n; i++)
	 {
	    HTTPConnection2* connection = _rep2->connections2[i];	
	    Sint32 socket = connection->getSocket();

	    if (socket == closeConnectionMessage->socket)
	    {
	       _monitor->unsolicitSocketMessages(socket);
	       _rep2->connections2.remove(i);
	       delete connection;
	       break;
	    }
	 }
      }

      default:
	 // ATTN: need unexpected message error!
	 break;
   };

   delete message;
}

void HTTPConnector2::handleEnqueue()
{

   Message* message = dequeue();

   if (!message)
      return;

   handleEnqueue(message);
}

HTTPConnection2* HTTPConnector2::connect(
   const String& host, 
   const Uint32 portNumber,
   SSLContext * sslContext,
   MessageQueue* outputMessageQueue)
{
   Sint32 socket;

#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
   if (host == String::EMPTY)
   {
      // Set up the domain socket for a local connection

      sockaddr_un address;
      address.sun_family = AF_UNIX;
      strcpy(address.sun_path, PEGASUS_LOCAL_DOMAIN_SOCKET_PATH);
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
      AtoE(address.sun_path);
#endif

      socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
      if (socket < 0)
         throw CannotCreateSocketException();

      // Connect the socket to the address:

      if (::connect(socket,
                    reinterpret_cast<sockaddr*>(&address),
                    sizeof(address)) < 0)
      {
	 
      	//l10n
         //throw CannotConnectException("Cannot connect to local CIM server. Connection failed.");
         MessageLoaderParms parms("Common.HTTPConnector2.CONNECTION_FAILED_LOCAL_CIM_SERVER",
         						  "Cannot connect to local CIM server. Connection failed.");
         throw CannotConnectException(parms);
      }
   }
   else
   {
#endif

   // Make the internet address:

   sockaddr_in address;

   if (!_MakeAddress((const char*)host.getCString(), portNumber, address))
   {
      char portStr [32];
      sprintf (portStr, "%u", portNumber);
      throw InvalidLocatorException(host + ":" + portStr);
   }


   // Create the socket:

   socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if (socket < 0)
      throw CannotCreateSocketException();

   // Conect the socket to the address:

   if (::connect(socket,
                 reinterpret_cast<sockaddr*>(&address),
                 sizeof(address)) < 0)
   {
      char portStr [32];
      sprintf (portStr, "%u", portNumber);
      //l10n
      //throw CannotConnectException("Cannot connect to " + host + ":" + portStr +". Connection failed.");
      MessageLoaderParms parms("Common.HTTPConnector2.CONNECTION_FAILED_TO",
         					   "Cannot connect to $0:$1. Connection failed.",
         					   host,
         					   portStr);
      throw CannotConnectException(parms);
   }

#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
   }
#endif

   // Create HTTPConnection2 object:

  //MP_Socket * mp_socket = new MP_Socket(socket, sslContext);
    bsd_socket_rep *bsdrep=new bsd_socket_rep(socket);
    pegasus_socket * peg_socket = new pegasus_socket(bsdrep);
  /* if (peg_socket->connect() < 0) {
      char portStr [32];
      sprintf (portStr, "%u", portNumber);
      //l10n
      //throw CannotConnectException("Cannot connect to " + host + ":" + portStr +". Connection failed.");
      MessageLoaderParms parms("Common.HTTPConnector2.CONNECTION_FAILED_TO",
         					   "Cannot connect to $0:$1. Connection failed.",
         					   host,
         					   portStr);
      throw CannotConnectException(parms);
   }*/

    
   HTTPConnection2* connection = new HTTPConnection2(*peg_socket,static_cast<MessageQueue *>(outputMessageQueue));
//   HTTPConnection* connection = new HTTPConnection(_monitor, mp_socket,
//	this, static_cast<MessageQueueService *>(outputMessageQueue));

   // Solicit events on this new connection's socket:
   _monitor->add_entry(*peg_socket, CLIENTSESSION, this, connection);

 /*
   if (-1 == (_entry_index = _monitor->solicitSocketMessages(
  socket,
  SocketMessage::READ | SocketMessage::EXCEPTION,
  connection->getQueueId(), Monitor::CONNECTOR)))
   {
      peg_socket->close();
   }
 */
   // Save the socket for cleanup later:

   _rep2->connections2.append(connection);

   return connection;
}

void HTTPConnector2::destroyConnections()
{
   // For each connection created by this object:

   for (Uint32 i = 0, n = _rep2->connections2.size(); i < n; i++)
   {
      _deleteConnection(_rep2->connections2[i]);
   }

   _rep2->connections2.clear();
}


void HTTPConnector2::disconnect(HTTPConnection2* currentConnection)
{
    //
    // find and delete the specified connection
    //
    for (Uint32 i = 0, n = _rep2->connections2.size(); i < n; i++)
    {
        if (currentConnection == _rep2->connections2[i])
        {
	   Sint32 socket = _rep2->connections2[i]->getSocket();
	   _monitor->unsolicitSocketMessages(socket);
	   _rep2->connections2.remove(i);

            Socket::close(socket);
            return;
        }
    }
}

void HTTPConnector2::_deleteConnection(HTTPConnection2* httpConnection)
{
    Sint32 socket = httpConnection->getSocket();

    // Unsolicit SocketMessages:

    _monitor->unsolicitSocketMessages(socket);

    // Destroy the connection (causing it to close):

    delete httpConnection;
}

PEGASUS_NAMESPACE_END
