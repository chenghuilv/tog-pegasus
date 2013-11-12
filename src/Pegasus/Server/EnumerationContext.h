//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef PegasusEnumerationContext_h
#define PegasusEnumerationContext_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/NumericArg.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Magic.h>
#include <Pegasus/Common/Condition.h>
#include <Pegasus/General/Stopwatch.h>

PEGASUS_NAMESPACE_BEGIN

// Conditional compiles for EnumerationContext and table Classes
// If the following define is set, the enumeration context interoperation
// timeout is tested with a separate thread.  If not, it is simply tested
// with each new operation context creation eliminating the extra thread.
// This means, however, that a context that times out will not be cleaned
// up until another operation sequence begins and another context created

//#define PEGASUS_USE_PULL_TIMEOUT_THREAD

// End of conditional compile variables for EnumerationContext and Table
// Classes

#define LOCAL_MIN(a, b) ((a < b) ? a : b)

/******************************************************************************
**
**    Class that caches each outstanding enumeration sequence. Contains
**    the parameters and current status of existing enumerations
**    that the server is processing.  Enumerations are those
**    sequences of operations starting with an Open... operation
**    and proceeding through Pull... and possible close Enumeration
**    operations.  The enumerationContext is the glue information
**    that ties the sequence of operations together.  This struct
**    defines the information that is maintained througout the
**    life of the sequence.
**    This structure also contains the queue of CIMOperationData objects
**    that is fed from provider returns and accessed by the operation requests
**
******************************************************************************/

/*
    Keep total and average statistics on a 32bit integer. Used by
    enumeration to keep statistics on info in the enumeration.
*/
class PEGASUS_SERVER_LINKAGE uint32Stats
{
public:
    uint32Stats();
    void reset();
    // Add an entry to the statistics
    void add(Uint32 newInfo);
    // get the various pieces of intformation.
    Uint32 getHighWaterMark();
    Uint32 getAverage();
    Uint32 getCounter();
private:
    Uint32 _highWaterMark;
    Uint32 _counter;
    Uint32 _average;
    Uint64 _total;
    bool _overflow;
};

/*
    Controls the EnumerationContext for pull operations.  An instance
    of this class is the controller for the sequence of operations representing
    a pull sequence from open to completion. This instance provides the
    tools for maintaining interoperation information including the cache
    of objects to be returned, request information that is used by pull and
    close operations (timers, open request parameters, etc.), and the state
    of the pull operation sequence.
    nameSpace - Namespace in which this context defined. Used to confirm
    that pulls and closes are operating on the same namespace.
    operationTimeoutSec - Number of seconds for the interoperation
    timeout for this pull sequence.  Set with the open and
    used by the startTimer to set the timeout value between operations
    pOperationAggregate - pointer to the operation aggregate
    that this enumeration controls
    interoperationTimeout - microsecond timer that defines
    the next timeout.  If zero,there is no timeout in process
    magicNumber - Diagnostic to be sure that the structure
    is valid.
    processing - If true, a request operation is active on this context.
    Any pull or close operation request will be refused.
*/

class EnumerationContextTable;

class PEGASUS_SERVER_LINKAGE EnumerationContext
{
public:
    // KS_TODO want this to be protected and available only to its friend
    // class.  We do not want others to create contexts.
    EnumerationContext(const CIMNamespaceName& nameSpace,
        Uint32 interOperationTimeoutValue,
        Boolean continueOnError_,
        MessageType originatingOpenRequestType,
        CIMResponseData::ResponseDataContent contentType);

    ~EnumerationContext();

    /**
       Get the name of this enumeration context. The Name is the key
       to access the context entry in the enumeration context table.
       @return Context name String.
     */
    String& getName();

    /**
       Get the Type of the CIMResponseData object in the enumeration
       context.
       @param return Type
    */
    CIMResponseData::ResponseDataContent getCIMResponseDataType();

