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

#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/IPCExceptions.h>
#include "Semaphore.h"

PEGASUS_NAMESPACE_BEGIN

static const Uint32 PEGASUS_SEM_VALUE_MAX = 0x0000ffff;

//==============================================================================
//
// PEGASUS_USE_PTHREAD_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_USE_PTHREAD_SEMAPHORE)

Semaphore::Semaphore(Uint32 initial)
{
    pthread_mutex_init(&_rep.mutex, NULL);
    pthread_cond_init(&_rep.cond, NULL);

    if (initial > PEGASUS_SEM_VALUE_MAX)
        _count = PEGASUS_SEM_VALUE_MAX - 1;
    else
        _count = initial;

    _rep.owner = Threads::self();
    _rep.waiters = 0;
}

Semaphore::~Semaphore()
{
#if !defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    && !defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    pthread_mutex_lock(&_rep.mutex);
    int r = 0;
    while ((r = pthread_cond_destroy(&_rep.cond) == EBUSY) ||
           (r == -1 && errno == EBUSY))
    {
        pthread_mutex_unlock(&_rep.mutex);
        Threads::yield();
        pthread_mutex_lock(&_rep.mutex);
    }
    pthread_mutex_unlock(&_rep.mutex);
    pthread_mutex_destroy(&_rep.mutex);
#else
    int val;
    val = pthread_mutex_destroy(&_rep.mutex);

    if (val != 0)
        pthread_cond_destroy(&_rep.cond);
    else
        val = pthread_cond_destroy(&_rep.cond);

    while (EBUSY == val)
    {
        Threads::yield();
        val = pthread_mutex_destroy(&_rep.mutex);
        if (val != 0)
            pthread_cond_destroy(&_rep.cond);
        else
            val = pthread_cond_destroy(&_rep.cond);
    }
#endif
}

#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
// cleanup function
static void semaphore_cleanup(void *arg)
{
    // cast back to proper type and unlock mutex
    SemaphoreRep *s = (SemaphoreRep *) arg;
    pthread_mutex_unlock(&s->mutex);
}
#endif

// block until this semaphore is in a signalled state or
// throw an exception if the wait fails
void Semaphore::wait(Boolean ignoreInterrupt)
{
    // Acquire mutex to enter critical section.
    pthread_mutex_lock(&_rep.mutex);

    // Push cleanup function onto cleanup stack
    // The mutex will unlock if the thread is killed early
#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    Threads::cleanup_push(&semaphore_cleanup, &_rep);
#endif

    // Keep track of the number of waiters so that <sema_post> works correctly.
    _rep.waiters++;

    // Wait until the semaphore count is > 0, then atomically release
    // <lock_> and wait for <count_nonzero_> to be signaled.
    while (_count == 0)
        pthread_cond_wait(&_rep.cond, &_rep.mutex);

    // <_rep.mutex> is now held.

    // Decrement the waiters count.
    _rep.waiters--;

    // Decrement the semaphore's count.
    _count--;

    // Since we push an unlock onto the cleanup stack
    // We will pop it off to release the mutex when leaving the critical
    // section.
#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    Threads::cleanup_pop(1);
#endif
    // Release mutex to leave critical section.
    pthread_mutex_unlock(&_rep.mutex);
}

void Semaphore::try_wait()
{
// not implemented
    throw(WaitFailed(_rep.owner));
}

void Semaphore::time_wait(Uint32 milliseconds)
{
    // Acquire mutex to enter critical section.
    pthread_mutex_lock(&_rep.mutex);
    Boolean timedOut = false;

#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    // Push cleanup function onto cleanup stack
    // The mutex will unlock if the thread is killed early
    Threads::cleanup_push(&semaphore_cleanup, &_rep);
#endif

    // Keep track of the number of waiters so that <sema_post> works correctly.
    _rep.waiters++;

    struct timeval now = { 0, 0 };
    struct timespec waittime = { 0, 0 };
    gettimeofday(&now, NULL);
    waittime.tv_sec = now.tv_sec;
    waittime.tv_nsec = now.tv_usec + (milliseconds * 1000);     // microseconds
    waittime.tv_sec += (waittime.tv_nsec / 1000000);    // roll overflow into
    waittime.tv_nsec = (waittime.tv_nsec % 1000000);    // the "seconds" part
    waittime.tv_nsec = waittime.tv_nsec * 1000; // convert to nanoseconds

    while ((_count == 0) && !timedOut)
    {
        int r = pthread_cond_timedwait(&_rep.cond, &_rep.mutex, &waittime);

        if (((r == -1 && errno == ETIMEDOUT) || (r == ETIMEDOUT)) &&
            _count == 0)
        {
            timedOut = true;
        }
    }

    if (!timedOut)
    {
        // Decrement the semaphore's count.
        _count--;
    }

    // Decrement the waiters count.
    _rep.waiters--;

#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    // Since we push an unlock onto the cleanup stack
    // We will pop it off to release the mutex when leaving the critical
    // section.
    Threads::cleanup_pop(1);
#endif

    // Release mutex to leave critical section.
    pthread_mutex_unlock(&_rep.mutex);

    if (timedOut)
    {
        throw TimeOut(Threads::self());
    }
}

