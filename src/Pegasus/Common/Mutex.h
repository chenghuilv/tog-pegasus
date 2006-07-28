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
// Author: Mike Brasher (m.brasher@inovadevelopment.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Mutex_h
#define Pegasus_Mutex_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/IPCExceptions.h>
#include <Pegasus/Common/Magic.h>
#include <Pegasus/Common/Threads.h>

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// MutexRep
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)
typedef pthread_mutex_t MutexType;
struct MutexRep
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
};
inline void mutex_lock(MutexType* mutex) { pthread_mutex_lock(mutex); }
inline void mutex_unlock(MutexType* mutex) { pthread_mutex_unlock(mutex); }
# define PEGASUS_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#endif

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)
typedef HANDLE MutexType;
struct MutexRep
{
    MutexType handle;
    size_t count;
};
inline void mutex_lock(MutexType* m) { WaitForSingleObject(*m, INFINITE); }
inline void mutex_unlock(MutexType* m) { ReleaseMutex(*m); }
# define PEGASUS_MUTEX_INITIALIZER (CreateMutex(NULL, FALSE, NULL))
#endif

//==============================================================================
//
// Mutex
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE Mutex
{
public:

    Mutex();

    ~Mutex();

    void lock();

    void try_lock();

    void timed_lock(Uint32 milliseconds);

    void unlock();

private:
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);


    MutexRep _rep;
    Magic<0x57D11485> _magic;

    friend class Condition;
};

//==============================================================================
//
// AutoMutex
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE AutoMutex
{
public:

    AutoMutex(Mutex& mutex, Boolean autoLock = true) : 
	_mutex(mutex), _locked(autoLock)
    {
        if (autoLock)
            _mutex.lock();
    }

    ~AutoMutex()
    {
        try
        {
	    if (_locked)
		unlock();
        }
        catch (...)
        {
            // Do not propagate exception from destructor
        }
    }

    void lock()
    {
        if (_locked)
            throw AlreadyLocked(ThreadType());

        _mutex.lock();
        _locked = true;
    }

    void unlock()
    {
        if (!_locked)
            throw Permission(ThreadType());

        _mutex.unlock();
        _locked = false;
    }

    Boolean isLocked() const
    {
        return _locked;
    }

private:
    AutoMutex(); // Unimplemented
    AutoMutex(const AutoMutex& x); // Unimplemented
    AutoMutex& operator=(const AutoMutex& x); // Unimplemented

    Mutex& _mutex;
    Boolean _locked;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Mutex_h */
