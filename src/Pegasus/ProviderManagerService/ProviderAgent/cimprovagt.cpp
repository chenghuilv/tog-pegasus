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

#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/AnonymousPipe.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/ProviderManagerService/ProviderAgent/ProviderAgent.h>

#if defined(PEGASUS_OS_TYPE_UNIX) && !defined(PEGASUS_OS_OS400)
# include <unistd.h>
# include <sys/types.h>
# include <errno.h>
#endif

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

void usage()
{
    cerr << "cimprovagt is an internal program used by cimserver." << endl;
    cerr << "cimprovagt should not be invoked directly." << endl;
}

int setUserContext(int argc, char* argv[])
{
#if !defined(PEGASUS_DISABLE_PROV_USERCTXT) && \
    !defined(PEGASUS_OS_ZOS) && \
    !defined(PEGASUS_OS_OS400)

    PEGASUS_UID_T newUid = (PEGASUS_UID_T)-1;
    PEGASUS_GID_T newGid = (PEGASUS_GID_T)-1;

    if (!System::lookupUserId(argv[4], newUid, newGid))
    {
        return -1;
    }

    if (!System::changeUserContext_SingleThreaded(argv[4], newUid, newGid))
    {
        return -1;
    }

# if defined(PEGASUS_OS_TYPE_UNIX)

    // Execute a new cimprovagt process to reset the saved user id and group id.

    int pid = (int)fork();

    if (pid < 0)
    {
        cerr << "fork failed: " << strerror(errno) << endl;
        return -1;
    }

    if (pid > 0)
    {
        // Parent process
        _exit(0);
    }

    setsid();

    if (execl(
            argv[0],
            argv[0],
            "0",
            argv[2],
            argv[3],
            argv[4],
            argv[5],
            0) == -1)
    {
        cerr << "execl failed: " << strerror(errno) << endl;
        return -1;
    }
# endif

    return 0;

#else

    return -1;

#endif
}

//
// Dummy function for the Thread object associated with the initial thread.
// Since the initial thread is used to process CIM requests, this is
// needed to localize the exceptions thrown during CIM request processing.
// Note: This function should never be called! 
// 
ThreadReturnType PEGASUS_THREAD_CDECL dummyThreadFunc(void *parm)
{
    return(ThreadReturnType(0));
}

/////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    // Usage: cimprovagt ( 0 | 1 ) <input_pipe> <output_pipe> <user_name> <id>

    //
    // Get the arguments from the command line
    // arg1 is a flag indicating whether the user context must be set
    // arg2 is the input pipe handle
    // arg3 is the output pipe handle
    // arg4 is a user name defining the user context for this provider agent
    // arg5 is the Provider Module Name (used for process identification)
    //

    if (argc < 6)
    {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "1") == 0)
    {
        if (setUserContext(argc, argv) == -1)
        {
            cerr << "Failed to set user context for user " << argv[4] << endl;
            return 1;
        }
    }

    const char* moduleName;

    try
    {
        AnonymousPipe pipeFromServer(argv[2], 0);
        AnonymousPipe pipeToServer(0, argv[3]);
        const char* userName = argv[4];
        moduleName = argv[5];

        Tracer::setModuleName(String(moduleName) + "." + userName);

        // Set message loading not to use process locale
        MessageLoader::_useProcessLocale = false;

        // Create a dummy Thread object that can be used to store the
        // AcceptLanguageList object for CIM requests that are serviced
        // by this thread (initial thread of cimprovagt).  Need to do this
        // because this thread is not in a ThreadPool, but is used
        // to service CIM requests.
        // The run function for the dummy Thread should never be called,
        Thread *dummyInitialThread = new Thread(dummyThreadFunc, NULL, false);
        Thread::setCurrent(dummyInitialThread); 
        AcceptLanguageList default_al;
        try
        {
            default_al = LanguageParser::getDefaultAcceptLanguages();   
            Thread::setLanguages(new AcceptLanguageList(default_al));
        }
        catch (InvalidAcceptLanguageHeader& e)
        {
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                "src.Server.cimserver.FAILED_TO_SET_PROCESS_LOCALE",
                "Could not convert the system process locale into a valid "
                    "AcceptLanguage format.");  
            Logger::put(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                e.getMessage()); 
        }

        //
        // Instantiate and run the Provider Agent
        //
        ProviderAgent providerAgent(moduleName, &pipeFromServer, &pipeToServer);
        providerAgent.run();
    }
    catch (Exception& e)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            "ProviderManager.ProviderAgent.cimprovagt.CIMPROVAGT_EXCEPTION",
            "cimprovagt \"$0\" error: $1", moduleName, e.getMessage());
        return 1;
    }
    catch (...)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            "ProviderManager.ProviderAgent.cimprovagt.CIMPROVAGT_ERROR",
            "cimprovagt \"$0\" error.  Exiting.", moduleName);
        return 1;
    }

    return 0;
}
