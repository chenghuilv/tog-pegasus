//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 The Open group, BMC Software, Tivoli Systems, IBM
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
// Author: Markus Mueller (sedgewick_de@yahoo.de)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

//
// This code doesn't work under Windows !!!
// 

#include <Pegasus/Common/IPC.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/MessageQueue.h>
#include "MT_MessageQueue.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

typedef HashTable<Uint32, MessageQueue*, EqualFunc<Uint32>, HashFunc<Uint32> >
    QueueTable;

static Mutex _MT_queueTable_lock;
static QueueTable _MT_queueTable(128);

MT_MessageQueue::MT_MessageQueue()
{
    _MT_queueTable_lock.lock(pegasus_thread_self());

    // has to be rewritten to use the lockable QueueTable !!!
    _mqueue = new MessageQueue();

    // the message queue MAY only be changed with lock _sharedMutex held

    _sharedMutex = new Mutex();
#if PEGASUS_OS_UNIX
    _full = new Condition(*_sharedMutex);
    _empty= new Condition(*_sharedMutex);
#else
    _full = new Condition();
    _empty= new Condition();
#endif

    _lowWaterMark = PEGASUS_DEFAULT_LOWWATERMARK;
    _highWaterMark = PEGASUS_DEFAULT_HIGHWATERMARK;

    _preferReader = false; //default

    _MT_queueTable_lock.unlock();
}

MT_MessageQueue::~MT_MessageQueue()
{
    _MT_queueTable_lock.lock(pegasus_thread_self());

    // has to be rewritten to use the lockable QueueTable
    delete _full;
    delete _empty;
    delete _sharedMutex;
    delete _mqueue;

    _MT_queueTable_lock.unlock();
}

void MT_MessageQueue::enqueue(Message* message)
{
    Uint32 count;

    if (!message)
	throw NullPointer();

    _full->lock_object(pegasus_thread_self());
 
    while(_mqueue->getCount() >= _highWaterMark) 
    {
        try
        {
            _writers++;
            _full->unlocked_timed_wait(500, pegasus_thread_self());
            _writers--;
        }
        catch(TimeOut& to) { ; }
    }

    _mqueue->enqueue(message);

    if (_readers > 0)
        _empty->unlocked_signal(pegasus_thread_self());

    _full->unlock_object();

    handleEnqueue();
}

Message* MT_MessageQueue::dequeue()
{
    Message * message; 

    _empty->lock_object(pegasus_thread_self());
 
    while (_mqueue->getCount() <= _lowWaterMark) 
    {
        try
        {
            _readers++;
            _empty->unlocked_timed_wait(500, pegasus_thread_self());
            _readers--;
        }
        catch(TimeOut& to) { ; }
    }

    message = _mqueue->dequeue();

    if (_writers > 0)
        _full->unlocked_signal(pegasus_thread_self());

    _empty->unlock_object();

    return message;
}

void MT_MessageQueue::remove(Message* message)
{
}

Message* MT_MessageQueue::findByType(Uint32 type)
{
    return NULL;
}

Message* MT_MessageQueue::findByKey(Uint32 key)
{
    return NULL;
}

void MT_MessageQueue::print(ostream& os) const
{
}

Message* MT_MessageQueue::find(Uint32 type, Uint32 key)
{
    return NULL;
}

void MT_MessageQueue::lock()
{
    _full->lock_object(pegasus_thread_self());
}

void MT_MessageQueue::unlock()
{
    _full->unlock_object();
}

MessageQueue* MT_MessageQueue::lookup(Uint32 queueId)
{
    MessageQueue* queue = 0;

    _MT_queueTable_lock.lock(pegasus_thread_self());
    if (_MT_queueTable.lookup(queueId, queue))
    {
        _MT_queueTable_lock.unlock();
	return queue;
    }

    // Not found!
    _MT_queueTable_lock.unlock();
    return 0;
}

void MT_MessageQueue::handleEnqueue()
{
    _mqueue->handleEnqueue();
}

PEGASUS_NAMESPACE_END
