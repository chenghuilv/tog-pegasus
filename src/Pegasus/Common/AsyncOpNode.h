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

#ifndef Pegasus_AsyncOpNode_h
#define Pegasus_AsyncOpNode_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Linkable.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/Thread.h>

PEGASUS_NAMESPACE_BEGIN

#define ASYNC_OPFLAGS_UNKNOWN           0x00000000
#define ASYNC_OPFLAGS_NORMAL            0x00000000
#define ASYNC_OPFLAGS_SINGLE            0x00000008
#define ASYNC_OPFLAGS_META_DISPATCHER   0x00000040
#define ASYNC_OPFLAGS_FIRE_AND_FORGET   0x00000080
#define ASYNC_OPFLAGS_SIMPLE_STATUS     0x00000100
#define ASYNC_OPFLAGS_CALLBACK          0x00000200
#define ASYNC_OPFLAGS_PSEUDO_CALLBACK   0x00000400
#define ASYNC_OPFLAGS_SAFE_CALLBACK     0x00000800

#define ASYNC_OPSTATE_UNKNOWN           0x00000000
#define ASYNC_OPSTATE_PROCESSING        0x00000008
#define ASYNC_OPSTATE_COMPLETE          0x00000040
#define ASYNC_OPSTATE_RELEASED          0x00002000

class PEGASUS_COMMON_LINKAGE AsyncOpNode : public Linkable
{
public:

    AsyncOpNode();
    ~AsyncOpNode();

    void setRequest(Message* request);
    Message* getRequest();
    Message* removeRequest();

    void setResponse(Message* response);
    Message* getResponse();
    Message* removeResponse();

    Uint32 getState();

    void lock();
    void unlock();

    void processing();
    void complete();
    void release();

private:
    AsyncOpNode(const AsyncOpNode&);
    AsyncOpNode& operator=(const AsyncOpNode&);

    Semaphore _client_sem;
    Mutex _mut;
    AutoPtr<Message> _request;
    AutoPtr<Message> _response;

    Uint32 _state;
    Uint32 _flags;
    Uint32 _completion_code;
    MessageQueue *_op_dest;

    void (*_async_callback)(AsyncOpNode *, MessageQueue *, void *);
    void (*__async_callback)(Message *, void *, void *);

    // pointers for async callbacks  - don't use
    AsyncOpNode *_callback_node;
    MessageQueue *_callback_request_q;
    MessageQueue *_callback_response_q;
    void *_callback_ptr;
    void *_callback_parameter;
    void *_callback_handle;

    // pointers to help static class message handlers - don't use
    MessageQueue *_service_ptr;
    Thread *_thread_ptr;

    friend class cimom;
    friend class MessageQueueService;
    friend class ProviderManagerService;
    friend class BinaryMessageHandler;
};


inline void AsyncOpNode::setRequest(Message* request)
{
    AutoMutex autoMut(_mut);
    PEGASUS_ASSERT(_request.get() == 0);
    PEGASUS_ASSERT(request != 0);
    _request.reset(request);
}

inline Message* AsyncOpNode::getRequest()
{
    AutoMutex autoMut(_mut);
    PEGASUS_ASSERT(_request.get() != 0);
    return _request.get();
}

inline Message* AsyncOpNode::removeRequest()
{
    AutoMutex autoMut(_mut);
    PEGASUS_ASSERT(_request.get() != 0);
    Message* request = _request.get();
    _request.release();
    return request;
}

inline void AsyncOpNode::setResponse(Message* response)
{
    AutoMutex autoMut(_mut);
    PEGASUS_ASSERT(_response.get() == 0);
    PEGASUS_ASSERT(response != 0);
    _response.reset(response);
}

inline Message* AsyncOpNode::getResponse()
{
    AutoMutex autoMut(_mut);
    PEGASUS_ASSERT(_response.get() != 0);
    return _response.get();
}

inline Message* AsyncOpNode::removeResponse()
{
    AutoMutex autoMut(_mut);
    PEGASUS_ASSERT(_response.get() != 0);
    Message* response = _response.get();
    _response.release();
    return response;
}

inline Uint32 AsyncOpNode::getState()
{
    AutoMutex autoMut(_mut);
    return _state;
}

inline void AsyncOpNode::lock()
{
    _mut.lock();
}

inline void AsyncOpNode::unlock()
{
    _mut.unlock();
}

inline void AsyncOpNode::processing()
{
    AutoMutex autoMut(_mut);
    _state |= ASYNC_OPSTATE_PROCESSING;
}

inline void AsyncOpNode::complete()
{
    AutoMutex autoMut(_mut);
    _state |= ASYNC_OPSTATE_COMPLETE;
}

inline void AsyncOpNode::release()
{
    AutoMutex autoMut(_mut);
    _state |= ASYNC_OPSTATE_RELEASED;
}

PEGASUS_NAMESPACE_END

#endif //Pegasus_AsyncOpNode_h
