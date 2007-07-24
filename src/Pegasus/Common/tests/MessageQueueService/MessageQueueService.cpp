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


#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/AsyncQueue.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/Cimom.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static char * verbose;

class TestRequestMessage : public AsyncRequest
{
public:
    typedef AsyncRequest Base;

    TestRequestMessage(
        AsyncOpNode *op,
        Uint32 destination,
        Uint32 response,
        const char *message)
    : Base(
        CIM_DELETE_CLASS_REQUEST_MESSAGE,
        0,
        op,
        destination,
        response,
        true),
      greeting(message)
    {
    }

    virtual ~TestRequestMessage()
    {
    }

    String greeting;
};


class TestResponseMessage : public AsyncReply
{
public:
    typedef AsyncReply Base;

    TestResponseMessage(
        AsyncOpNode *op,
        Uint32 result,
        Uint32 destination,
        const char *message)
    : Base(
        CIM_DELETE_CLASS_RESPONSE_MESSAGE,
        0,
        op,
        result,
        destination,
        true),
      greeting(message)
    {
    }

    virtual ~TestResponseMessage()
    {
    }

    String greeting;
};


class MessageQueueServer : public MessageQueueService
{
public:
    typedef MessageQueueService Base;
    MessageQueueServer(const char *name)
    : Base(
        name, MessageQueue::getNextQueueId(), 0,
            MessageMask::ha_request |
            MessageMask::ha_reply |
            MessageMask::ha_async),
      dienow(0)
    {
    }

    virtual ~MessageQueueServer()
    {
    }

    virtual void _handle_incoming_operation(AsyncOpNode *operation);

    virtual Boolean messageOK(const Message *msg);

    virtual void handleEnqueue()
    {
        // This method is pure abstract in the superclass
        PEGASUS_TEST_ASSERT(0);
    }

    virtual void handleEnqueue(Message* msg)
    {
        // This method is pure abstract in the superclass
        PEGASUS_TEST_ASSERT(0);
    }

    void handleTestRequestMessage(AsyncRequest *msg);
    virtual void handleCimServiceStop(CimServiceStop *req);
    virtual void _handle_async_request(AsyncRequest *req);
    void handleLegacyOpStart(AsyncLegacyOperationStart *req);
    AtomicInt dienow;
};


class MessageQueueClient : public MessageQueueService
{

   public:
      typedef MessageQueueService Base;

      MessageQueueClient(const char *name)
         : Base(name, MessageQueue::getNextQueueId(), 0,
                MessageMask::ha_request |
                MessageMask::ha_reply |
                MessageMask::ha_async),
           client_xid(1)
      {
         _client_capabilities = Base::_capabilities;
         _client_mask = Base::_mask;
      }

      virtual ~MessageQueueClient()
      {
      }

      virtual Boolean messageOK(const Message *msg);

      virtual void handleEnqueue()
      {
          // This method is pure abstract in the superclass
          PEGASUS_TEST_ASSERT(0);
      }

      virtual void handleEnqueue(Message* msg)
      {
          // This method is pure abstract in the superclass
          PEGASUS_TEST_ASSERT(0);
      }

      void sendTestRequestMessage(const char *greeting, Uint32 qid);
      Uint32 get_qid();

      Uint32 _client_capabilities;
      Uint32 _client_mask;

      virtual void _handle_async_request(AsyncRequest *req);
      AtomicInt client_xid;
};

AtomicInt msg_count;
AtomicInt client_count;


Uint32 MessageQueueClient::get_qid()
{
    return _queueId;
}

void MessageQueueServer::_handle_incoming_operation(AsyncOpNode *operation)
{
    if (operation != 0)
    {
        Message* rq = operation->getRequest();

        PEGASUS_TEST_ASSERT(rq != 0);
        if (rq->getMask() & MessageMask::ha_async)
        {
            _handle_async_request(static_cast<AsyncRequest *>(rq));
        }
        else
        {
            if (rq->getType() == CIM_CREATE_CLASS_REQUEST_MESSAGE)
            {
                if (verbose)
                {
                    cout << " caught a hacked legacy message " << endl;
                }
            }
            delete rq;
        }
    }

    return;
}

