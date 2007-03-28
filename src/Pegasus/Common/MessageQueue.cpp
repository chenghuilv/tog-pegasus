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

#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CimomMessage.h>
#include "MessageQueue.h"
#include "IDFactory.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

typedef HashTable<Uint32, MessageQueue*, EqualFunc<Uint32>, HashFunc<Uint32> >
    QueueTable;

static QueueTable _queueTable(256);
static Mutex q_table_mut ;

static IDFactory _qidFactory(CIMOM_Q_ID + 1);

Uint32 MessageQueue::getNextQueueId()
{
    return _qidFactory.getID();
}

void MessageQueue::putQueueId(Uint32 queueId)
{
    if (queueId != CIMOM_Q_ID)
        _qidFactory.putID(queueId);
}

MessageQueue::MessageQueue(
    const char* name,
    Boolean async,
    Uint32 queueId)
   : _queueId(queueId), _capabilities(0), _async(async)
{
    //
    // Copy the name:
    //

    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,"MessageQueue::MessageQueue()");

    if (!name)
        name = "";

    _name = new char[strlen(name) + 1];
    strcpy(_name, name);

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
        "MessageQueue::MessageQueue  name = %s, queueId = %u", name, queueId));

    //
    // Insert into queue table:
    //
    AutoMutex autoMut(q_table_mut);
    while (!_queueTable.insert(_queueId, this))
        ;

    PEG_METHOD_EXIT();
}

MessageQueue::~MessageQueue()
{
    // ATTN-A: thread safety!
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,"MessageQueue::~MessageQueue()");
    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
        "MessageQueue::~MessageQueue queueId = %i, name = %s",
        _queueId,
        _name));

    {
        AutoMutex autoMut(q_table_mut);
        _queueTable.remove(_queueId);
    } // mutex unlocks here

    // Free the name:

    delete [] _name;

    // Return the queue id.

    putQueueId(_queueId);

    PEG_METHOD_EXIT();
}

void MessageQueue::enqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,"MessageQueue::enqueue()");

    PEGASUS_ASSERT(message != 0);

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
        "Queue name: %s",
        getQueueName()));

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
        "Message: [%s]",
        MessageTypeToString(message->getType())));

    _messageList.insert_back(message);

    handleEnqueue();
    PEG_METHOD_EXIT();
}

Message* MessageQueue::dequeue()
{
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,"MessageQueue::dequeue()");

    Message* message = _messageList.remove_front();

    PEG_METHOD_EXIT();
    return message;
}

const char* MessageQueue::getQueueName() const
{
    return _name;
}

MessageQueue* MessageQueue::lookup(Uint32 queueId)
{

    MessageQueue* queue = 0;
    AutoMutex autoMut(q_table_mut);

    if (_queueTable.lookup(queueId, queue))
    {
        return queue;
    }

    // Not found!

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
        "MessageQueue::lookup failure queueId = %u", queueId));

    return 0;
}


MessageQueue* MessageQueue::lookup(const char *name)
{

    if (name == NULL)
        throw NullPointer();

    AutoMutex autoMut(q_table_mut);
    for (QueueTable::Iterator i = _queueTable.start(); i; i++)
    {
        // ATTN: Need to decide how many characters to compare in queue names
        if (!strcmp(((MessageQueue *)i.value())->getQueueName(), name))
        {
            return (MessageQueue *)i.value();
        }
    }

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
        "MessageQueue::lookup failure - name = %s", name));

    return 0;
}


void MessageQueue::handleEnqueue()
{

}

PEGASUS_NAMESPACE_END