    /**
        Set the request properties that must be moved to subsequent
        pull operations.
    */
    void setRequestProperties(
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    // Start the interOperation timer for this context
    void startTimer();

    // Stop the interOperation timer for this context
    void stopTimer();

    /**
        Test if this context timed out given the current time
        @param currentTime
        @return true if interoperation timer timed out
    */
    Boolean isTimedOut(Uint64 currentTime);
    /**
        Test if this context timed out. Gets current time and tests
        against the timeout in the enumeration context entry

        @return true if interoperation timer timed out
    */
    Boolean isTimedOut();

    // diagnostic tests magic number in context to see if valid
    // Diagnostic tool only.
    Boolean valid() const;

    Boolean isClosed();

    Boolean isErrorState();

    /**Set the error state flag and set the current cimException
       into the context object.

        @param cimException
     */
    void setErrorState(CIMException cimException);

    /**
        Test the Message type for the open message saved in the
        context against the type parameter.  This provides a test
        that insures that Pull message types match the open type.
        Ex. PullPaths can only be used with OpenPath contexts
        @param type MessageType for (for pull operation)
        @return Returns true if type match is correct. Returns false
        if not correct type.
     */
    Boolean isValidPullRequestType(MessageType type) const;

    /** Test context to determine if it is active (i.e an operation
        is in process in the CIMOperationRequestDispatcher.
    */
    Boolean isProcessing();

    // Diagnostic to display the current context into the
    // trace file  KS_TODO eliminate this diagnostic
    void trace();

    /**Put the CIMResponseData from the response message into the
       enumerationContext cache and if providersComplete is true,
       set the enumeration state to providersComplete. This function
       signals the getCache function conditional variable. This
       function may also wait if the cache is full.
       @return true if data set into cache, false if enumeration
               closed and data not inserted into cache.
     */
    Boolean putCache(MessageType type,
                  CIMResponseMessage*& response,
                  Boolean providersComplete);

    /**Get up to the number of objects defined by count from the
       CIMResponseData cache in the enumerationContext into the rtn
       CIMResponseData object. This function waits for a number of
       possible events as defined below and returns only when one of
       these events is true.
       This function also executes a ProviderLimitCondition signal
       before returning to tell the ProviderLimit condition variable
       that the size of the cache may have changed.

       NOTE: This function gives up control while waiting.

       Wait events:
           a. Number of objects in cache matches or exceeds count
           b. _providersComplete flag to be set.
           c. Future - The errorState to be set KS_TODO
       @param count Uint32 count of max number of objects to return
       @param rtnData CIMResponseData containing the
                                 objects returned
     */
    void getCache(Uint32 count, CIMResponseData& rtnData);

    /**
        Return reference to the CIMResponseData in the Enumeration
        Context. This CIMResponseData object holds aggregated
        CIMResponseData for this enumeration context. Data is added
        with putCache and removed with getCache
        @return reference to the CIMResponseData object that is the
                cache
    */
    CIMResponseData& getResponseData();

    /**
        Returns count of objects in the EnumerationContext CIMResponseData
        cache.
        @return  Uint32 count of objects currently in the cache
    */
    Uint32 responseCacheSize();

     /**
        Set the ProvidersComplete state.  This should be set from provider
        responses when all responses processed.
    */
    void setProvidersComplete();

    /**
        Closed the client side of the EnumerationContext. From this
        point on, any client side requests should be rejected. Note
        that the providers may still be delivering CIMResponseData
        to the enumerationContextQueue. The
        CIMOperationRequestDispatcher uses the closed state to
        refuse pull/close operations. Once the EnumerationContext is
        closed, it may be removed from the enumeration context table
        (normally this happens when closed and providersComplete are
        set).
    */
    void setClientClosed();

    /**
        Sets the active state (i.e. Request being processed).
        Setting processing = true stops the interOperation timer.
        Otherwise the interoperation timer is started
        @param state Boolean defines whether to set processing or
        !processing. Processing means request being processed.
        @return - NOT USED TODAY

     */
    Boolean setProcessingState(Boolean state);

    /**
        Test if the provider processing is complete.
        @return true if provider processing is complete.
     */
    Boolean ifProvidersComplete() const ;

    /**
        Called by the Dispatcher Client operation when the
        processing of a Request is complete, this function
        determines sets the next state for the operation,
        either back to wait for the next operation or complete.
         @param errorFound Boolean indicating that an error was
         encountered which, if continueOnError = false, forces
         the operation to be closed and the true response returned.
        @return Boolean true if the enumeration is complete.
    */
    Boolean setNextEnumerationState(Boolean errorFound);

    /**
        Increment the count of the number of pull operations executed
        for this context. This method also controls the counting
        of operations with zero length through the input parameter.
        The zero length counter is reset for each call with the
        input parameter != zero so that this function counts total
        operations and also counts consecutive maxObjectCount zero
        length requests.

        @param isZeroLength Boolean indicating if this operation is a request
        for zero objects which is used to count consecutive zero length
        pull operations.
        @return true if the count of consecutive zero length pull operations
        exceeds a predefined maximum.
    */
    Boolean incAndTestPullCounters(Boolean isZeroLength);

    // Exception placed here in case of error. This is set by the operation
    // aggregate with any CIMException recieved from providers.  Note that
    // Only one is allowed.
    // NO  multiple errors until we get continueOnError
    // or really get way to return multiple  errors.
    CIMException _cimException;

    CIMNamespaceName getNamespace() const;

private:
    // Default constructor not used
    EnumerationContext();
    // hide assignment and copy constructor
    EnumerationContext(const EnumerationContext& x);
    EnumerationContext& operator=(const EnumerationContext&);

    friend class EnumerationContextTable;

    // Name of this EnumerationContext. Used to find the context by
    // pull and close operations.
    String _enumerationContextName;

    // Namespace for this pull sequence.  Set by open and used by
    // pull and close.
    CIMNamespaceName _nameSpace;

    // Interoperation timeout value in seconds.  From operation request
    // parameters.
    Uint32 _operationTimeoutSec;

    // ContinueOnError request flag.Set by open...
    Boolean _continueOnError;

    // Timeout absolute time value in seconds for interoperation timeout.
    // 0 indicates  timer not active. Contains timeout time for
    // this sequence in microseconds.
    Uint64 _interOperationTimer;

    // Request Type for pull operations for this pull sequence.
    // Set by open and all pulls must match this type.
    MessageType _pullRequestType;

    // status flags.
    // Set true when context closed from client side
    Boolean _clientClosed;

    // Set to true when input from providers complete
    Boolean _providersComplete;

    // set true when CIMServer is processing a request within the
    // enumeration context
    Boolean _processing;

    // Set true when error received from Providers.
    Boolean _error;

    // Timers for the wait conditions. Counts time waiting
    Stopwatch _waitingCacheSizeConditionTime;
    Stopwatch _waitingProviderLimitConditionTime;

    // Block simultaneous access to certain functions in the Enumeration
    // context.
    Mutex _cacheBlock;

    // Object cache for this context.  All pull responses feed their
    // CIMResponseData into this cache using putCache(..) and all
    // Open and Pull responses get data from the cache using getCache()
    // Simultaneous access to the cache is controlled with _cacheBlock mutex.
    Mutex _responseCacheMutex;
    CIMResponseData _responseCache;

    // Condition variable and mutex for the  cache size tests. This condition
    // variable is used by getCache(..) to force a wait until specific
    // conditions have been met by the EnumerationContext.
    Condition _cacheTestCondition;
    Mutex _cacheTestCondMutex;
    Uint32 _conditionCounter;
    /**
        Tests the cache to determine if we are ready to send a response.
        The test is two parts, a) enough objects (i.e. GE size input parameter)
        or end-of-sequence set indicating that we have completed provider
        processing.
    */
    void waitCacheSizeCondition(Uint32 size);

    /**
        Signal that the cache size condition may have been met. Normally
        called for every putcache and when providers complete set.
        Uses the CacheTestCondition Condition Variable and used inc
        conjunction with waitCacheSizeCondition
    */
    void signalCacheSizeCondition();

    // Condition variable and mutex for the provider wait
    // condition.  This is a hold on returns from putcache when cache
    // reaches a defined limit that is cleared when the cache level drops
    // to below the defined level (See functions waitProviderLimitCondition and
    // signalProviderLimitCondition)
    Condition _providerLimitCondition;
    Mutex _providerLimitConditionMutex;

    // Functions for using providerLimitCondition.  Wait executes a wait
    // until the conditions of the condition variable are met.
    // @param size defines the limit at which wait occurs.
    void waitProviderLimitCondition(Uint32 limit);

    // signalProviderLimitCondition signals the condition variable that
    // it should test the _providerLimitCondition wait conditions.
    void signalProviderLimitCondition();

    // Count Of pull operations executed in this context.  Used for statistics
    Uint32 _pullOperationCounter;

    // Counter of consecutive requests with maxObjectCount == 0. Used to limit
    // client stalling by executing excessive number of zero return pull
    // operations.
    Uint32 _consecutiveZeroLenMaxObjectRequestCounter;

    // Maximum number of objects that can be placed in the response Cache
    // before blocking providers.
    Uint32 _responseCacheMaximumSize;

    Uint64 _startTime;
    // Max number of objects in the cache.
    Uint32 _cacheHighWaterMark;

    Magic<0x57D11474> _magic;
};

// Inline functions

inline String& EnumerationContext::getName()
{
    return _enumerationContextName;
}

inline CIMResponseData::ResponseDataContent
    EnumerationContext::getCIMResponseDataType()
{
    return _responseCache.getResponseDataContent();
}

inline CIMResponseData& EnumerationContext::getResponseData()
{
    return _responseCache;
}

inline Boolean EnumerationContext::isProcessing()
{
    return _processing;
}

inline Boolean EnumerationContext::isClosed()
{
    return _clientClosed;
}

inline Boolean EnumerationContext::isErrorState()
{
    return _error;
}

/*
    Test the current pull message against the type set on the create
    context. they must match
*/
inline Boolean EnumerationContext::isValidPullRequestType(
   MessageType type) const
{
    return(type == _pullRequestType);
}

inline Boolean EnumerationContext::ifProvidersComplete() const
{
    return _providersComplete;
}

inline Uint32 EnumerationContext::responseCacheSize()
{
    PEGASUS_ASSERT(valid());   // KS_TEMP
    return _responseCache.size();
}

inline CIMNamespaceName EnumerationContext::getNamespace() const
{
    return _nameSpace;
}

PEGASUS_NAMESPACE_END

#endif /* PegasusEnumerationContext_h */
