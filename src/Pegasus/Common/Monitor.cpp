//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>

#include <cstring>
#include "Monitor.h"
#include "MessageQueue.h"
#include "Socket.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/HTTPConnection.h>

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

Monitor::Monitor()
   : _module_handle(0), _controller(0), _async(false)
{
    Socket::initializeInterface();
    _rep = 0;
    _entries.reserveCapacity(32);
    for( int i = 0; i < 32; i++ )
    {
       _MonitorEntry entry(0, 0, 0);
       _entries.append(entry);
    }
}

Monitor::Monitor(Boolean async)
   : _module_handle(0), _controller(0), _async(async)
{
    Socket::initializeInterface();
    _rep = 0;
    _entries.reserveCapacity(32);
    for( int i = 0; i < 32; i++ )
    {
       _MonitorEntry entry(0, 0, 0);
       _entries.append(entry);
    }
    if( _async == true )
    {
       _thread_pool = new ThreadPool(0, 
				     "Monitor", 
				     0, 
				     0,
				     create_time, 
				     destroy_time, 
				     deadlock_time);
    }
    else 
       _thread_pool = 0;
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
    if(_async == true)
       delete _thread_pool;
}


int Monitor::kill_idle_threads()
{
   static struct timeval now, last;
   gettimeofday(&now, NULL);
   int dead_threads = 0;
   
   if( now.tv_sec - last.tv_sec > 120 )
   {
      gettimeofday(&last, NULL);
      try 
      {
	 dead_threads =  _thread_pool->kill_dead_threads();
      }
      catch(IPCException& )
      {
      }
      
   }
   return dead_threads;
}


