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

#ifndef Pegasus_Semaphore_h
#define Pegasus_Semaphore_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Threads.h>

//==============================================================================
//
// Select sempahore implementation by defining one of these:
//
//    PEGASUS_USE_POSIX_SEMAPHORE
//    PEGASUS_USE_PTHREAD_SEMAPHORE
//    PEGASUS_USE_WINDOWS_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM) || \
    defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) || \
    defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX) || \
    defined(PEGASUS_OS_DARWIN) || \
    defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
    defined(PEGASUS_PLATFORM_VMS_IA64_DECCXX) || \
    defined(PEGASUS_PLATFORM_VMS_ALPHA_DECCXX)
# define PEGASUS_USE_PTHREAD_SEMAPHORE
#elif defined(PEGASUS_OS_TYPE_WINDOWS)
# define PEGASUS_USE_WINDOWS_SEMAPHORE
#else
# define PEGASUS_USE_POSIX_SEMAPHORE
#endif

#if defined(PEGASUS_USE_POSIX_SEMAPHORE)
# include <semaphore.h>
#endif

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// SemaphoreRep
//
//==============================================================================

#if defined(PEGASUS_USE_PTHREAD_SEMAPHORE)
struct SemaphoreRep
{
    Uint32 waiters;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    ThreadType owner;
};
#endif /* PEGASUS_USE_PTHREAD_SEMAPHORE */

#if defined(PEGASUS_USE_POSIX_SEMAPHORE)
struct SemaphoreRep
{
    sem_t sem;
    ThreadType owner;
};
#endif /* defined(PEGASUS_USE_POSIX_SEMAPHORE) */

#if defined(PEGASUS_USE_WINDOWS_SEMAPHORE)
struct SemaphoreRep
{
    HANDLE sem;
    ThreadType owner;
};
#endif /* PEGASUS_USE_WINDOWS_SEMAPHORE */

//==============================================================================
//
// Semaphore
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE Semaphore
{
public:

    /** Creates a semaphore and sets its initial value as specified.
        @param initial The initial value for the Semaphore (defaults to 1).
    */
    Semaphore(Uint32 initial = 1);

    /** Destructor.
    */
    ~Semaphore();

    /** Blocks until this Semaphore is in a signalled state.
        @param ignoreInterrupt Indicates whether the wait operation should
        continue (true) or an exception should be thrown (false) when an
        interrupt is received.
        @exception WaitFailed If unable to block on the semaphore.
        @exception WaitInterrupted If the operation is interrupted.
    */
    void wait(Boolean ignoreInterrupt = true);

    /** Checks whether the Semaphore is signalled without waiting.  This method
        returns normally if the Semaphore has a non-zero count.
        @exception WaitFailed If the wait operation does not immediately
        succeed.
    */
    void try_wait();

    /** Waits for the Semaphore to be signalled for a specified time interval.
        This method returns normally if the Semaphore has a non-zero count or
        it is signalled during the specified time interval.
        @param milliseconds The time interval to wait (in milliseconds).
        @exception TimeOut If the wait operation does not succeed within
        the specified time interval.
    */
    void time_wait(Uint32 milliseconds);

    /** Increments the count of the semaphore.
    */
    void signal();

    /** Return the count of the semaphore.
    */
    int count() const;

private:

    Semaphore(const Semaphore& x); // Unimplemented
    Semaphore& operator=(const Semaphore& x); // Unimplemented

    mutable int _count;
    mutable SemaphoreRep _rep;
    friend class Condition;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Semaphore_h */