// increment the count of the semaphore
void Semaphore::signal()
{
    pthread_mutex_lock(&_rep.mutex);

    // Always allow one thread to continue if it is waiting.
    if (_rep.waiters > 0)
        pthread_cond_signal(&_rep.cond);

    // Increment the semaphore's count.
    _count++;

    pthread_mutex_unlock(&_rep.mutex);
}

// return the count of the semaphore
int Semaphore::count() const
{
    return _count;
}

#endif /* PEGASUS_USE_PTHREAD_SEMAPHORE */

//==============================================================================
//
// PEGASUS_USE_POSIX_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_USE_POSIX_SEMAPHORE)

Semaphore::Semaphore(Uint32 initial)
{
    if (initial > PEGASUS_SEM_VALUE_MAX)
        initial = PEGASUS_SEM_VALUE_MAX - 1;
    sem_init(&_rep.sem, 0, initial);
    _rep.owner = Threads::self();
}

Semaphore::~Semaphore()
{
    while (EBUSY == sem_destroy(&_rep.sem))
    {
        Threads::yield();
    }
}

// block until this semaphore is in a signalled state, or
// throw an exception if the wait fails
void Semaphore::wait(Boolean ignoreInterrupt)
{
    do
    {
        int rc = sem_wait(&_rep.sem);
        if (rc == 0)
            break;

        int e = errno;
        if (e == EINTR)
        {
            if (ignoreInterrupt == false)
                throw(WaitInterrupted(_rep.owner));
        }
        else
            throw(WaitFailed(_rep.owner));

        // keep going if above conditions fail
    }
    while (true);

}

// wait succeeds immediately if semaphore has a non-zero count,
// return immediately and throw and exception if the
// count is zero.
void Semaphore::try_wait()
{
    if (sem_trywait(&_rep.sem))
        throw(WaitFailed(_rep.owner));
}




// Note: I could not get sem_timed_wait to work reliably.
// See my comments above on mut timed_wait.
// I reimplemented using try_wait, which works reliably.
// mdd Sun Aug  5 13:25:31 2001

// wait for milliseconds and throw an exception
// if wait times out without gaining the semaphore
void Semaphore::time_wait(Uint32 milliseconds)
{
    int retcode, i = 0;

    struct timeval now, finish, remaining;
    Uint32 usec;

    gettimeofday(&finish, NULL);
    finish.tv_sec += (milliseconds / 1000);
    milliseconds %= 1000;
    usec = finish.tv_usec + (milliseconds * 1000);
    finish.tv_sec += (usec / 1000000);
    finish.tv_usec = usec % 1000000;

    while (1)
    {
        do
        {
            retcode = sem_trywait(&_rep.sem);
        }
        while (retcode == -1 && errno == EINTR);

        if (retcode == 0)
            return;

        if (retcode == -1 && errno != EAGAIN)
            throw IPCException(Threads::self());
        gettimeofday(&now, NULL);
        if (Time::subtract(&remaining, &finish, &now))
            throw TimeOut(Threads::self());
        Threads::yield();
    }
}

// increment the count of the semaphore
void Semaphore::signal()
{
    sem_post(&_rep.sem);
}

// return the count of the semaphore
int Semaphore::count() const
{
    sem_getvalue(&_rep.sem, &_count);
    return _count;
}

#endif /* PEGASUS_USE_POSIX_SEMAPHORE */

//==============================================================================
//
// PEGASUS_USE_WINDOWS_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_USE_WINDOWS_SEMAPHORE)

Semaphore::Semaphore(Uint32 initial)
{
    if (initial > PEGASUS_SEM_VALUE_MAX)
        initial = PEGASUS_SEM_VALUE_MAX - 1;
    _count = initial;
    _rep.owner = Threads::self();
    _rep.sem = CreateSemaphore(NULL, initial, PEGASUS_SEM_VALUE_MAX, NULL);
}

Semaphore::~Semaphore()
{
    CloseHandle(_rep.sem);
}

// block until this semaphore is in a signalled state
// note that windows does not support interrupt
void Semaphore::wait(Boolean ignoreInterrupt)
{
    DWORD errorcode = WaitForSingleObject(_rep.sem, INFINITE);
    if (errorcode != WAIT_FAILED)
        _count--;
    else
        throw(WaitFailed(Threads::self()));
}

// wait succeeds immediately if semaphore has a non-zero count,
// return immediately and throw and exception if the
// count is zero.
void Semaphore::try_wait()
{
    DWORD errorcode = WaitForSingleObject(_rep.sem, 0);
    if (errorcode == WAIT_TIMEOUT || errorcode == WAIT_FAILED)
        throw(WaitFailed(Threads::self()));
    _count--;
}


// wait for milliseconds and throw an exception
// if wait times out without gaining the semaphore
void Semaphore::time_wait(Uint32 milliseconds)
{
    DWORD errorcode = WaitForSingleObject(_rep.sem, milliseconds);
    if (errorcode == WAIT_TIMEOUT || errorcode == WAIT_FAILED)
        throw(TimeOut(Threads::self()));
    _count--;
}

// increment the count of the semaphore
void Semaphore::signal()
{
    _count++;
    ReleaseSemaphore(_rep.sem, 1, NULL);
}

// return the count of the semaphore
int Semaphore::count() const
{
    return _count;
}

#endif /* PEGASUS_USE_WINDOWS_SEMAPHORE */

PEGASUS_NAMESPACE_END
