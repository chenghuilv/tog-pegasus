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
// Modified By: Mike Day (monitor_2) mdday@us.ibm.com 
//              Amit K Arora (Bug#1153) amita@in.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>

#include <cstring>
#include "Monitor.h"
#include "MessageQueue.h"
#include "Socket.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Exception.h>

#ifdef PEGASUS_OS_TYPE_WINDOWS
# if defined(FD_SETSIZE) && FD_SETSIZE != 1024
#  error "FD_SETSIZE was not set to 1024 prior to the last inclusion \
of <winsock.h>. It may have been indirectly included (e.g., by including \
<windows.h>). Finthe inclusion of that header which is visible to this \
compilation unit and #define FD_SETZIE to 1024 prior to that inclusion; \
otherwise, less than 64 clients (the default) will be able to connect to the \
CIMOM. PLEASE DO NOT SUPPRESS THIS WARNING; PLEASE FIX THE PROBLEM."

# endif
# define FD_SETSIZE 1024
# include <windows.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


static AtomicInt _connections = 0;

static struct timeval create_time = {0, 1};
static struct timeval destroy_time = {300, 0};
static struct timeval deadlock_time = {0, 0};

////////////////////////////////////////////////////////////////////////////////
//
// MonitorRep
//
////////////////////////////////////////////////////////////////////////////////

struct MonitorRep
{
    fd_set rd_fd_set;
    fd_set wr_fd_set;
    fd_set ex_fd_set;
    fd_set active_rd_fd_set;
    fd_set active_wr_fd_set;
    fd_set active_ex_fd_set;
};

////////////////////////////////////////////////////////////////////////////////
//
// Monitor
//
////////////////////////////////////////////////////////////////////////////////

#define MAX_NUMBER_OF_MONITOR_ENTRIES  32
Monitor::Monitor()
   : _module_handle(0), 
     _controller(0),
     _async(false),
     _stopConnections(0),
     _stopConnectionsSem(0),
     _solicitSocketCount(0)
{
    int numberOfMonitorEntriesToAllocate = MAX_NUMBER_OF_MONITOR_ENTRIES;
    Socket::initializeInterface();
    _rep = 0;
    _entries.reserveCapacity(numberOfMonitorEntriesToAllocate);

    // setup the tickler
    initializeTickler();
    
    // Start the count at 1 because initilizeTickler() 
    // has added an entry in the first position of the 
    // _entries array
    for( int i = 1; i < numberOfMonitorEntriesToAllocate; i++ )
    {
       _MonitorEntry entry(0, 0, 0);
       _entries.append(entry);
    }
}

Monitor::Monitor(Boolean async)
   : _module_handle(0),
     _controller(0),
     _async(async),
     _stopConnections(0),
     _stopConnectionsSem(0),
     _solicitSocketCount(0)
{
    int numberOfMonitorEntriesToAllocate = MAX_NUMBER_OF_MONITOR_ENTRIES;
    Socket::initializeInterface();
    _rep = 0;
    _entries.reserveCapacity(numberOfMonitorEntriesToAllocate);

    // setup the tickler
    initializeTickler();

    // Start the count at 1 because initilizeTickler() 
    // has added an entry in the first position of the
    // _entries array
    for( int i = 1; i < numberOfMonitorEntriesToAllocate; i++ )
    {
       _MonitorEntry entry(0, 0, 0);
       _entries.append(entry);
    }
}

Monitor::~Monitor()
{
    Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                  "deregistering with module controller");

    if(_module_handle != NULL)
    {
       _controller->deregister_module(PEGASUS_MODULENAME_MONITOR);
       _controller = 0;
       delete _module_handle;
    }
    Tracer::trace(TRC_HTTP, Tracer::LEVEL4, "deleting rep");

    Tracer::trace(TRC_HTTP, Tracer::LEVEL4, "uninitializing interface");
    Socket::uninitializeInterface();
    Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                  "returning from monitor destructor");
}

