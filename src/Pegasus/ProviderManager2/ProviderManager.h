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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                  (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ProviderManager_h
#define Pegasus_ProviderManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>

#include <Pegasus/ProviderManager2/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

typedef void (*PEGASUS_INDICATION_CALLBACK_T)(
    CIMProcessIndicationRequestMessage*);

typedef void (*PEGASUS_RESPONSE_CHUNK_CALLBACK_T)(
    CIMRequestMessage* request, CIMResponseMessage* response);

class PEGASUS_PPM_LINKAGE ProviderManager
{
public:
    ProviderManager(void);
    virtual ~ProviderManager(void);

    virtual Message * processMessage(Message * message) = 0;

    virtual Boolean hasActiveProviders() = 0;
    virtual void unloadIdleProviders() = 0;
    static String _resolvePhysicalName(String physicalName);

    /**
        Sets the callback function to which indications generated by this
        ProviderManager should be forwarded for processing.
     */
    virtual void setIndicationCallback(
        PEGASUS_INDICATION_CALLBACK_T indicationCallback);

    /**
        Sets the callback function to which response chunks generated by this
        ProviderManager should be forwarded for processing.
     */
    virtual void setResponseChunkCallback(
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual Boolean supportsRemoteNameSpaces() { return false; }

    /**
        Sets the SubscriptionInitComplete flag indicating whether the Indication
        Service has completed its initialization.
     */
    virtual void setSubscriptionInitComplete
        (Boolean subscriptionInitComplete);

protected:
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback;
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;

    /**
        Indicates whether the Indication Service has completed initialization.

        For more information, please see the description of the 
        ProviderManagerRouter::_subscriptionInitComplete member variable.
     */
    Boolean _subscriptionInitComplete;

    // ingredients for CIMOMHandle (binary message handler, repository, etc.)
};

PEGASUS_NAMESPACE_END

#endif
