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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Monitor_h 
#define Pegasus_Monitor_h 

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/Common/pegasus_socket.h>
#include <Pegasus/Common/DQueue.h>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/Linkage.h> 

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE _MonitorEntry
{
public:
  Sint32 socket;
  Uint32 queueId;
  AtomicInt _status;
  int _type;
      
  _MonitorEntry(Sint32 sock, Uint32 q, int Type)
    : socket(sock), queueId(q), _status(EMPTY), _type(Type)
	   
  {
  }

  _MonitorEntry() : socket(0), queueId(0), _status(EMPTY), _type(0)
  {
  }
      
  Boolean operator ==(const void *key) const 
  {
    if(key != 0 && 
       (socket == (reinterpret_cast<_MonitorEntry *>(const_cast<void *>(key)))->socket))
      return true;
    return false;
  }
      
  Boolean operator ==(const _MonitorEntry & key) const
  {
    if(key.socket == socket)
      return true;
    return false;
  }

  _MonitorEntry & operator =(const _MonitorEntry & entry)
  {
    if( this != &entry )
      {
	this->socket = entry.socket;
	this->queueId = entry.queueId;
	this->_status = entry._status;
	this->_type = entry._type;
      }
	 
    return *this;
  }
      
  enum entry_status 
    {
      IDLE,
      BUSY,
      DYING,
      EMPTY
    };
};

struct MonitorRep;

/** This message occurs when there is activity on a socket. */
class SocketMessage : public Message
{
public:

  enum Events { READ = 1, WRITE = 2, EXCEPTION = 4 };
      

  SocketMessage(Sint32 socket_, Uint32 events_) :
    Message(SOCKET_MESSAGE), socket(socket_), events(events_)
  {
  }

  Sint32 socket;
  Uint32 events;
};

/** This class monitors system-level events and notifies its clients of these
    events by posting messages to their queues.

    The monitor generates following message types:

    <ul>
    <li> SocketMessage - occurs when activity on a socket </li>
    </ul>

    Clients solicit these messages by calling one of the following methods:

    <ul>
    <li> solicitSocketMessages() </li>
    </ul>

    The following example shows how to solicit socket messages:

    <pre>
    Monitor monitor;
    Sint32 socket;
    Uint32 queueId;


    ...

    monitor.solicitSocketMessages(
    socket, 
    SocketMessage::READ | SocketMessage::WRITE, 
    queueId);
    </pre>

    Each time activity occurs on the given socket, a SocketMessage is
    enqueued on the given queue.

    In order the monitor to generate messages, it must be run by calling
    the run() method as shown below:

    <pre>
    Monitor monitor;

    ...

    Uint32 milliseconds = 5000;
    monitor.run(milliseconds);
    </pre>

    In this example, the monitor is run for five seconds. The run method
    returns after the first message is occurs or five seconds has transpired
    (whichever occurs first).
*/
class PEGASUS_COMMON_LINKAGE Monitor
{
public:
  enum Type 
    {
      UNTYPED, ACCEPTOR, CONNECTOR, CONNECTION
    };
      
      
  /** Default constructor. */
  Monitor();
  Monitor(Boolean async);
    

  /** This destruct deletes all handlers which were installed. */
  ~Monitor();

  /** Monitor system-level for the given number of milliseconds. Post a
      message to the corresponding queue when such an event occurs.
      Return after the time has elapsed or a single event has occurred,
      whichever occurs first.

      @param timeoutMsec the number of milliseconds to wait for an event.
      @return true if an event occured.
  */
  Boolean run(Uint32 timeoutMsec);

  /** Solicit interest in SocketMessages. Note that there may only 
      be one solicitor per socket.

      @param socket the socket to monitor for activity.
      @param events socket events to monitor (see the SocketMessage::Events
      enumeration for details).
      @param queueId of queue on which to post socket messages.
      @return false if messages have already been solicited on this socket.
  */
  int solicitSocketMessages(
			    Sint32 socket, 
			    Uint32 events,
			    Uint32 queueId,
			    int type);

  /** Unsolicit messages on the given socket.

  @param socket on which to unsolicit messages.
  @return false if no such solicitation has been made on the given socket.
  */
  void unsolicitSocketMessages(Sint32);

  /** dispatch a message to the cimom on an independent thread 
      Note: The Monitor class uses the MessageQueueService ThreadPool.
      This ThreadPool is only available if it has been initialized by
      the MessageQueueService.  Therefore, the Monitor class should
      only be used when the MessageQueueService is active in the
      system.
   */
  static PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL _dispatch(void *);
      