void Monitor::initializeTickler(){
    /* 
       NOTE: On any errors trying to 
             setup out tickle connection, 
             throw an exception/end the server
    */

    /* setup the tickle server/listener */

    // get a socket for the server side
    if((_tickle_server_socket = ::socket(PF_INET, SOCK_STREAM, 0)) < 0){
	//handle error
	throw Exception("Monitor::initializeTickler(), create socket failed on tickle server.");
    }

    // initialize the address
    memset(&_tickle_server_addr, 0, sizeof(_tickle_server_addr));
#ifdef PEGASUS_OS_ZOS
    _tickle_server_addr.sin_addr.s_addr = inet_addr_ebcdic("127.0.0.1");
#else
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
#pragma convert(37)
#endif
    _tickle_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
#pragma convert(0)
#endif
#endif
    _tickle_server_addr.sin_family = PF_INET;
    _tickle_server_addr.sin_port = 0;
                                                                                                                             
    PEGASUS_SOCKLEN_SIZE _addr_size = sizeof(_tickle_server_addr);

    // bind server side to socket
    if((::bind(_tickle_server_socket,(struct sockaddr *)&_tickle_server_addr, sizeof(_tickle_server_addr))) < 0){
	// handle error
	throw Exception("Monitor::initializeTickler(), bind failed on tickle server socket.");
    }

    // tell the kernel we are a server
    if((::listen(_tickle_server_socket,3)) < 0){
	// handle error
	throw Exception("Monitor::initializeTickler(), listen failed on tickle server socket");
    }
    
    // make sure we have the correct socket for our server
    int sock = ::getsockname(_tickle_server_socket,(struct sockaddr*)&_tickle_server_addr, &_addr_size); 
    if(sock < 0){
	// handle error
	throw Exception("Monitor::initializeTickler(), getsockname failed on tickle server socket");
    }

    /* set up the tickle client/connector */
     
    // get a socket for our tickle client
    if((_tickle_client_socket = ::socket(PF_INET, SOCK_STREAM, 0)) < 0){
	// handle error
        throw Exception("Monitor::initializeTickler(), create socket failed on tickle client.");
    }

    // setup the address of the client
    memset(&_tickle_client_addr, 0, sizeof(_tickle_client_addr));
#ifdef PEGASUS_OS_ZOS
    _tickle_client_addr.sin_addr.s_addr = inet_addr_ebcdic("127.0.0.1");
#else
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
#pragma convert(37)
#endif
    _tickle_client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
#pragma convert(0)
#endif
#endif
    _tickle_client_addr.sin_family = PF_INET;
    _tickle_client_addr.sin_port = 0;

    // bind socket to client side
    if((::bind(_tickle_client_socket,(struct sockaddr*)&_tickle_client_addr, sizeof(_tickle_client_addr))) < 0){
	// handle error
	throw Exception("Monitor::initializeTickler(), bind failed on tickle client socket.");
    }

    // connect to server side
    if((::connect(_tickle_client_socket,(struct sockaddr*)&_tickle_server_addr, sizeof(_tickle_server_addr))) < 0){
	// handle error
	throw Exception("Monitor::initializeTickler(), connect failed between tickle client and tickle server.");
    }

    /* set up the slave connection */
    memset(&_tickle_peer_addr, 0, sizeof(_tickle_peer_addr));
    PEGASUS_SOCKLEN_SIZE peer_size = sizeof(_tickle_peer_addr);
    pegasus_sleep(1); 

    // this call may fail, we will try a max of 20 times to establish this peer connection
    if((_tickle_peer_socket = ::accept(_tickle_server_socket,(struct sockaddr*)&_tickle_peer_addr, &peer_size)) < 0){
        if(_tickle_peer_socket == -1 && errno == EAGAIN)
        {
          int retries = 0;                                                                        
          do
          {
            pegasus_sleep(1);
            _tickle_peer_socket = ::accept(_tickle_server_socket,(struct sockaddr*)&_tickle_peer_addr, &peer_size);
            retries++;
          } while(_tickle_peer_socket == -1 && errno == EAGAIN && retries < 20);
        }
    }
    if(_tickle_peer_socket == -1){
	// handle error
	throw Exception("Monitor::initializeTickler(), accept failed, peer socket connection not established.");
    }
    // add the tickler to the list of entries to be monitored and set to IDLE because Monitor only
    // checks entries with IDLE state for events
    _MonitorEntry entry(_tickle_peer_socket, 1, INTERNAL);
    entry._status = _MonitorEntry::IDLE;
    _entries.append(entry);
}

void Monitor::tickle(void)
{
  static char _buffer[] =
    {
      '0','0'
    };
                                                                                                                             
  Socket::disableBlocking(_tickle_client_socket);    
  Socket::write(_tickle_client_socket,&_buffer, 2);
  Socket::enableBlocking(_tickle_client_socket); 
}

