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
// Modified By: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                  (carolann_graves@hp.com)
//              Mike Day (mdday@us.ibm.com)
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Arthur Pichlkostner (via Markus: sedgewick_de@yahoo.de)
//				Willis White (whiwill@us.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Message_h
#define Pegasus_Message_h

#include <Pegasus/Common/Config.h>
#include <iostream>
#include <cstring>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/IPC.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMDateTime.h>

PEGASUS_NAMESPACE_BEGIN

// REVIEW: could class be renamed to MessageMask (coding standard)

class PEGASUS_COMMON_LINKAGE message_mask
{
   public:

      static Uint32 type_legacy;
      static Uint32 type_CIMOperation;
      static Uint32 type_CIMAsyncOperation;
      static Uint32 type_export;
      static Uint32 type_lifetime;
      static Uint32 type_socket;
      static Uint32 type_connection;
      static Uint32 type_http;
      static Uint32 type_http_error;
      static Uint32 type_cimom;
      static Uint32 type_control;
      static Uint32 type_service;
      static Uint32 type_broadcast;
      static Uint32 type_client_exception;

      static Uint32 ha_no_delete;
      static Uint32 ha_request;
      static Uint32 ha_reply;
      static Uint32 ha_synchronous;
      static Uint32 ha_async;
      static Uint32 ha_wait;


      // more for documentation than for use

      inline Uint32 get_type(Uint32 flags)
      {
	 return (flags & 0x000fffff);
      }

      inline Uint32 get_handling(Uint32 flags)
      {
	 return( flags & 0xfff00000);
      }
};

class cimom;
class MessageQueue;
class MessageQueueService;
class AsyncLegacyOperationStart;
class AsyncLegacyOperationResult;

enum HttpMethod
{
    HTTP_METHOD__POST,
    HTTP_METHOD_M_POST
};

/** The Message class and derived classes are used to pass messages between
    modules. Messages are passed between modules using the message queues
    (see MessageQueue class). Derived classes may add their own fields.
    This base class defines two common fields: type, which is the type of
    the message, and key which is a key value whose meaning is defined by
    the derived class. The MessageQueue class provides methods for finding
    messages by both type and key.

    The Message class also provides previous and next pointers which are
    used to place the messages on a queue by the MessageQueue class.
*/
class PEGASUS_COMMON_LINKAGE Message
{
   public:

      Message(
	 Uint32 type,
	 Uint32 destination = 0,
	 Uint32 key = getNextKey(),
	 Uint32 routing_code = 0,
	 Uint32 mask = message_mask::type_legacy )
	 :
	 _type(type),
	 _key(key),
	 _routing_code(routing_code),
	 _mask(mask),
         _httpMethod (HTTP_METHOD__POST),
	_last_thread_id(pegasus_thread_self()),
	 _next(0),
	 _prev(0),
	 _async(0),
	 dest(destination)
      {

      }

      Message & operator = ( const Message & msg)
      {
	 if (this != &msg)
	 {
	    _type = msg._type;
	    _key = msg._key;
	    _routing_code = msg._routing_code;
	    _mask = msg._mask;
	    _last_thread_id = msg._last_thread_id;
	    _next = _prev = _async = 0;
	    dest = msg.dest;
            _httpMethod = msg._httpMethod;
	    
	 }
	 return *this;
      }


      virtual ~Message();

      Uint32 getType() const { return _type; }

      void setType(Uint32 type) { _type = type; }

      Uint32 getKey() const { return _key; }

      void setKey(Uint32 key) { _key = key; }

      Uint32 getRouting() const { return _routing_code; }
      void setRouting(Uint32 routing) { _routing_code = routing; }

      Uint32 getMask() const { return _mask; }

      void setMask(Uint32 mask) { _mask = mask; }

      HttpMethod getHttpMethod() const { return _httpMethod; }

      void setHttpMethod(HttpMethod httpMethod) {_httpMethod = httpMethod;}

#ifdef PEGASUS_HAS_PERFINST
//
// Needed for performance measurement
//

      void startServer();

      void endServer();

      void startProvider();

      void endProvider();

      CIMDateTime getStartServerTime() const { return _timeServerStart; }

      void setStartServerTime(CIMDateTime timeServerStart)
      {
           _timeServerStart = timeServerStart;
      }

