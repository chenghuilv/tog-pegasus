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

#ifndef Pegasus_Server_h
#define Pegasus_Server_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Cimom.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/ProviderManager2/Default/ProviderMessageHandler.h>
#include <Pegasus/Server/CIMServerState.h>
#include <Pegasus/Server/HTTPAuthenticatorDelegator.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

struct ServerRep;

class Monitor;
class CIMOperationRequestDispatcher;
class CIMOperationResponseEncoder;
class CIMOperationRequestDecoder;
class CIMOperationRequestAuthorizer;
class CIMExportRequestDispatcher;
class CIMExportResponseEncoder;
class CIMExportRequestDecoder;
class HTTPAcceptor;
class CIMRepository;

class ModuleController;
class IndicationHandlerService;
class IndicationService;
class ProviderManagerService;
class ProviderRegistrationManager;
class BinaryMessageHandler;
class SSLContextManager;


class PEGASUS_SERVER_LINKAGE CIMServer
{
public:

    enum Protocol { PROPRIETARY, STANDARD };

    /** Constructor - Creates a CIMServer object.
        The CIM Server objects establishes a repository object,
        a dispatcher object, and creates a channnel factory and
        acceptor for the Server.
        @param monitor      monitor object for the server.
        @exception - ATTN
    */
    CIMServer(Monitor* monitor);

#if defined PEGASUS_OS_TYPE_WINDOWS && !defined(PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET)
    CIMServer(Monitor* monitor, Monitor* monitorPipe);
#endif
    ~CIMServer();

    /** Adds a connection acceptor for the specified listen socket.
        @param localConnection Boolean specifying whether the acceptor should
               listen on a local-system-only connection.
        @param portNumber Port number that should be used by the listener.
               This parameter is ignored if localConnection=true.
        @param useSSL Boolean specifying whether SSL should be used for
               connections created by this acceptor.
    */
    void addAcceptor(
        Boolean localConnection,
        Uint32 portNumber,
        Boolean useSSL);

    /** Bind the acceptors to the specified listen sockets.
    @exception - This function may receive exceptions from
    Channel specific subfunctions.
    */
    void bind();

    void tickle_monitor();

    /** runForever Main runloop for the server.
    */
    void runForever();

    /** Call to gracefully shutdown the server.  The server connection socket
        will be closed to disable new connections from clients.
    */
    void stopClientConnection();

    /** Call to gracefully shutdown the server.  It is called when the server
        has been stopped and is ready to be shutdown.  Next time runForever()
        is called, the server shuts down.
    */
    void shutdown();

    /** Returns true if the server is in the process of shutting down,
        false otherwise.
    */
    Boolean terminated() { return _dieNow; };

    /** Call to resume the sever.
    */
    void resume();

    /** Call to set the CIMServer state.  Also inform the appropriate
        message queues about the current state of the CIMServer.
    */
    void setState(Uint32 state);

    Uint32 getOutstandingRequestCount();

    /** Signal to shutdown
    */
    static void shutdownSignal();

    /** startSLPProvider is a temporary function to get this
        provider started as part of system startup.
        This MUST be replaced with something more permanent.
    */
    void startSLPProvider();

private:
    Boolean _dieNow;

#ifdef PEGASUS_ENABLE_SLP
    Boolean _runSLP;
#endif

    Monitor* _monitor;
#if defined PEGASUS_OS_TYPE_WINDOWS && !defined(PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET)
    Monitor* _monitor_pipe;
    DWORD waitRes ;
    int pCount;
    HANDLE    thrdSocketHandle ;
    HANDLE    thrdPipeHandle ;
#endif
    CIMRepository* _repository;
    CIMOperationRequestDispatcher* _cimOperationRequestDispatcher;
    CIMOperationResponseEncoder* _cimOperationResponseEncoder;
    CIMOperationRequestDecoder* _cimOperationRequestDecoder;
    CIMOperationRequestAuthorizer* _cimOperationRequestAuthorizer;

    CIMExportRequestDispatcher* _cimExportRequestDispatcher;
    CIMExportResponseEncoder* _cimExportResponseEncoder;
    CIMExportRequestDecoder* _cimExportRequestDecoder;
    HTTPAuthenticatorDelegator* _httpAuthenticatorDelegator;

    Array<HTTPAcceptor*> _acceptors;
    Array<ProviderMessageHandler*> _controlProviders;
    AutoPtr<CIMServerState> _serverState; //PEP101

    ModuleController* _controlService;
    IndicationHandlerService* _handlerService;
    IndicationService* _indicationService;
    ProviderManagerService* _providerManager;
    ProviderRegistrationManager* _providerRegistrationManager;
    BinaryMessageHandler *_binaryMessageHandler;
    SSLContextManager* _sslContextMgr;

    void _init(void);
    SSLContext* _getSSLContext();
};
#if defined PEGASUS_OS_TYPE_WINDOWS && !defined(PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET)
int runPipe(Monitor* ptrMonitor);
#endif

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Server_h */