Boolean Monitor::run(Uint32 milliseconds)
{

    Boolean handled_events = false;
     int i = 0;
   // #if defined(PEGASUS_OS_OS400) || defined(PEGASUS_OS_HPUX)
    struct timeval tv = {milliseconds/1000, milliseconds%1000*1000};
//#else
  //  struct timeval tv = {0, 1};
//#endif
    fd_set fdread;
    FD_ZERO(&fdread);
    _entry_mut.lock(pegasus_thread_self());
    
    // Check the stopConnections flag.  If set, clear the Acceptor monitor entries  
    if (_stopConnections == 1) 
    {
        for ( int indx = 0; indx < (int)_entries.size(); indx++)
        {
            if (_entries[indx]._type == Monitor::ACCEPTOR)
            {
                if ( _entries[indx]._status.value() != _MonitorEntry::EMPTY)
                {
                   if ( _entries[indx]._status.value() == _MonitorEntry::IDLE ||
                        _entries[indx]._status.value() == _MonitorEntry::DYING )
                   {
                       // remove the entry
		       _entries[indx]._status = _MonitorEntry::EMPTY;
                   }
                   else
                   {
                       // set status to DYING
                      _entries[indx]._status = _MonitorEntry::DYING;
                   }
               }
           }
        }
        _stopConnections = 0;
	_stopConnectionsSem.signal();
    }

    for( int indx = 0; indx < (int)_entries.size(); indx++)
    {
       if ((_entries[indx]._status.value() == _MonitorEntry::DYING) &&
                (_entries[indx]._type == Monitor::CONNECTION))
       {
          MessageQueue *q = MessageQueue::lookup(_entries[indx].queueId);
          PEGASUS_ASSERT(q != 0);
          MessageQueue & o = static_cast<HTTPConnection *>(q)->get_owner();
          Message* message= new CloseConnectionMessage(_entries[indx].socket);
          message->dest = o.getQueueId();

          // HTTPAcceptor is responsible for closing the connection. 
          // The lock is released to allow HTTPAcceptor to call
          // unsolicitSocketMessages to free the entry. 
          // Once HTTPAcceptor completes processing of the close
          // connection, the lock is re-requested and processing of
          // the for loop continues.  This is safe with the current
          // implementation of the _entries object.  Note that the
          // loop condition accesses the _entries.size() on each
          // iteration, so that a change in size while the mutex is
          // unlocked will not result in an ArrayIndexOutOfBounds
          // exception.

          _entry_mut.unlock();
          o.enqueue(message);
          _entry_mut.lock(pegasus_thread_self());
       }
    }

    Uint32 _idleEntries = 0;
   
    /*
	We will keep track of the maximum socket number and pass this value
	to the kernel as a parameter to SELECT.  This loop seems like a good 
	place to calculate the max file descriptor (maximum socket number)
	because we have to traverse the entire array.
    */ 
    int maxSocketCurrentPass = 0;
    for( int indx = 0; indx < (int)_entries.size(); indx++)
    {
       if(maxSocketCurrentPass < _entries[indx].socket)
	  maxSocketCurrentPass = _entries[indx].socket;

       if(_entries[indx]._status.value() == _MonitorEntry::IDLE)
       {
	  _idleEntries++;
	  FD_SET(_entries[indx].socket, &fdread);
       }
    }

    /*
	Add 1 then assign maxSocket accordingly. We add 1 to account for
	descriptors starting at 0.
    */
    maxSocketCurrentPass++;

    // Fixed in monitor_2 but added because Monitor is still the default monitor.
    // When _idleEntries is 0 don't immediately return, otherwise this loops out of control
    // kicking off kill idle thread threads.  E.g. There is nothing to select on when the cimserver
    // is shutting down.
    /*if( _idleEntries == 0 )
    {
        Thread::sleep( milliseconds );
        _entry_mut.unlock();
        return false;
    }*/
   
    _entry_mut.unlock(); 
    //int events = select(FD_SETSIZE, &fdread, NULL, NULL, &tv);
    int events = select(maxSocketCurrentPass, &fdread, NULL, NULL, &tv);
   _entry_mut.lock(pegasus_thread_self());

#ifdef PEGASUS_OS_TYPE_WINDOWS
    if(events == SOCKET_ERROR)
#else
    if(events == -1)
#endif
    {
       Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
          "Monitor::run - errorno = %d has occurred on select.", errno);
       // The EBADF error indicates that one or more or the file
       // descriptions was not valid. This could indicate that
       // the _entries structure has been corrupted or that
       // we have a synchronization error.

       PEGASUS_ASSERT(errno != EBADF);
    }
    else if (events)
    {
       Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
          "Monitor::run select event received events = %d, monitoring %d idle entries", 
	   events, _idleEntries);
       for( int indx = 0; indx < (int)_entries.size(); indx++)
       {
          // The Monitor should only look at entries in the table that are IDLE (i.e.,
          // owned by the Monitor).
	  if((_entries[indx]._status.value() == _MonitorEntry::IDLE) && 
	     (FD_ISSET(_entries[indx].socket, &fdread)))
	  {
	     MessageQueue *q = MessageQueue::lookup(_entries[indx].queueId);
             Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                  "Monitor::run indx = %d, queueId =  %d, q = %p",
                  indx, _entries[indx].queueId, q);
             PEGASUS_ASSERT(q !=0);

	     try 
	     {
		if(_entries[indx]._type == Monitor::CONNECTION)
		{
                   Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                     "_entries[indx].type for indx = %d is Monitor::CONNECTION", indx);
		   static_cast<HTTPConnection *>(q)->_entry_index = indx;
		   _entries[indx]._status = _MonitorEntry::BUSY;
                   // If allocate_and_awaken failure, retry on next iteration
/*
                   if (!MessageQueueService::get_thread_pool()->allocate_and_awaken(
                           (void *)q, _dispatch))
                   {
                      Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                          "Monitor::run: Insufficient resources to process request.");
                      _entries[indx]._status = _MonitorEntry::IDLE;
                      _entry_mut.unlock();
                      return true;
                   }
*/
// begin hack
		   HTTPConnection *dst = reinterpret_cast<HTTPConnection *>(q);
  			 Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                         "Monitor::_dispatch: entering run() for indx  = %d, queueId = %d, q = %p",
                   dst->_entry_index, dst->_monitor->_entries[dst->_entry_index].queueId, dst);
                   try
                   {
                       dst->run(1);
                   }
   		   catch (...)
   		   {
      			Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
          		"Monitor::_dispatch: exception received");
   		   }
   		   Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                   "Monitor::_dispatch: exited run() for index %d", dst->_entry_index);
                                                                                                                                                             
   		   PEGASUS_ASSERT(dst->_monitor->_entries[dst->_entry_index]._status.value() == _MonitorEntry::BUSY);
                                                                                                                                                             
   // Once the HTTPConnection thread has set the status value to either
   // Monitor::DYING or Monitor::IDLE, it has returned control of the connection
   // to the Monitor.  It is no longer permissible to access the connection
   // or the entry in the _entries table.
   		if (dst->_connectionClosePending)
   		{
      			dst->_monitor->_entries[dst->_entry_index]._status = _MonitorEntry::DYING;
   		}
   		else
   		{
      			dst->_monitor->_entries[dst->_entry_index]._status = _MonitorEntry::IDLE;
  		}	