      CIMDateTime getStartProviderTime() const { return _timeProviderStart; }

      void setStartProviderTime(CIMDateTime timeProviderStart)
      {
          _timeProviderStart = timeProviderStart;
      }

      CIMDateTime getEndServerTime() const { return _timeServerEnd; }

      void setEndServerTime (CIMDateTime timeServerEnd)
      {
          _timeServerEnd = timeServerEnd;
      }

      CIMDateTime getEndProviderTime() const { return _timeProviderEnd; }

      void setEndProviderTime(CIMDateTime timeProviderEnd)
      {
          _timeProviderEnd = timeProviderEnd;
      }

	  CIMDateTime getProviderTime() { return _providerTime;}

      CIMDateTime getTotalTime() { return _totalTime; }

	  CIMDateTime getServerTime() { return _serverTime; }
//
#endif

      Message* getNext() { return _next; }

      const Message* getNext() const { return _next; }

      Message* getPrevious() { return _prev; }

      const Message* getPrevious() const { return _prev; }

      static Uint32 getNextKey()
      {
	
	 _mut.lock( pegasus_thread_self() ) ;
	 Uint32 ret = _nextKey++;
	 _mut.unlock();
	 return ret;
      }

      virtual void print(
	  PEGASUS_STD(ostream)& os, 
	  Boolean printHeader = true) const;

      // << Thu Dec 27 10:46:04 2001 mdd >> for use with DQueue container
      // as used by AsyncOpNode
      Boolean operator == (const void *msg )
      {
	 if (reinterpret_cast<void *>(this) == msg )
	    return true;
	 return false;
      }

      Message *get_async(void)
      {
	 Message *ret = _async;
	 _async = 0;
	 return ret;
	
      }

      void put_async(Message * msg)
      {
	 _async = msg;
      }

      // << Tue Jul  1 11:02:49 2003 mdd >> pep_88 and helper for i18n and l10n
      Boolean thread_changed(void)
      {
	 if(_last_thread_id != pegasus_thread_self())
	 {
	    _last_thread_id = pegasus_thread_self();
	    return true;
	 }

	 return false;
      }
      
      // << Tue Jul  1 13:41:02 2003 mdd >> pep_88 - 
      // assist in synchronizing responses with requests

      void synch_response(Message *req)
      {
	 _key = req->_key;
	 _routing_code = req->_routing_code;
      }
      

   private:
      Uint32 _type;
      Uint32 _key;
      Uint32 _routing_code;
      Uint32 _mask;
      HttpMethod _httpMethod;
// Needed for performance measurement
      CIMDateTime _timeServerStart;
      CIMDateTime _timeServerEnd;
      CIMDateTime _timeProviderStart;
      CIMDateTime _timeProviderEnd;
	  CIMDateTime _providerTime;
	  CIMDateTime _serverTime;	
      CIMDateTime _totalTime;
//

      // << Tue Jul  1 11:02:35 2003 mdd >> pep_88 and helper for i18n and l10n
      PEGASUS_THREAD_TYPE _last_thread_id;
      
      Message* _next;
      Message* _prev;

   protected:

   public:
      Message *_async;
      Uint32 dest;
   private:
      MessageQueue* _owner;
      static Uint32 _nextKey;
      static Mutex _mut;



      friend class cimom;
      friend class MessageQueue;
      friend class MessageQueueService;
      friend class AsyncLegacyOperationStart;
      friend class AsyncLegacyOperationResult;

};


enum MessageType
{
    DUMMY_MESSAGE,

    // CIM Message types:

    CIM_GET_CLASS_REQUEST_MESSAGE,
    CIM_GET_INSTANCE_REQUEST_MESSAGE,
    CIM_EXPORT_INDICATION_REQUEST_MESSAGE,
    CIM_DELETE_CLASS_REQUEST_MESSAGE,
    CIM_DELETE_INSTANCE_REQUEST_MESSAGE,
    CIM_CREATE_CLASS_REQUEST_MESSAGE,
    CIM_CREATE_INSTANCE_REQUEST_MESSAGE,
    CIM_MODIFY_CLASS_REQUEST_MESSAGE,
    CIM_MODIFY_INSTANCE_REQUEST_MESSAGE,
    CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE, //10
    CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE,
    CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE,
    CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE,
    CIM_EXEC_QUERY_REQUEST_MESSAGE,
    CIM_ASSOCIATORS_REQUEST_MESSAGE,
    CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE,
    CIM_REFERENCES_REQUEST_MESSAGE,
    CIM_REFERENCE_NAMES_REQUEST_MESSAGE,
    CIM_GET_PROPERTY_REQUEST_MESSAGE,
    CIM_SET_PROPERTY_REQUEST_MESSAGE, //20
    CIM_GET_QUALIFIER_REQUEST_MESSAGE,
    CIM_SET_QUALIFIER_REQUEST_MESSAGE,
    CIM_DELETE_QUALIFIER_REQUEST_MESSAGE,
    CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE,
    CIM_INVOKE_METHOD_REQUEST_MESSAGE,
    CIM_ENABLE_INDICATION_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_MODIFY_INDICATION_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_DISABLE_INDICATION_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_PROCESS_INDICATION_REQUEST_MESSAGE,
    CIM_HANDLE_INDICATION_REQUEST_MESSAGE, // 30
    CIM_NOTIFY_PROVIDER_REGISTRATION_REQUEST_MESSAGE,
    CIM_NOTIFY_PROVIDER_TERMINATION_REQUEST_MESSAGE,

    // new
    CIM_ENABLE_INDICATIONS_REQUEST_MESSAGE,
    CIM_DISABLE_INDICATIONS_REQUEST_MESSAGE,
    CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE,

    // new
    CIM_DISABLE_MODULE_REQUEST_MESSAGE,
    CIM_ENABLE_MODULE_REQUEST_MESSAGE,

    CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE,  // 40

    CIM_GET_CLASS_RESPONSE_MESSAGE,
    CIM_GET_INSTANCE_RESPONSE_MESSAGE,
    CIM_EXPORT_INDICATION_RESPONSE_MESSAGE,
    CIM_DELETE_CLASS_RESPONSE_MESSAGE,
    CIM_DELETE_INSTANCE_RESPONSE_MESSAGE,
    CIM_CREATE_CLASS_RESPONSE_MESSAGE,
    CIM_CREATE_INSTANCE_RESPONSE_MESSAGE,
    CIM_MODIFY_CLASS_RESPONSE_MESSAGE,
    CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE,
    CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE,  // 50
    CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE,
    CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE,
    CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE,
    CIM_EXEC_QUERY_RESPONSE_MESSAGE,
    CIM_ASSOCIATORS_RESPONSE_MESSAGE,
    CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE,
    CIM_REFERENCES_RESPONSE_MESSAGE,
    CIM_REFERENCE_NAMES_RESPONSE_MESSAGE,
    CIM_GET_PROPERTY_RESPONSE_MESSAGE,
    CIM_SET_PROPERTY_RESPONSE_MESSAGE,  // 60
    CIM_GET_QUALIFIER_RESPONSE_MESSAGE,
    CIM_SET_QUALIFIER_RESPONSE_MESSAGE,
    CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE,
    CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE,
    CIM_INVOKE_METHOD_RESPONSE_MESSAGE,
    CIM_ENABLE_INDICATION_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_MODIFY_INDICATION_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_DISABLE_INDICATION_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_PROCESS_INDICATION_RESPONSE_MESSAGE,
    CIM_NOTIFY_PROVIDER_REGISTRATION_RESPONSE_MESSAGE,  // 70
    CIM_NOTIFY_PROVIDER_TERMINATION_RESPONSE_MESSAGE,
    CIM_HANDLE_INDICATION_RESPONSE_MESSAGE,

    // new
    CIM_ENABLE_INDICATIONS_RESPONSE_MESSAGE,
    CIM_DISABLE_INDICATIONS_RESPONSE_MESSAGE,
    CIM_CREATE_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_MODIFY_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_DELETE_SUBSCRIPTION_RESPONSE_MESSAGE,

    // new
    CIM_DISABLE_MODULE_RESPONSE_MESSAGE,
    CIM_ENABLE_MODULE_RESPONSE_MESSAGE,

    CIM_STOP_ALL_PROVIDERS_RESPONSE_MESSAGE,  // 80

    // Monitor-related messages:

    SOCKET_MESSAGE,

    // Connection-oriented messages:

    CLOSE_CONNECTION_MESSAGE,

    // HTTP messages:

    HTTP_MESSAGE,
    HTTP_ERROR_MESSAGE,

    // Exception messages to be passed to a CIM client application:

    CLIENT_EXCEPTION_MESSAGE,

    ASYNC_REGISTER_CIM_SERVICE,
    ASYNC_DEREGISTER_CIM_SERVICE,
    ASYNC_UPDATE_CIM_SERVICE,
    ASYNC_IOCTL,
    ASYNC_CIMSERVICE_START,  // 90
    ASYNC_CIMSERVICE_STOP,
    ASYNC_CIMSERVICE_PAUSE,
    ASYNC_CIMSERVICE_RESUME,

    ASYNC_ASYNC_OP_START,
    ASYNC_ASYNC_OP_RESULT,
    ASYNC_ASYNC_LEGACY_OP_START,
    ASYNC_ASYNC_LEGACY_OP_RESULT,

    ASYNC_FIND_SERVICE_Q,
    ASYNC_FIND_SERVICE_Q_RESULT,
    ASYNC_ENUMERATE_SERVICE,  // 100
    ASYNC_ENUMERATE_SERVICE_RESULT,

    ASYNC_REGISTERED_MODULE,
    ASYNC_DEREGISTERED_MODULE,
    ASYNC_FIND_MODULE_IN_SERVICE,
    ASYNC_FIND_MODULE_IN_SERVICE_RESPONSE,

    ASYNC_ASYNC_MODULE_OP_START,
    ASYNC_ASYNC_MODULE_OP_RESULT,

	CIM_INITIALIZE_PROVIDER_REQUEST_MESSAGE,
    CIM_INITIALIZE_PROVIDER_RESPONSE_MESSAGE,


    CIM_NOTIFY_PROVIDER_ENABLE_REQUEST_MESSAGE,
    CIM_NOTIFY_PROVIDER_ENABLE_RESPONSE_MESSAGE,

    NUMBER_OF_MESSAGES
};

PEGASUS_COMMON_LINKAGE const char* MessageTypeToString(Uint32 messageType);

/** This class implements a stack of queue-ids. Many messages must keep a
    stack of queue-ids of queues which they must be returned to. This provides
    a light efficient stack for this purpose.
*/
class QueueIdStack
{
public:

    QueueIdStack() : _size(0)
    {
    }

    QueueIdStack(const QueueIdStack& x) : _size(x._size)
    {
	memcpy(_items, x._items, sizeof(_items));
    }

    PEGASUS_EXPLICIT QueueIdStack(Uint32 x) : _size(0)
    {
	push(x);
    }

    PEGASUS_EXPLICIT QueueIdStack(Uint32 x1, Uint32 x2) : _size(0)
    {
	push(x1);
	push(x2);
    }

    ~QueueIdStack()
    {
    }

    QueueIdStack& operator=(const QueueIdStack& x)
    {
	if (this != &x)
	{
	    memcpy(_items, x._items, sizeof(_items));
	    _size = x._size;
	}
	return *this;
    }

    Uint32 size() const
    {
	return _size;
    }

    Boolean isEmpty() const
    {
	return _size == 0;
    }

    void push(Uint32 x)
    {
	if (_size == MAX_SIZE)
	    throw StackOverflow();

	_items[_size++] = x;
    }

    Uint32& top()
    {
	if (_size == 0)
	    throw StackUnderflow();

	return _items[_size-1];
    }

    Uint32 top() const
    {
	return ((QueueIdStack*)this)->top();
    }

    void pop()
    {
	if (_size == 0)
	    throw StackUnderflow();

	_size--;
    }

    /** Make a copy of this stack and then pop the top element. */
    QueueIdStack copyAndPop() const
    {
	return QueueIdStack(*this, 0);
    }

private:

    // Copy the given stack but then pop the top element:
    QueueIdStack(const QueueIdStack& x, int) : _size(x._size)
    {
	memcpy(_items, x._items, sizeof(_items));
	pop();
    }

    enum { MAX_SIZE = 5 };
    Uint32 _items[MAX_SIZE];
    Uint32 _size;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Message_h */