void MessageQueueServer::_handle_async_request(AsyncRequest *req)
{
    if (req->getType() == CIM_DELETE_CLASS_REQUEST_MESSAGE)
    {
        req->op->processing();
        handleTestRequestMessage(req);
    }
    else if (req->getType() == ASYNC_CIMSERVICE_STOP)
    {
        req->op->processing();
        handleCimServiceStop(static_cast<CimServiceStop *>(req));
    }
    else if (req->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        req->op->processing();
        handleLegacyOpStart(static_cast<AsyncLegacyOperationStart *>(req));
    }
    else
    {
        Base::_handle_async_request(req);
    }
}

Boolean MessageQueueServer::messageOK(const Message *msg)
{
    if (msg->getType() == CIM_DELETE_CLASS_REQUEST_MESSAGE ||
        msg->getType() == ASYNC_CIMSERVICE_STOP ||
        msg->getType() == ASYNC_CIMSERVICE_PAUSE ||
        msg->getType() == ASYNC_ASYNC_LEGACY_OP_START ||
        msg->getType() == ASYNC_CIMSERVICE_RESUME ||
        msg->getType() == CIM_CREATE_CLASS_REQUEST_MESSAGE)
    {
        return true;
    }

    return false;
}

void MessageQueueServer::handleLegacyOpStart(AsyncLegacyOperationStart *req)
{
    Message *legacy = req->get_action();

    if (verbose)
    {
        cout << " ### handling legacy messages " << endl;
    }

    AsyncReply *resp =
        new AsyncReply(
            ASYNC_REPLY,
            0,
            req->op,
            async_results::OK,
            req->resp,
            req->block);
    _completeAsyncResponse(req, resp, ASYNC_OPSTATE_COMPLETE, 0);

    if (verbose)
    {
        if (legacy != 0)
            cout << " legacy msg type: " << legacy->getType() << endl;
    }

    delete legacy;
}


void MessageQueueServer::handleTestRequestMessage(AsyncRequest *msg)
{
    if (msg->getType() == CIM_DELETE_CLASS_REQUEST_MESSAGE)
    {
        TestResponseMessage *resp = new TestResponseMessage(
            msg->op,
            async_results::OK,
            msg->dest,
            "i am a test response");
       _completeAsyncResponse(msg, resp, ASYNC_OPSTATE_COMPLETE, 0);
   }
}

void MessageQueueServer::handleCimServiceStop(CimServiceStop *req)
{
    AsyncReply *resp =
        new AsyncReply(
            ASYNC_REPLY,
            0,
            req->op,
            async_results::CIM_SERVICE_STOPPED,
            req->resp,
            req->block);
    _completeAsyncResponse(req, resp, ASYNC_OPSTATE_COMPLETE, 0);

    if (verbose)
    {
        cout << "recieved STOP from test client" << endl;
    }

    dienow++;
}


void MessageQueueClient::_handle_async_request(AsyncRequest *req)
{
    Base::_handle_async_request(req);
}


Boolean MessageQueueClient::messageOK(const Message *msg)
{
   if(msg->getMask() & MessageMask::ha_async)
   {
      if (msg->getType() == CIM_DELETE_CLASS_RESPONSE_MESSAGE ||
          msg->getType() == ASYNC_CIMSERVICE_STOP ||
          msg->getType() == ASYNC_CIMSERVICE_PAUSE ||
          msg->getType() == ASYNC_CIMSERVICE_RESUME)
      return true;
   }
   return false;
}


void MessageQueueClient::sendTestRequestMessage(
    const char *greeting,
    Uint32 qid)
{
    TestRequestMessage *req =
        new TestRequestMessage(
            0,
            qid,
            _queueId,
            greeting);

    AsyncMessage *response = SendWait(req);
    if (response != 0)
    {
        msg_count++;
        delete response;
        if (verbose)
        {
            cout << " test message " << msg_count.get() << endl;
        }
    }
    delete req;
}