// end hack
		}
	        else if( _entries[indx]._type == Monitor::INTERNAL){
			// set ourself to BUSY, 
                        // read the data  
                        // and set ourself back to IDLE
		
		   	_entries[indx]._status == _MonitorEntry::BUSY;
			static char buffer[2];
      			Socket::disableBlocking(_entries[indx].socket);
      			Sint32 amt = Socket::read(_entries[indx].socket,&buffer, 2);
      			Socket::enableBlocking(_entries[indx].socket);
			_entries[indx]._status == _MonitorEntry::IDLE;
		}
		else
		{
                   Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
                     "Non-connection entry, indx = %d, has been received.", indx);
		   int events = 0;
		   events |= SocketMessage::READ;
		   Message *msg = new SocketMessage(_entries[indx].socket, events);
		   _entries[indx]._status = _MonitorEntry::BUSY;
		   _entry_mut.unlock();

		   q->enqueue(msg);
		   _entries[indx]._status = _MonitorEntry::IDLE;
		   return true;
		}
	     }
	     catch(...)
	     {
	     }
	     handled_events = true;
	  }
       }
    }
    _entry_mut.unlock();
    return(handled_events);
}

void Monitor::stopListeningForConnections()
{
    PEG_METHOD_ENTER(TRC_HTTP, "Monitor::stopListeningForConnections()");
    // set boolean then tickle the server to recognize _stopConnections 
    _stopConnections = 1;
    tickle();

    // Wait for the monitor to notice _stopConnections.  Otherwise the
    // caller of this function may unbind the ports while the monitor
    // is still accepting connections on them.
    try
    {
      _stopConnectionsSem.time_wait(10000);
    }
    catch (TimeOut &)
    {
      // The monitor is probably busy processng a very long request, and is
      // not accepting connections.  Let the caller unbind the ports.
    }
    
    PEG_METHOD_EXIT();
}


int  Monitor::solicitSocketMessages(
    Sint32 socket, 
    Uint32 events,
    Uint32 queueId, 
    int type)
{
     PEG_METHOD_ENTER(TRC_HTTP, "Monitor::solicitSocketMessages");
                                                                                                                                                             
   _entry_mut.lock(pegasus_thread_self());
   // Check to see if we need to dynamically grow the _entries array
   // We always want the _entries array to 2 bigger than the
   // current connections requested
   _solicitSocketCount++;  // bump the count
   int size = (int)_entries.size();
   if(_solicitSocketCount >= (size-1)){
        for(int i = 0; i < (_solicitSocketCount - (size-1)); i++){
                _MonitorEntry entry(0, 0, 0);
                _entries.append(entry);
        }
   }
                                                                                                                                                             
   int index;
   for(index = 1; index < (int)_entries.size(); index++)
   {
      try
      {
         if(_entries[index]._status.value() == _MonitorEntry::EMPTY)
         {
            _entries[index].socket = socket;
            _entries[index].queueId  = queueId;
            _entries[index]._type = type;
            _entries[index]._status = _MonitorEntry::IDLE;
            _entry_mut.unlock();
                                                                                                                                                             
            return index;
         }
      }
      catch(...)
      {
      }
                                                                                                                                                             
   }
   _solicitSocketCount--;  // decrease the count, if we are here we didnt do anything meaningful
   _entry_mut.unlock();
   PEG_METHOD_EXIT();
   return -1;

}

void Monitor::unsolicitSocketMessages(Sint32 socket)
{

    PEG_METHOD_ENTER(TRC_HTTP, "Monitor::unsolicitSocketMessages");
    _entry_mut.lock(pegasus_thread_self());

    /*
        Start at index = 1 because _entries[0] is the tickle entry which never needs
        to be EMPTY;
    */
    int index;
    for(index = 1; index < _entries.size(); index++)
    {
       if(_entries[index].socket == socket)
       {
          _entries[index]._status = _MonitorEntry::EMPTY;
          _entries[index].socket = -1;
          _solicitSocketCount--;
          break;
       }
    }

    /*
	Dynamic Contraction:
	To remove excess entries we will start from the end of the _entries array
	and remove all entries with EMPTY status until we find the first NON EMPTY.
	This prevents the positions, of the NON EMPTY entries, from being changed.
    */ 
    index = _entries.size() - 1;
    while(_entries[index]._status == _MonitorEntry::EMPTY){
	if(_entries.size() > MAX_NUMBER_OF_MONITOR_ENTRIES)
                _entries.remove(index);
	index--;
    }

    _entry_mut.unlock();
    PEG_METHOD_EXIT();
}

PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL Monitor::_dispatch(void *parm)
{
   HTTPConnection *dst = reinterpret_cast<HTTPConnection *>(parm);
   Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
        "Monitor::_dispatch: entering run() for indx  = %d, queueId = %d, q = %p",
        dst->_entry_index, dst->_monitor->_entries[dst->_entry_index].queueId, dst);
   try
   {
      dst->run(1);
   }
   catch (...)
   {
      Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
          "Monitor::_dispatch: exception received");
   }
   Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
          "Monitor::_dispatch: exited run() for index %d", dst->_entry_index);
   
   PEGASUS_ASSERT(dst->_monitor->_entries[dst->_entry_index]._status.value() == _MonitorEntry::BUSY);

   // Once the HTTPConnection thread has set the status value to either
   // Monitor::DYING or Monitor::IDLE, it has returned control of the connection
   // to the Monitor.  It is no longer permissible to access the connection
   // or the entry in the _entries table.
   if (dst->_connectionClosePending)
   {
      dst->_monitor->_entries[dst->_entry_index]._status = _MonitorEntry::DYING;
   }
   else
   {
      dst->_monitor->_entries[dst->_entry_index]._status = _MonitorEntry::IDLE;
   }
   return 0;
}



////************************* monitor 2 *****************************////
////************************* monitor 2 *****************************////
////************************* monitor 2 *****************************////
////************************* monitor 2 *****************************////
////************************* monitor 2 *****************************////
////************************* monitor 2 *****************************////
////************************* monitor 2 *****************************////





m2e_rep::m2e_rep(void)
  :Base(), state(IDLE)

{
}

m2e_rep::m2e_rep(monitor_2_entry_type _type, 
		 pegasus_socket _sock, 
		 void* _accept, 
		 void* _dispatch)
  : Base(), type(_type), state(IDLE), psock(_sock), 
    accept_parm(_accept), dispatch_parm(_dispatch)
{
  
}

m2e_rep::~m2e_rep(void)
{
}

m2e_rep::m2e_rep(const m2e_rep& r)
  : Base()
{
  if(this != &r){
    type = r.type;
    psock = r.psock;
    accept_parm = r.accept_parm;
    dispatch_parm = r.dispatch_parm;
    state = IDLE;
    
  }
}


m2e_rep& m2e_rep::operator =(const m2e_rep& r)
{
  if(this != &r) {
    type = r.type;
    psock = r.psock;
    accept_parm = r.accept_parm;
    dispatch_parm = r.dispatch_parm;
    state = IDLE;
  }
  return *this;
}

Boolean m2e_rep::operator ==(const m2e_rep& r)
{
  if(this == &r)
    return true;
  return false;
}

Boolean m2e_rep::operator ==(void* r)
{
  if((void*)this == r)
    return true;
  return false;
}

m2e_rep::operator pegasus_socket() const 
{
  return psock;
}


monitor_2_entry::monitor_2_entry(void)
{
  _rep = new m2e_rep();
}

monitor_2_entry::monitor_2_entry(pegasus_socket& _psock, 
				 monitor_2_entry_type _type, 
				 void* _accept_parm, void* _dispatch_parm)
{
  _rep = new m2e_rep(_type, _psock, _accept_parm, _dispatch_parm);
}

monitor_2_entry::monitor_2_entry(const monitor_2_entry& e)
{
  if(this != &e){
    Inc(this->_rep = e._rep);
  }
}

monitor_2_entry::~monitor_2_entry(void)
{
   
  Dec(_rep);
}

monitor_2_entry& monitor_2_entry::operator=(const monitor_2_entry& e)
{
  if(this != &e){
    Dec(_rep);
    Inc(this->_rep = e._rep);
  }
  return *this;
}

Boolean monitor_2_entry::operator ==(const monitor_2_entry& me) const
{
  if(this == &me)
    return true;
  return false;
}

Boolean monitor_2_entry::operator ==(void* k) const
{
  if((void *)this == k)
    return true;
  return false;
}


monitor_2_entry_type monitor_2_entry::get_type(void) const
{
  return _rep->type;
}

void monitor_2_entry::set_type(monitor_2_entry_type t)
{
  _rep->type = t;
}


monitor_2_entry_state  monitor_2_entry::get_state(void) const
{
  return (monitor_2_entry_state) _rep->state.value();
}

void monitor_2_entry::set_state(monitor_2_entry_state t)
{
  _rep->state = t;
}

void* monitor_2_entry::get_accept(void) const
{
  return _rep->accept_parm;
}

void monitor_2_entry::set_accept(void* a)
{
  _rep->accept_parm = a;
}


void* monitor_2_entry::get_dispatch(void) const
{
  return _rep->dispatch_parm;
}

void monitor_2_entry::set_dispatch(void* a)
{
  _rep->dispatch_parm = a;
}

pegasus_socket monitor_2_entry::get_sock(void) const
{
  return _rep->psock;
}


void monitor_2_entry::set_sock(pegasus_socket& s)
{
  _rep->psock = s;
  
}

//static monitor_2* _m2_instance;

AsyncDQueue<HTTPConnection2> monitor_2::_connections(true, 0);