Boolean Monitor::run(Uint32 milliseconds)
{

    Boolean handled_events = false;
     int i = 0;
    #if defined(PEGASUS_OS_OS400) || defined(PEGASUS_OS_HPUX)
    struct timeval tv = {milliseconds/1000, milliseconds%1000*1000};
#else
    struct timeval tv = {0, 1};
#endif
    fd_set fdread;
    FD_ZERO(&fdread);
    _entry_mut.lock(pegasus_thread_self());
    
    for( int indx = 0; indx < (int)_entries.size(); indx++)
    {
       if(_entries[indx]._status.value() == _MonitorEntry::IDLE)
       {
	  FD_SET(_entries[indx].socket, &fdread);
       }
    }

    
    int events = select(FD_SETSIZE, &fdread, NULL, NULL, &tv);

#ifdef PEGASUS_OS_TYPE_WINDOWS
    if(events && events != SOCKET_ERROR )
#else
    if(events && events != -1 )
#endif
    {
       for( int indx = 0; indx < (int)_entries.size(); indx++)
       {
	  if(FD_ISSET(_entries[indx].socket, &fdread))
	  {
	     MessageQueue *q = MessageQueue::lookup(_entries[indx].queueId);
	     if(q == 0)
	     {
		try
		{
		   _entries[indx]._status = _MonitorEntry::EMPTY;
		}
		catch(...)
		{

		}
		continue;
	     }
	     try 
	     {
		if(_entries[indx]._type == Monitor::CONNECTION)
		{
		   static_cast<HTTPConnection *>(q)->_entry_index = indx;
		   if(static_cast<HTTPConnection *>(q)->_dying.value() > 0 )
		   {
		      _entries[indx]._status = _MonitorEntry::DYING;
		      MessageQueue & o = static_cast<HTTPConnection *>(q)->get_owner();
		      Message* message= new CloseConnectionMessage(_entries[indx].socket);
		      message->dest = o.getQueueId();
		      _entry_mut.unlock();
		      o.enqueue(message);
		      return true;
		   }
		   _entries[indx]._status = _MonitorEntry::BUSY;
		   _thread_pool->allocate_and_awaken((void *)q, _dispatch);
		}
		else
		{
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



int  Monitor::solicitSocketMessages(
    Sint32 socket, 
    Uint32 events,
    Uint32 queueId, 
    int type)
{

   PEG_METHOD_ENTER(TRC_HTTP, "Monitor::solicitSocketMessages");

   int index = -1;
   _entry_mut.lock(pegasus_thread_self());
   
   for(index = 0; index < (int)_entries.size(); index++)
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
      _entry_mut.unlock();
   PEG_METHOD_EXIT();
   return index;
}

void Monitor::unsolicitSocketMessages(Sint32 socket)
{
    PEG_METHOD_ENTER(TRC_HTTP, "Monitor::unsolicitSocketMessages");
    _entry_mut.lock(pegasus_thread_self());
    
    for(int index = 0; index < (int)_entries.size(); index++)
    {
       if(_entries[index].socket == socket)
       {
	  _entries[index]._status = _MonitorEntry::EMPTY;
	  break;
       }
    }
    _entry_mut.unlock();
    PEG_METHOD_EXIT();
}

PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL Monitor::_dispatch(void *parm)
{
   HTTPConnection *dst = reinterpret_cast<HTTPConnection *>(parm);
   
   dst->run(1);
   if(  dst->_monitor->_entries.size() > (Uint32)dst->_entry_index )
      dst->_monitor->_entries[dst->_entry_index]._status = _MonitorEntry::IDLE;
   
   return 0;
}



////************************* monitor 2 *****************************////


monitor_2_entry::monitor_2_entry(void)
  : type(UNTYPED)
{
}

monitor_2_entry::monitor_2_entry(pegasus_socket& _psock, monitor_2_entry_type _type)
  : type(_type), psock(_psock)
{

}

monitor_2_entry::monitor_2_entry(const monitor_2_entry& e)
{
  if(this != &e){
    psock = e.psock;
    type = e.type;
  }
}

monitor_2_entry::~monitor_2_entry(void)
{

}

monitor_2_entry& monitor_2_entry::operator=(const monitor_2_entry& e)
{
  if(this != &e){
    psock = e.psock;
    type = e.type;
  }
  return *this;
}

Boolean monitor_2_entry::operator ==(const monitor_2_entry& me)
{
  if(this == &me)
    return true;
  if((Sint32)this->psock == (Sint32)me.psock)
    return true;
  return false;
}

Boolean monitor_2_entry::operator ==(void* k)
{
  if((void *)this == k)
    return true;
  return false;
}


monitor_2_entry::operator pegasus_socket() const
{
  return psock;
}


monitor_2::monitor_2(void)
  : _session_dispatch(0), _listeners(true, 0), _ready(true),  _die(0)
{
  try {
    
    bsd_socket_factory _factory;

    // set up the listener/acceptor 
    pegasus_socket temp = pegasus_socket(&_factory);
    
    temp.socket(PF_INET, SOCK_STREAM, 0);
    // initialize the address
    memset(&_tickle_addr, 0, sizeof(_tickle_addr));
    _tickle_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
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
    _addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    _addr.sin_family = PF_INET;
    _addr.sin_port = 0;
    tickler.bind((struct sockaddr*)&_addr, sizeof(_addr));
    tickler.connect((struct sockaddr*)&_tickle_addr, sizeof(_tickle_addr));

    _tickler.psock = tickler;
    _tickler.type = INTERNAL;

    struct sockaddr_in peer;
    memset(&peer, 0, sizeof(peer));
    PEGASUS_SOCKLEN_SIZE peer_size = sizeof(peer);

    pegasus_socket accepted = temp.accept((struct sockaddr*)&peer, &peer_size);
    monitor_2_entry* _tickle = new monitor_2_entry(accepted, INTERNAL);
    _listeners.insert_first(_tickle);

  }
  catch(...){  }
}

monitor_2::~monitor_2(void)
{
  PEGASUS_STD(cout) << "monitor destructor" << PEGASUS_STD(endl);
  
  try {
    monitor_2_entry* temp = _listeners.remove_first();
    while(temp){
      delete temp;
      temp = _listeners.remove_first();
    }
  }
  catch(...){  }
}


void monitor_2::run(void)
{
  monitor_2_entry* temp;
  while(_die.value() == 0) {
    struct timeval tv = {0, 0};

    // place all sockets in the select set 
    FD_ZERO(&rd_fd_set);
    try {
      _listeners.lock(pegasus_thread_self());
      temp = _listeners.next(0);
      while(temp != 0 ){
	FD_SET((Sint32)temp->psock, &rd_fd_set);
	temp = _listeners.next(temp);
      }
      _listeners.unlock();
    } 
    catch(...){
      return;
    }

    int events = select(FD_SETSIZE, &rd_fd_set, NULL, NULL, NULL);
    try {
      _listeners.lock(pegasus_thread_self());
     
      temp = _listeners.next(0);
      while(temp != 0 ){
	if(FD_ISSET((Sint32)temp->psock, &rd_fd_set)) {
	  FD_CLR((Sint32)temp->psock,  &rd_fd_set);
	  monitor_2_entry* entry = new monitor_2_entry(*temp);
	  _ready.insert_first((void*)entry);
	}
	temp = _listeners.next(temp);
      }
      _listeners.unlock();
    } 
    catch(...){
      return;
    }

    // now handle the sockets that are ready to read 
    _dispatch();
  } // while alive 
}

void* monitor_2::set_session_dispatch(void (*dp)(pegasus_socket&))
{
  void* old = (void*)_session_dispatch;
  _session_dispatch = dp;
  return old;
}


void monitor_2::_dispatch(void)
{
  monitor_2_entry* entry = (monitor_2_entry*) _ready.remove_first();
  while(entry != 0 ){
    switch(entry->type) {
    case INTERNAL:
      static char buffer[2];
      entry->psock.read(&buffer, 2);
      break;
    case LISTEN:
      {
	static struct sockaddr peer;
	static PEGASUS_SOCKLEN_SIZE peer_size = sizeof(peer);
	pegasus_socket connected = entry->psock.accept(&peer, &peer_size);
	add_entry(connected, SESSION);
      }
      break;
    case SESSION:
      if(_session_dispatch != 0 )
	_session_dispatch(entry->psock);
      else {
	static char buffer[4096];
	int bytes = entry->psock.read(&buffer, 4096);
      }
    
      break;
    
    case UNTYPED:
    default:
      break;
    
    }
    delete entry;
    entry = (monitor_2_entry*) _ready.remove_first();
  }
}


void monitor_2::stop(void)
{
  _die = 1;
  tickle();
  
  // shut down the listener list, free the list nodes
  _tickler.psock.close();
  _listeners.shutdown_queue();
}

void monitor_2::tickle(void)
{
  static char _buffer[] = 
    {
      '0','0'
    };
  
  _tickler.psock.write(&_buffer, 2);
}


Boolean monitor_2::add_entry(pegasus_socket& ps, monitor_2_entry_type type)
{
  monitor_2_entry* m2e = new monitor_2_entry(ps, type);
  
  try{
    _listeners.insert_first(m2e);
  }
  catch(...){
    delete m2e;
    return false;
  }
  tickle();
  return true;
}

Boolean monitor_2::remove_entry(Sint32 s)
{
  monitor_2_entry* temp;
  try {
    _listeners.try_lock(pegasus_thread_self());
    temp = _listeners.next(0);
    while(temp != 0){
      if(s == (Sint32)temp->psock ){
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



PEGASUS_NAMESPACE_END