  /** stop listening for client connections 
   */
  void stopListeningForConnections();

private:
      
  Array<_MonitorEntry> _entries;
  MonitorRep* _rep;
  pegasus_module * _module_handle;
  ModuleController * _controller;
  Boolean _async;
  Mutex _entry_mut;
  AtomicInt _stopConnections;
  friend class HTTPConnection;
      
};

enum monitor_2_entry_type { UNTYPED, INTERNAL, LISTEN, CONNECT, SESSION };
enum monitor_2_entry_state {IDLE, BUSY, CLOSED };



class PEGASUS_COMMON_LINKAGE m2e_rep : public Sharable
{
public:
  typedef Sharable Base;
  
  m2e_rep();
  m2e_rep(monitor_2_entry_type, pegasus_socket, void*, void*);
  ~m2e_rep();
  m2e_rep(const m2e_rep& );
  m2e_rep& operator =(const m2e_rep& );
  Boolean operator ==(const m2e_rep& );
  Boolean operator ==(void*);
  operator pegasus_socket() const;
    

  monitor_2_entry_type type;
  AtomicInt state;

  pegasus_socket psock;
  void* accept_parm;
  void* dispatch_parm;
};



class PEGASUS_COMMON_LINKAGE monitor_2_entry
{
public:
  Sint32 socket;
  AtomicInt _status;

  monitor_2_entry(void);
  monitor_2_entry(pegasus_socket&, monitor_2_entry_type _type, void* accept, void* dispatch);
  monitor_2_entry(const monitor_2_entry&);
  monitor_2_entry(m2e_rep* );
  
  ~monitor_2_entry(void);

  monitor_2_entry& operator=(const monitor_2_entry&);
  Boolean operator ==(const monitor_2_entry& ) const;
  Boolean operator ==(void*) const;
   
  
  monitor_2_entry_type get_type() const;
  void set_type(monitor_2_entry_type);

  monitor_2_entry_state get_state() const;
  void set_state(monitor_2_entry_state);

  pegasus_socket get_sock(void) const;
  
  void set_sock(pegasus_socket&);

  void* get_accept(void) const;
  void set_accept(void*);
   
  void* get_dispatch(void) const;
  void set_dispatch(void*);
  
  m2e_rep* _rep;
	enum entry_status {IDLE, BUSY, DYING, EMPTY};
  
};

class HTTPConnection2;

class PEGASUS_COMMON_LINKAGE monitor_2 
{

   public:
      
      monitor_2(void);
      ~monitor_2(void);
      monitor_2_entry* add_entry(pegasus_socket& , monitor_2_entry_type, void*, void*);
      Boolean remove_entry(Sint32 );
      void tickle(void);
      void run(void);
      void stop(void);
      Array<monitor_2_entry> _entries2;
      Mutex _entry2_mut;

      void unsolicitSocketMessages(Sint32);
      int solicitSocketMessages(
          Sint32 socket,
          Uint32 events,
          Uint32 queueId,
          int type);
   
      
      void* set_session_dispatch( void (*)(monitor_2_entry*));
      void* set_accept_dispatch( void (*)(monitor_2_entry*));
      void* set_idle_dispatch( void(*)(void*));
      void* set_idle_parm(void* );
      
      

      Uint32 getOutstandingRequestCount(void);
      
      static HTTPConnection2* remove_connection(Sint32 sock);
      static Boolean insert_connection(HTTPConnection2* connection);
      
//      static monitor_2* get_monitor2(void);
      

   private:


      void _dispatch(void);
      void (*_session_dispatch)(monitor_2_entry*);
      void (*_accept_dispatch)(monitor_2_entry*);
      void (*_idle_dispatch)(void*);
      void* _idle_parm;
      
      AsyncDQueue<monitor_2_entry> _listeners;
      AsyncDQueue<monitor_2_entry> _ready;
      static AsyncDQueue<HTTPConnection2> _connections;
      
      
      monitor_2_entry _tickler;
      struct sockaddr_in _tickle_addr;
      AtomicInt _die;
      fd_set rd_fd_set;  
      AtomicInt _requestCount;
      Mutex _entry_mut;
      Array<monitor_2_entry> _entries;
};




PEGASUS_NAMESPACE_END

#endif /* Pegasus_Monitor_h */
