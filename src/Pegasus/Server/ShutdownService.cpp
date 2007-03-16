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
//%////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  ShutdownService
/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Server/ShutdownExceptions.h>
#include <Pegasus/Server/CIMServerState.h>
#include <Pegasus/Server/ShutdownService.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Tracer.h>

#if defined(PEGASUS_OS_TYPE_UNIX)
# include <sys/types.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The constant representing the shutdown timeout property name
*/
static String SHUTDOWN_TIMEOUT_PROPERTY = "shutdownTimeout";

/**
    Initialize ShutdownService instance
*/
ShutdownService* ShutdownService::_instance = 0;

/**
    Initialize all other class variables
*/
CIMServer* ShutdownService::_cimserver = 0;
Uint32 ShutdownService::_shutdownTimeout = 0;
ModuleController* ShutdownService::_controller = 0;

/** Constructor. */
ShutdownService::ShutdownService(CIMServer* cimserver)
{
    _cimserver = cimserver;

    //
    // get module controller
    //
    _controller = ModuleController::getModuleController();
}

/**
    Terminates the shutdown service
*/
void ShutdownService::destroy()
{
    delete _instance;
    _instance = 0;
}

/** Destructor. */
ShutdownService::~ShutdownService()
{
}

/**
   return a pointer to the ShutdownService instance.
*/
ShutdownService* ShutdownService::getInstance(CIMServer* cimserver)
{
    if (!_instance)
    {
        _instance = new ShutdownService(cimserver);
    }
    return _instance;
}

/**
    The shutdown method to be called by the ShutdownProvider to
    process a shutdown request from the cimcli client.
*/
void ShutdownService::shutdown(
    Boolean force,
    Uint32 timeout,
    Boolean requestPending)
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::shutdown");

    //
    // Initialize variables
    //
    Boolean timeoutExpired = false;
    Boolean noMoreRequests = false;

    _shutdownTimeout = timeout;

    try
    {
        //
        // set CIMServer state to TERMINATING
        //
        _cimserver->setState(CIMServerState::TERMINATING);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "ShutdownService::shutdown - CIM server state set to "
                "CIMServerState::TERMINATING");

        //
        // Tell the CIMServer to stop accepting new client connection requests.
        //
        _cimserver->stopClientConnection();


        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "ShutdownService::shutdown - No longer accepting new client "
                "connection requests.");

        //
        // Determine if there are any outstanding CIM operation requests
        // (take into account that one of the request is the shutdown request).
        //
        Uint32 requestCount = _cimserver->getOutstandingRequestCount();
        if (requestCount > (requestPending ? 1 : 0))
        {

            Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
                "ShutdownService::shutdown - Waiting for outstanding CIM "
                    "operations to complete.  Request count: $0",
                requestCount);
            noMoreRequests = waitUntilNoMoreRequests(requestPending);
        }
        else
        {
            noMoreRequests = true;
        }

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "ShutdownService::shutdown - All outstanding CIM operations "
                "complete");

        //
        // proceed to shutdown the CIMServer
        //
        _shutdownCIMServer();
    }
    catch (CIMException& e)
    {
        PEG_TRACE_STRING(TRC_SHUTDOWN, Tracer::LEVEL4,
            "Error occurred during CIMServer shutdown: " + e.getMessage());
    }
    catch (Exception& e)
    {
        PEG_TRACE_STRING(TRC_SHUTDOWN, Tracer::LEVEL4,
            "Error occurred during CIMServer shutdown: " + e.getMessage());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_SHUTDOWN, Tracer::LEVEL4,
            "Unexpected error occurred during CIMServer shutdown. ");
    }

    //
    // All done
    //
    PEG_METHOD_EXIT();
}

/**********************************************************/
/*  private methods                                       */
/**********************************************************/

void ShutdownService::_shutdownCIMServer()
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::_shutdownCIMServer");

    //
    // Shutdown the providers
    //

    _shutdownProviders();

    Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
        "ShutdownService::_shutdownCIMServer - CIM server provider shutdown "
            "complete");

    //
    // Send a shutdown signal to the CIMServer. CIMServer itself will take
    // care of shutting down the CimomServices and deleting them. In other
    // words, _DO_ _NOT_ call 'shutdownCimomServices' from a provider.
    //
    _cimserver->shutdown();

    Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
        "ShutdownService::_shutdownCIMServer - Cimom services shutdown "
            "complete");

    PEG_METHOD_EXIT();
}

