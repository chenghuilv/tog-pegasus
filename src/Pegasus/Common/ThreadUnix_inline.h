//%2005////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
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
// Author: Mike Day (mdday@us.ibm.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////


#ifndef ThreadUnix_inline_h
#define ThreadUnix_inline_h

#ifdef PEGASUS_PLATFORM_SOLARIS_SPARC_CC
// _start wrapper to overcome "C" "C++" binding warnings
// (code "borrowed" from ThreadzOS_inline.h)
// Actually the Solaris compiler doesn't need this as "C" "C++"
// bindings are the same, but it moans like hell about it !!
// (Its correct to moan, but its a pain all the same).


typedef struct {                                   
    void * (PEGASUS_THREAD_CDECL * _start)(void *);
    void * realParm;                               
} zosParmDef;                                      

extern "C" { void * _linkage(void * zosParm); }
                                                   
inline Boolean Thread::run()
{
    zosParmDef * zosParm = (zosParmDef *)malloc(sizeof(zosParmDef));
    zosParm->_start = _start;
    zosParm->realParm = (void *) this;
    if (_is_detached)
    {
        pthread_attr_setdetachstate(&_handle.thatt, PTHREAD_CREATE_DETACHED);
    }
    pthread_attr_setschedpolicy(&_handle.thatt, SCHED_RR);

    int rc;
    rc = pthread_create((pthread_t *)&_handle.thid,
                        &_handle.thatt, &_linkage, zosParm);
    if (rc == EAGAIN)
    {
        _handle.thid = 0;
        return false;
    }
    else if (rc != 0)
    {
        // ATTN: Error behavior has not yet been defined (see Bugzilla 972)
        _handle.thid = 0;
        return true;
    }
    return true;
}
#else // PEGASUS_PLATFORM_SOLARIS_SPARC_CC
inline Boolean Thread::run()
{
    if (_is_detached)
    {
        pthread_attr_setdetachstate(&_handle.thatt, PTHREAD_CREATE_DETACHED);
    }

#ifdef PEGASUS_OS_OS400
    // Initialize the pegasusValue to 1, see IPCOs400.h.
    _handle.thid.pegasusValue = 1;  
#endif

    int rc;
    rc = pthread_create((pthread_t *)&_handle.thid,
                        &_handle.thatt, _start, this);
    if (rc == EAGAIN)
    {
        _handle.thid = 0;
        return false;
    }
    else if (rc != 0)
    {
        // ATTN: Error behavior has not yet been defined (see Bugzilla 972)
        _handle.thid = 0;
        return true;
    }
    return true;
}
#endif // PEGASUS_PLATFORM_SOLARIS_SPARC_CC


inline void Thread::cancel()
{
   _cancelled = true;
   pthread_cancel(_handle.thid);
}

inline void Thread::test_cancel()
{
  pthread_testcancel();
}

inline Boolean Thread::is_cancelled(void)
{
   return _cancelled;
}

inline void Thread::thread_switch()
{
  sched_yield();
}

#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
inline void Thread::suspend()
{
    pthread_kill(_handle.thid,SIGSTOP);
}

inline void Thread::resume()
{
    pthread_kill(_handle.thid,SIGCONT);
}
#endif


inline void Thread::sleep(Uint32 msec)
{
   pegasus_sleep(msec);
}

inline void Thread::join(void) 
{ 
   if((! _is_detached) && (_handle.thid != 0))
      pthread_join(_handle.thid, &_exit_code) ; 
   _handle.thid = 0;
}

inline void Thread::thread_init(void)
{
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  _cancel_enabled = true;
}

// *****----- native thread exit routine -----***** //

#if defined(PEGASUS_PLATFORM_HPUX_ACC) || defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
#define PEGASUS_THREAD_EXIT_NATIVE 
inline void Thread::exit_self(void *return_code) { pthread_exit(return_code) ; }
#endif

// *****----- native cleanup routines -----***** //
#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
#define PEGASUS_THREAD_CLEANUP_NATIVE 
inline void Thread::cleanup_push( void (*routine)(void *), void *parm) throw(IPCException)
{
   cleanup_handler *cu = new cleanup_handler(routine, parm);
   try { _cleanup.insert_first(cu); } 
   catch(IPCException& e) { delete cu; throw; }
   _pthread_cleanup_push(&(cu->_cleanup_buffer), routine, parm);
   return;
}


inline void Thread::cleanup_pop(Boolean execute) throw(IPCException)
{
   cleanup_handler *cu ;
   try { cu = static_cast<cleanup_handler *>(_cleanup.remove_first()) ;}
   catch(IPCException& e) { PEGASUS_ASSERT(0); }
   _pthread_cleanup_pop(&(cu->_cleanup_buffer), execute);
   delete cu;
}

#endif 

inline void Thread::detach(void)
{
   _is_detached = true;
   pthread_detach(_handle.thid);
}

#endif // ThreadUnix_inline_h