monitor_2::monitor_2(void)
  : _session_dispatch(0), _accept_dispatch(0), _listeners(true, 0), 
    _ready(true, 0), _die(0), _requestCount(0)
{
  try {
    
    bsd_socket_factory _factory;

    // set up the listener/acceptor 
    pegasus_socket temp = pegasus_socket(&_factory);
    
    temp.socket(PF_INET, SOCK_STREAM, 0);
    // initialize the address
    memset(&_tickle_addr, 0, sizeof(_tickle_addr));
#ifdef PEGASUS_OS_ZOS
    _tickle_addr.sin_addr.s_addr = inet_addr_ebcdic("127.0.0.1");
#else
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
#pragma convert(37)
#endif
    _tickle_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#ifdef PEGASUS_PLATFORM_OS400_ISERIES_IBM
#pragma convert(0)
#endif
#endif
    _tickle_addr.sin_family = PF_INET;
    _tickle_addr.sin_port = 0;

    PEGASUS_SOCKLEN_SIZE _addr_size = sizeof(_tickle_addr);
    
    temp.bind((struct sockaddr *)&_tickle_addr, sizeof(_tickle_addr));
    temp.listen(3);  
    temp.getsockname((struct sockaddr*)&_tickle_addr, &_addr_size);

    // set up the connector

    pegasus_socket tickler = pegasus_socket(&_factory);
    tickler.socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in _addr;
    memset(&_addr, 0, sizeof(_addr));
#ifdef PEGASUS_OS_ZOS
    _addr.sin_addr.s_addr = inet_addr_ebcdic("127.0.0.1");
#else
    _addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
    _addr.sin_family = PF_INET;
    _addr.sin_port = 0;
    tickler.bind((struct sockaddr*)&_addr, sizeof(_addr));
    tickler.connect((struct sockaddr*)&_tickle_addr, sizeof(_tickle_addr));

    _tickler.set_sock(tickler);
    _tickler.set_type(INTERNAL);
    _tickler.set_state(BUSY);
    
    struct sockaddr_in peer;
    memset(&peer, 0, sizeof(peer));
    PEGASUS_SOCKLEN_SIZE peer_size = sizeof(peer);

    pegasus_socket accepted = temp.accept((struct sockaddr*)&peer, &peer_size);
    
    monitor_2_entry* _tickle = new monitor_2_entry(accepted, INTERNAL, 0, 0);

// No need to set _tickle's state as BUSY, since monitor_2::run() now
// does a select only on sockets which are in IDLE (default) state.
//  _tickle->set_state(BUSY);
    
    _listeners.insert_first(_tickle);

  }
  catch(...){  }
}

monitor_2::~monitor_2(void)
{

   stop();

  try {
    monitor_2_entry* temp = _listeners.remove_first();
    while(temp){
      delete temp;
      temp = _listeners.remove_first();
    }
  }

  catch(...){  }
  

  try 
  {
     HTTPConnection2* temp = _connections.remove_first();
     while(temp)
     {
	delete temp;
	temp = _connections.remove_first();
     }
  }
  catch(...)
  {
  }
  

}


void monitor_2::run(void)
{
  monitor_2_entry* temp;
  int _nonIdle=0, _idleCount=0, events;

  while(_die.value() == 0) {
    _nonIdle=_idleCount=0;
     
     struct timeval tv_idle = { 60, 0 };
     
    // place all sockets in the select set 
    FD_ZERO(&rd_fd_set);
    try {
      _listeners.lock(pegasus_thread_self());
      temp = _listeners.next(0);
      Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
       "monitor_2::run:Creating New FD list for SELECT.");
      while(temp != 0 ){
	if(temp->get_state() == CLOSED ) {
	  monitor_2_entry* closed = temp;
      temp = _listeners.next(closed);
	  _listeners.remove_no_lock(closed);
      
      Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
       "monitor_2::run:Deleteing CLOSED socket fd=%d.",(Sint32)closed->get_sock());
	  
	  HTTPConnection2 *cn = monitor_2::remove_connection((Sint32)(closed->get_sock()));
	  delete cn;
	  delete closed;
	}
	if(temp == 0)
	   break;


        //Count the number if IDLE sockets
        if(temp->get_state() != IDLE ) _nonIdle++;
         else _idleCount++;

	Sint32 fd = (Sint32) temp->get_sock();

        //Select should be called ONLY on the FDs which are in IDLE state
	if((fd >= 0) && (temp->get_state() == IDLE))
        {
          Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
           "monitor_2::run:Adding FD %d to the list for SELECT.",fd);
	  FD_SET(fd , &rd_fd_set);
        }
	   temp = _listeners.next(temp);
      }
      _listeners.unlock();
    } 
    catch(...){
      return;
    }

    // important -  the dispatch routine has pointers to all the 
    // entries that are readable. These entries can be changed but 
    // the pointer must not be tampered with. 
    if(_connections.count() )
       events = select(FD_SETSIZE, &rd_fd_set, NULL, NULL, NULL);
    else
       events = select(FD_SETSIZE, &rd_fd_set, NULL, NULL, &tv_idle);
    
    if(_die.value())
    {
       break;
    }

#ifdef PEGASUS_OS_TYPE_WINDOWS
    if(events == SOCKET_ERROR)
#else
    if(events == -1)