void ShutdownService::shutdownCimomServices()
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::shutdownCimomServices");

    //
    // Shutdown the Indication Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_INDICATIONSERVICE);

    // Shutdown the Indication Handler Service
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_INDHANDLERMANAGER);

    // PEGASUS_QUEUENAME_OPRESPENCODER
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_OPRESPENCODER);

    // PEGASUS_QUEUENAME_EXPORTRESPENCODER
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_EXPORTRESPENCODER);

    //
    // shutdown  Authenticator Delegator Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_HTTPAUTHDELEGATOR);

    //
    // shutdown  CIM Operation Request Authorizer Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_OPREQAUTHORIZER);

    //
    // shutdown  CIM Operation Request Decoder Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_OPREQDECODER);


    //
    // shutdown  CIM Export Request Decoder Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_EXPORTREQDECODER);

    //
    // shutdown  CIM Export Request Dispatcher Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_EXPORTREQDISPATCHER);

    //
    // shutdown binary message handler
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_BINARY_HANDLER);

    //
    // shutdown  CIM Operation Request Dispatcher Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_OPREQDISPATCHER);

    // shutdown CIM Provider Manager
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);

    // shutdown ModuleController also called ControlService.
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_CONTROLSERVICE);

    PEG_METHOD_EXIT();
}

void ShutdownService::_sendShutdownRequestToService(const char* serviceName)
{
    MessageQueueService* _mqs = static_cast<MessageQueueService*>(_controller);

    Array<Uint32> _services;
    Uint32 _queueId;

    _mqs->find_services(String(serviceName), 0, 0, &_services);

    if (_services.size() == 0 )
    {
        // service not found, just return
        return;
    }
    _queueId = _services[0];

    // send a Stop (this is a legacy message that in some of the MQS does
    // termination of its internal stuff. Then follow it with a Stop (to
    // open up its incoming queue), and then with a AsyncIoctl::IO_CLOSE
    // which closes the incoming queue.

    // All of these messages MUST be sequential. Do not use SendForget or
    // SendAsync as those are asynchronous and their receipt is guaranteed
    // to be undeterministic and possibly out of sequence (which is something
    // we do not want).

    CimServiceStop stop_message(
        NULL,
        _queueId,
        _controller->getQueueId(),
        true);

    AutoPtr<AsyncReply> StopAsyncReply(
        _controller->ClientSendWait(_queueId, &stop_message));

    CimServiceStart start_message(
        NULL,
        _queueId,
        _controller->getQueueId(),
        true);

    AutoPtr <AsyncReply> StartAsyncReply(
        _controller->ClientSendWait(_queueId, &start_message));

    AsyncIoctl close_request(
        NULL,
        _queueId,
        _controller->getQueueId(),
        false,
        AsyncIoctl::IO_CLOSE,
        0,
        0);

    AutoPtr <AsyncReply> CloseAsyncReply(
        _controller->ClientSendWait(_queueId, &close_request));
}

void ShutdownService::_shutdownProviders()
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::_shutdownProviders");

    //
    // get provider manager service
    //
    MessageQueue* queue =
        MessageQueue::lookup(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);

    if (queue == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

    MessageQueueService* _service = dynamic_cast<MessageQueueService*>(queue);
    PEGASUS_ASSERT(_service != 0);
    Uint32 _queueId = _service->getQueueId();

    //
    // create stop all providers request
    //
    CIMStopAllProvidersRequestMessage* stopRequest =
        new CIMStopAllProvidersRequestMessage(
            XmlWriter::getNextMessageId(),
            QueueIdStack(_queueId));

    //
    // create async request message
    //
    AsyncLegacyOperationStart* asyncRequest =
        new AsyncLegacyOperationStart(
            NULL,
            _queueId,
            stopRequest,
            _queueId);

    // Use SendWait, which is serialized and waits. Do not use asynchronous
    // callback as the response might be received _after_ the provider or
    // this service has been deleted.

    AsyncReply* asyncReply =
        _controller->ClientSendWait(_queueId, asyncRequest);
    CIMStopAllProvidersResponseMessage* response =
       reinterpret_cast<CIMStopAllProvidersResponseMessage*>(
         (static_cast<AsyncLegacyOperationResult*>(asyncReply))->get_result());

    if (response->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            "Server.ShutdownService.CIM_PROVIDER_SHUTDOWN",
            "$0 - CIM provider shutdown exception has occurred.",
            "ShutdownService::_shutdownProviders");

        CIMException e = response->cimException;
        delete asyncRequest;
        delete asyncReply;
        delete response;
        PEG_METHOD_EXIT();
        throw e;
    }

    delete asyncRequest;
    delete asyncReply;
    delete response;

    PEG_METHOD_EXIT();
}

Boolean ShutdownService::waitUntilNoMoreRequests(Boolean requestPending)
{
    Uint32 maxWaitTime = _shutdownTimeout;  // maximum wait time in seconds
    Uint32 waitInterval = 1;                // one second wait interval

    Uint32 requestCount = _cimserver->getOutstandingRequestCount();

    //
    // Loop and wait one second until either there is no more requests
    // or until timeout expires.
    //
    while (requestCount > (requestPending ? 1 : 0) && maxWaitTime > 0)
    {
        Threads::sleep(waitInterval * 1000);
        maxWaitTime -= waitInterval;
        requestCount = _cimserver->getOutstandingRequestCount();
    }

    if (requestCount > 1)
        return false;
    else
        return true;
}

PEGASUS_NAMESPACE_END