ThreadReturnType PEGASUS_THREAD_CDECL client_func(void *parm);
ThreadReturnType PEGASUS_THREAD_CDECL server_func(void *parm);

int main(int argc, char **argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    try
    {
        Thread client(client_func, (void *)&msg_count, false);
        Thread another(client_func, (void *)&msg_count, false);
        Thread a_third(client_func, (void *)&msg_count, false);

        Thread server(server_func, (void *)&msg_count, false);

        server.run();
        client.run();
        another.run();
        a_third.run();

        while (msg_count.get() < 1500)
        {
            Threads::sleep(10);
        }

        a_third.join();
        another.join();
        client.join();
        server.join();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
    catch (...)
    {
        cout << "Caught unknown exception" << endl;
        exit(1);
    }

    if (verbose)
    {
        cout << "exiting main " << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}


ThreadReturnType PEGASUS_THREAD_CDECL client_func(void *parm)
{
    Thread* my_handle = reinterpret_cast<Thread *>(parm);
    AtomicInt& count = *(reinterpret_cast<AtomicInt *>(my_handle->get_parm()));

    char name_buf[128];

    sprintf(name_buf, "test client %s", Threads::id().buffer);

    MessageQueueClient *q_client = new MessageQueueClient(name_buf);

    client_count++;
    while (client_count.get() < 3)
        Threads::yield();

    Array<Uint32> services;

    while (services.size() == 0)
    {
        q_client->find_services(String("test server"), 0, 0, &services);
        Threads::yield();
    }

    if (verbose)
    {
        cout << "found server at " << services[0] << endl;
    }

    while (msg_count.get() < 1500)
    {
        q_client->sendTestRequestMessage("i am the test client" , services[0]);
    }
    // now that we have sent and received all of our responses, tell
    // the server thread to stop

    AsyncMessage *reply;

    if (verbose)
    {
        cout << " sending LEGACY to test server" << endl;
    }

    Message *legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);

    AsyncLegacyOperationStart *req = new AsyncLegacyOperationStart(
        0,
        services[0],
        legacy,
        q_client->getQueueId());
    reply = q_client->SendWait(req);
    delete req;
    delete reply;

    if (verbose)
    {
        cout << "trying SendForget " << endl;
    }

    legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);

    req = new AsyncLegacyOperationStart(
        0,
        services[0],
        legacy,
        q_client->getQueueId());

    q_client->SendForget(req);

    legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);
    legacy->dest = services[0];

    q_client->SendForget(legacy);

    MessageQueueService * server =
        static_cast<MessageQueueService *>(MessageQueue::lookup(services[0]));

#if 0
    legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);

    // ATTN: handleEnqueue() is not implemented
    server->enqueue(legacy);
#endif

    if (verbose)
    {
        cout << "sending STOP to test server" << endl;
    }

    CimServiceStop *stop = new CimServiceStop(
        0,
        services[0],
        q_client->get_qid(),
        true);

    reply = q_client->SendWait(stop);
    delete stop;
    delete reply;

    if (verbose)
    {
        cout << "deregistering client qid " << q_client->getQueueId() << endl;
    }

    q_client->deregister_service();

    if (verbose)
    {
        cout << " deleting client " << endl ;
    }

    delete q_client;

    if (verbose)
    {
        cout << " exiting " << endl;
    }

    my_handle->exit_self((ThreadReturnType) 1);
    return(0);
}


ThreadReturnType PEGASUS_THREAD_CDECL server_func(void *parm)
{
    Thread *my_handle = reinterpret_cast<Thread *>(parm);

    MessageQueueServer *q_server = new MessageQueueServer("test server") ;

    while (q_server->dienow.get()  < 3)
    {
        Threads::yield();
    }

    if (verbose)
    {
        cout << "deregistering server qid " << q_server->getQueueId() << endl;
    }

    q_server->deregister_service();

    if (verbose)
    {
        cout << " deleting server " << endl;
    }

    delete q_server;

    if (verbose)
    {
        cout << "exiting server " << endl;
    }

    my_handle->exit_self((ThreadReturnType) 1);
    return(0);
}