#endif
    {
       Tracer::trace(TRC_HTTP, Tracer::LEVEL2,
          "monitor_2:run:INVALID FD. errorno = %d on select.", errno);
       // The EBADF error indicates that one or more or the file
       // descriptions was not valid. This could indicate that
       // the _entries structure has been corrupted or that
       // we have a synchronization error.

     // Keeping the line below commented for time being.
     //  PEGASUS_ASSERT(errno != EBADF);
    }
    else if (events)
    {
       Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
          "monitor_2::run select event received events = %d, monitoring %d idle entries", events, _idleCount);
   
    
    try {
      _listeners.lock(pegasus_thread_self());
      temp = _listeners.next(0);
      while(temp != 0 ){
	  Sint32 fd = (Sint32) temp->get_sock();
	  if(fd >= 0 && FD_ISSET(fd, &rd_fd_set)) {
	  if(temp->get_type() != CLIENTSESSION) temp->set_state(BUSY);
	  FD_CLR(fd,  &rd_fd_set);
	  monitor_2_entry* ready = new monitor_2_entry(*temp);
	  try 
	  {
	     _ready.insert_first(ready);
	  }
	  catch(...)
	  {
	  }
	  
	  _requestCount++;
	}
	temp = _listeners.next(temp);
      }
      _listeners.unlock();
    } 
    catch(...){
      return;
    }
    // now handle the sockets that are ready to read 
    if(_ready.count())
       _dispatch();
    else
    {
       if(_connections.count() == 0 )
	  _idle_dispatch(_idle_parm);
    }
   }  // if events
  } // while alive 
  _die=0;

}

int  monitor_2::solicitSocketMessages(
    Sint32 socket,
    Uint32 events,
    Uint32 queueId,
    int type)
{

   PEG_METHOD_ENTER(TRC_HTTP, "monitor_2::solicitSocketMessages");

   _entry_mut.lock(pegasus_thread_self());

   for(int index = 0; index < (int)_entries.size(); index++)
   {
      try
      {
	 if(_entries[index]._status.value() == monitor_2_entry::EMPTY)
	 {
	    _entries[index].socket = socket;
	    //_entries[index].queueId  = queueId;
	    //_entries[index]._type = type;
	    _entries[index]._status = IDLE;
	    _entry_mut.unlock();

	    return index;
	 }
      }
      catch(...)
      {
      }

   }
   _entry_mut.unlock();
   PEG_METHOD_EXIT();
   return -1;
}


void monitor_2::unsolicitSocketMessages(Sint32 socket)
{

    PEG_METHOD_ENTER(TRC_HTTP, "monitor_2::unsolicitSocketMessages");
    _entry2_mut.lock(pegasus_thread_self());

    for(int index = 0; index < (int)_entries2.size(); index++)
    {
       if(_entries2[index].socket == socket)
       {
	  _entries2[index]._status = monitor_2_entry::EMPTY; 
	  _entries2[index].socket = -1;
	  break;
       }
    }
    _entry2_mut.unlock();
    PEG_METHOD_EXIT();
}

void* monitor_2::set_session_dispatch(void (*dp)(monitor_2_entry*))
{
  void* old = (void *)_session_dispatch;
  _session_dispatch = dp;
  return old;
}

void* monitor_2::set_accept_dispatch(void (*dp)(monitor_2_entry*))
{
  void* old = (void*)_accept_dispatch;
  _accept_dispatch = dp;
  return old;
}

void* monitor_2::set_idle_dispatch(void (*dp)(void*))
{
   void* old = (void*)_idle_dispatch;
   _idle_dispatch = dp;
   return old;
}

void* monitor_2::set_idle_parm(void* parm)
{
   void* old = _idle_parm;
   _idle_parm = parm;
   return old;
}



//-----------------------------------------------------------------
// Note on deleting the monitor_2_entry nodes: 
//  Each case: in the switch statement needs to handle the deletion 
//  of the monitor_2_entry * node differently. A SESSION dispatch 
//  routine MUST DELETE the entry during its dispatch handling. 
//  All other dispatch routines MUST NOT delete the entry during the 
//  dispatch handling, but must allow monitor_2::_dispatch to delete
//   the entry. 
//
//  The reason is pretty obscure and it is debatable whether or not
//  to even bother, but during cimserver shutdown the single monitor_2_entry* 
//  will leak unless the _session_dispatch routine takes care of deleting it. 
//
//  The reason is that a shutdown messages completely stops everything and 
//  the _session_dispatch routine never returns. So monitor_2::_dispatch is 
//  never able to do its own cleanup. 
//
// << Mon Oct 13 09:33:33 2003 mdd >>
//-----------------------------------------------------------------

