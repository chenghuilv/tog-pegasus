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

#ifndef Pegasus_ProviderMessageHandler_h
#define Pegasus_ProviderMessageHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/ProviderManager2/OperationResponseHandler.h>
#include <Pegasus/ProviderManager2/Default/ProviderStatus.h>
#include <Pegasus/ProviderManager2/Default/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_DEFPM_LINKAGE ProviderMessageHandler
{
public:
    ProviderMessageHandler(
        const String& name,
        CIMProvider* provider,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        Boolean subscriptionInitComplete);

    virtual ~ProviderMessageHandler();

    String getName() const;
    void setProvider(CIMProvider* provider);
    void initialize(CIMOMHandle& cimom);
    void terminate();
    void subscriptionInitComplete();

    CIMResponseMessage* processMessage(CIMRequestMessage* request);

private:
    CIMResponseMessage* _handleGetInstanceRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleEnumerateInstancesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleEnumerateInstanceNamesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleCreateInstanceRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleModifyInstanceRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleDeleteInstanceRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleExecQueryRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleAssociatorsRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleAssociatorNamesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleReferencesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleReferenceNamesRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleGetPropertyRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleSetPropertyRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleInvokeMethodRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleCreateSubscriptionRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleModifySubscriptionRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleDeleteSubscriptionRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleExportIndicationRequest(
        CIMRequestMessage* message);

    /**
        Calls the provider's enableIndications() method.
        If successful, the indications response handler is stored in
        _indicationResponseHandler.

        Note that since an exception thrown by the provider's
        enableIndications() method is considered a provider error, any such
        exception is logged and not propagated by this method.
     */
    void _enableIndications();

    void _disableIndications();

    String _name;
    CIMProvider* _provider;
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback;
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;
    Boolean _subscriptionInitComplete;
    EnableIndicationsResponseHandler* _indicationResponseHandler;

public:
    ProviderStatus status;
};


/**
    Encapsulates the calling of operationBegin() and operationEnd() for a
    ProviderMessageHandler to help ensure an accurate count of provider
    operations.
*/
class PEGASUS_DEFPM_LINKAGE ProviderOperationCounter
{
public:
    ProviderOperationCounter(ProviderMessageHandler* p)
        : _provider(p)
    {
        PEGASUS_ASSERT(_provider != 0);
        _provider->status.operationBegin();
    }

    ProviderOperationCounter(const ProviderOperationCounter& p)
        : _provider(p._provider)
    {
        PEGASUS_ASSERT(_provider != 0);
        _provider->status.operationBegin();
    }

    ~ProviderOperationCounter()
    {
        _provider->status.operationEnd();
    }

    ProviderMessageHandler& GetProvider()
    {
        return *_provider;
    }

private:
    ProviderOperationCounter();
    ProviderOperationCounter& operator=(const ProviderOperationCounter&);

    ProviderMessageHandler* _provider;
};

PEGASUS_NAMESPACE_END

#endif
