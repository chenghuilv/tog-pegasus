//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Author: Markus Mueller (markus_mueller@de.ibm.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Amit K Arora, IBM (amita@in.ibm.com) for Bug#1090
//
//%/////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstring>
#ifdef PEGASUS_PLATFORM_SOLARIS_SPARC_CC
#include <iostream.h>
#endif

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Signal.h>

PEGASUS_USING_PEGASUS;

void sig_act(int s_n, PEGASUS_SIGINFO_T * s_info, void * sig)
{
    PEGASUS_THREAD_RETURN retval = 0;

    if (s_n == PEGASUS_SIGABRT)
    {
        printf("Received an abort signal\n");
#ifdef PEGASUS_HAS_SIGNALS
        printf(" in address %p\n", s_info->si_addr);
#endif

        // In general it is dangerous to call pthread from within a
        // signal handler, because they are not signal safe
        exit_thread(retval);
    }
}


PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_HAS_SIGNALS

SignalHandler::SignalHandler() : reg_mutex()
{
   for(Uint32 i=0;i < 32;i++)
   {
       reg_handler[i].active = 0;
       reg_handler[i].sh = NULL;
       memset(&reg_handler[i].oldsa,0,sizeof(struct sigaction));
   }
}

SignalHandler::~SignalHandler()
{
   deactivateAll();
}

void SignalHandler::registerHandler(Uint32 signum, signal_handler _sighandler)
{
    AutoMutex autoMut(reg_mutex);
    deactivate_i(signum);
    reg_handler[signum].sh = _sighandler;
}

void SignalHandler::activate(Uint32 signum)
{
    AutoMutex autoMut(reg_mutex);
    if (reg_handler[signum].active) return; // throw exception

    struct sigaction * sig_acts = new struct sigaction;

    sig_acts->sa_sigaction = reg_handler[signum].sh;
    sigfillset(&(sig_acts->sa_mask));
#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) || defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM) || defined(PEGASUS_PLATFORM_HPUX_ACC) || defined(PEGASUS_PLATFORM_OS400_ISERIES_IBM) || defined(PEGASUS_PLATFORM_SOLARIS_SPARC_CC) || defined(PEGASUS_PLATFORM_DARWIN_PPC_GNU)
    sig_acts->sa_flags = SA_SIGINFO | SA_RESETHAND;
#else
    sig_acts->sa_flags = SA_SIGINFO | SA_ONESHOT;
#if !defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
    sig_acts->sa_restorer = NULL;
#endif
#endif

    sigaction(signum, sig_acts, &reg_handler[signum].oldsa);

    reg_handler[signum].active = -1;

    delete sig_acts;
}

void SignalHandler::deactivate(Uint32 signum)
{
    AutoMutex autoMut(reg_mutex);
    deactivate_i(signum);
}

void SignalHandler::deactivate_i(Uint32 signum)
{
    if (reg_handler[signum].active)
    {
        reg_handler[signum].active = 0;
        sigaction(signum, &reg_handler[signum].oldsa, NULL);
    }
}

void SignalHandler::deactivateAll()
{
    AutoMutex autoMut(reg_mutex);
    for (Uint32 i=0; i < 32; i++)
        if (reg_handler[i].active) deactivate_i(i);
}

void SignalHandler::ignore(Uint32 signum)
{
#if !defined(PEGASUS_PLATFORM_OS400_ISERIES_IBM) && !defined(PEGASUS_PLATFORM_DARWIN_PPC_GNU)
    ::sigignore(signum);
#else
    struct sigaction * sig_acts = new struct sigaction;

    sig_acts->sa_handler = SIG_IGN;
    sigfillset(&(sig_acts->sa_mask));
    sig_acts->sa_flags = 0;

    sigaction(signum, sig_acts, NULL);

    delete sig_acts;	
#endif
}

#else // PEGASUS_HAS_SIGNALS

SignalHandler::SignalHandler() { }

SignalHandler::~SignalHandler() { }

void SignalHandler::registerHandler(Uint32 signum, signal_handler _sighandler)
{ }

void SignalHandler::activate(Uint32 signum) { }

void SignalHandler::deactivate(Uint32 signum) { }

void SignalHandler::deactivateAll() { }

void SignalHandler::ignore(Uint32 signum) { }

#endif // PEGASUS_HAS_SIGNALS


// export the global signal handling object

SignalHandler _globalSignalHandler;
SignalHandler * _globalSignalHandlerPtr = &_globalSignalHandler;
SignalHandler * getSigHandle() { return _globalSignalHandlerPtr; }


PEGASUS_NAMESPACE_END


