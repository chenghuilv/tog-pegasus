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
//=============================================================================
//
//%////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AsyncQueue.h>  
#include <Pegasus/Common/Thread.h>
#include <iostream>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

PEGASUS_NAMESPACE_BEGIN

const Uint32 ITERATIONS = 100000;
Boolean verbose = false;

struct Message : public Linkable
{
    Message(Uint32 x_) : x(x_) { }
    Uint32 x;
};

typedef AsyncQueue<Message> Queue;

static ThreadReturnType PEGASUS_THREAD_CDECL _reader(void* self_)
{
    Thread* self = (Thread*)self_;
    Queue* queue = (Queue*)self->get_parm();

    for (Uint32 i = 0; i < ITERATIONS; i++)
    {
    Message* message = queue->dequeue_wait();
    PEGASUS_TEST_ASSERT(message);

    if (verbose)
    {
        if (((i + 1) % 1000) == 0)
        printf("iterations: %05u\n", message->x);
    }
// special dish of the day for Sun Solaris
// reports say that running as root causes
// the thread not being scheduled-out
// until this is resolved the yield()
// will stay here just for Solaris
#ifdef PEGASUS_OS_SOLARIS
    Threads::yield();
#endif
    }

    self->exit_self((ThreadReturnType)1);
    return(0);
}

static ThreadReturnType PEGASUS_THREAD_CDECL _writer(void* self_)
{
    Thread* self = (Thread*)self_;
    Queue* queue = (Queue*)self->get_parm();

    for (Uint32 i = 0; i < ITERATIONS; i++)
    {
        queue->enqueue_wait(new Message(i));
// special dish of the day for Sun Solaris
// reports say that running as root causes
// the thread not being scheduled-out
// until this is resolved the yield()
// will stay here just for Solaris
#ifdef PEGASUS_OS_SOLARIS
        Threads::yield();
#endif
    }

    self->exit_self((ThreadReturnType)1);
    return(0);
}

void testAsyncQueue()
{
    AsyncQueue<Message>* queue = new AsyncQueue<Message>(100);

    Thread reader(_reader, queue, false);
    Thread writer(_writer, queue, false);
    reader.run();
    writer.run();

    reader.join();
    writer.join();

    delete queue;
}

PEGASUS_NAMESPACE_END

int main(int argc, char **argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;
    try
    {
        testAsyncQueue();
    }
    catch (Exception& e)
    {
        cerr << argv[0] << " Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