void monitor_2::_dispatch(void)
{
   monitor_2_entry* entry;
   
   try 
   {

	 entry = _ready.remove_first();
   }
   catch(...)
   {
   }
   
  while(entry != 0 ) {
    switch(entry->get_type()) {
    case INTERNAL:
      static char buffer[2];
      entry->get_sock().disableBlocking();
      entry->get_sock().read(&buffer, 2);
      entry->get_sock().enableBlocking();
      entry->set_state(IDLE);   // Set state of the socket to IDLE so that 
                                // monitor_2::run can add to the list of FDs
                                // on which select would be called.

     
 
      delete entry;
      
      break;
    case LISTEN:
      {
	static struct sockaddr peer;
	static PEGASUS_SOCKLEN_SIZE peer_size = sizeof(peer);
	entry->get_sock().disableBlocking();
	pegasus_socket connected = entry->get_sock().accept(&peer, &peer_size);
        entry->set_state(IDLE);  // Set state of the LISTEN socket to IDLE
#ifdef PEGASUS_OS_TYPE_WINDOWS
    if((Sint32)connected  == SOCKET_ERROR)
#else
	if((Sint32)connected == -1 )
#endif
	{
	   delete entry;
	   break;
	}
	
	entry->get_sock().enableBlocking();
	monitor_2_entry *temp = add_entry(connected, SESSION, entry->get_accept(), entry->get_dispatch());
	if(temp && _accept_dispatch != 0)
	   _accept_dispatch(temp);
	delete entry;
	
      }
      break;
    case SESSION:
    case CLIENTSESSION:
       if(_session_dispatch != 0 )
       {
	  // NOTE: _session_dispatch will delete entry - do not do it here
	     unsigned client=0;
         if(entry->get_type() == CLIENTSESSION) client = 1;
         Sint32 sock=(Sint32)(entry->get_sock());
     
	     _session_dispatch(entry);

         if(client)
         {
           HTTPConnection2 *cn = monitor_2::remove_connection(sock);
	       if(cn) delete cn;
           // stop();
           _die=1;
         }
       }
       
      else {
	static char buffer[4096];
	int bytes = entry->get_sock().read(&buffer, 4096);
	delete entry;
      }
    
      break;
    case UNTYPED:
    default:
           delete entry;
      break;
    }
    _requestCount--;
    
    if(_ready.count() == 0 )
       break;
    
    try 
    {
       entry = _ready.remove_first();
    }
    catch(...)
    {
    }
    
  }
}

void monitor_2::stop(void)
{
  _die = 1;
  tickle();
  // shut down the listener list, free the list nodes
  _tickler.get_sock().close();
  _listeners.shutdown_queue();
}

void monitor_2::tickle(void)
{
  static char _buffer[] = 
    {
      '0','0'
    };
  
  _tickler.get_sock().disableBlocking();
  
  _tickler.get_sock().write(&_buffer, 2);
  _tickler.get_sock().enableBlocking();
  
}


monitor_2_entry*  monitor_2::add_entry(pegasus_socket& ps, 
				       monitor_2_entry_type type,
				       void* accept_parm, 
				       void* dispatch_parm)
{
  Sint32 fd1,fd2;

  fd2=(Sint32) ps;

  monitor_2_entry* m2e = new monitor_2_entry(ps, type, accept_parm, dispatch_parm);

// The purpose of the following piece of code is to avoid duplicate entries in
// the _listeners list. Would it be too much of an overhead ?
try {

     monitor_2_entry* temp;

      _listeners.lock(pegasus_thread_self());
      temp = _listeners.next(0);
      while(temp != 0 )
      {
        fd1=(Sint32) temp->get_sock();

        if(fd1 == fd2)
        {

           Tracer::trace(TRC_HTTP, Tracer::LEVEL3,
          "monitor_2::add_entry:Request for duplicate entry in _listeners for %d FD.", fd1);
            if(temp->get_state() == CLOSED)
            {
              temp->set_state(IDLE);
              Tracer::trace(TRC_HTTP, Tracer::LEVEL3,
              "monitor_2::add_entry:CLOSED state changed to IDLE for %d.", fd1);
             }
             _listeners.unlock();
            delete m2e;
            return 0;
        }
       temp = _listeners.next(temp);
      }
   } 
   catch(...) 
   {
      delete m2e;
      return 0;
   }


  _listeners.unlock();


  try{
    _listeners.insert_first(m2e);
  }
  catch(...){
    delete m2e;
    return 0;
  }
      Tracer::trace(TRC_HTTP, Tracer::LEVEL4,
      "monitor_2::add_entry:SUCCESSFULLY added to _listeners list. FD = %d.", fd2);
  tickle();
  return m2e;
}

Boolean monitor_2::remove_entry(Sint32 s)
{
  monitor_2_entry* temp;
  try {
    _listeners.try_lock(pegasus_thread_self());
    temp = _listeners.next(0);
    while(temp != 0){
      if(s == (Sint32)temp->_rep->psock ){
	temp = _listeners.remove_no_lock(temp);
	delete temp;
	_listeners.unlock();
	return true;
      }
      temp = _listeners.next(temp);
    }
    _listeners.unlock();
  }
  catch(...){
  }
  return false;
}

Uint32 monitor_2::getOutstandingRequestCount(void)
{
  return _requestCount.value();
  
}


HTTPConnection2* monitor_2::remove_connection(Sint32 sock)
{

   HTTPConnection2* temp;
   try 
   {
      monitor_2::_connections.lock(pegasus_thread_self());
      temp = monitor_2::_connections.next(0);
      while(temp != 0 )
      {
	 if(sock == temp->getSocket())
	 {
	    temp = monitor_2::_connections.remove_no_lock(temp);
	    monitor_2::_connections.unlock();
	    return temp;
	 }
	 temp = monitor_2::_connections.next(temp);
      }
      monitor_2::_connections.unlock();
   }
   catch(...)
   {
   }
   return 0;
}

Boolean monitor_2::insert_connection(HTTPConnection2* connection)
{
   try 
   {
      monitor_2::_connections.insert_first(connection);
   }
   catch(...)
   {
      return false;
   }
   return true;
}


PEGASUS_NAMESPACE_END
