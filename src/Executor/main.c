/*
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
//%/////////////////////////////////////////////////////////////////////////////
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "Config.h"
#include "Child.h"
#include "Parent.h"
#include "User.h"
#include "Fatal.h"
#include "Process.h"
#include "Path.h"
#include "Globals.h"
#include "Socket.h"
#include "Strlcpy.h"
#include "Strlcat.h"
#include "Log.h"
#include "Policy.h"
#include "Macro.h"
#include "Assert.h"
#include "Options.h"

/*
**==============================================================================
**
** GetServerUser
**
**     Determine which user to run CIMSERVERMAIN as.
**
**     Note: this algorithm is no longer in use.
**
**==============================================================================
*/

int GetServerUser(int* uid, int* gid)
{
    const char* username = PEGASUS_CIMSERVERMAIN_USER;

    if (GetUserInfo(username, uid, gid) != 0)
    {
        Fatal(FL, 
            "The %s user does not exist: \"%s\". Please create "
            "a system user with that name or use an OpenPegasus build that "
            "disables privilege separation.", CIMSERVERMAIN, username);
    }

    return -1;
}

/*
**==============================================================================
**
** ReadPidFile()
**
**==============================================================================
*/

static int ReadPidFile(const char* path, int* pid)
{
    FILE* is = fopen(path, "r");

    if (!is) 
        return -1;

    *pid = 0;

    fscanf(is, "%d\n", pid);
    fclose(is);

    if (*pid == 0)
        return -1;

    return 0;
}

/*
**==============================================================================
**
** TestCimServerProcess()
**
**     Returns 0 if cimserver process is running.
**
**==============================================================================
*/

int TestCimServerProcess()
{
    int pid;
    char name[EXECUTOR_BUFFER_SIZE];

    if (ReadPidFile(PEGASUS_CIMSERVER_START_FILE, &pid) != 0)
        return -1;

    if (GetProcessName(pid, name) != 0 || strcmp(name, CIMSERVERMAIN) != 0)
        return -1;

    return 0;
}

/*
**==============================================================================
**
** ExecShutdown()
**
**==============================================================================
*/

void ExecShutdown()
{
    char* tmpArgv[3];
    const char* cimshutdownPath;
    const char* shutdownTimeout;

    /* Get shutdownTimeout configuration parameter. */

    if ((shutdownTimeout = FindMacro("shutdownTimeout")) == NULL)
        Fatal(FL, "failed to resolve shutdownTimeout");

    /* Get absolute CIMSHUTDOWN program name. */

    if ((cimshutdownPath = FindMacro("cimshutdownPath")) == NULL)
        Fatal(FL, "failed to resolve cimshutdownPath");

    /* Create argument list. */

    tmpArgv[0] = CIMSHUTDOWN;
    tmpArgv[1] = (char*)shutdownTimeout;
    tmpArgv[2] = 0;

    /* Exec CIMSHUTDOWN program. */

    /* Flawfinder: ignore */
    execv(cimshutdownPath, tmpArgv);
    Fatal(FL, "failed to exec %s", cimshutdownPath);
}

/*
**==============================================================================
**
** DefineExecutorMacros()
**
**     Define macros used by the executor.
**
**==============================================================================
*/

void DefineExecutorMacros()
{
    /* Define ${internalBinDir} */
    {
        char path[EXECUTOR_BUFFER_SIZE];

        if (GetPegasusInternalBinDir(path) != 0)
            Fatal(FL, "failed to resolve internal pegasus bin directory");

        DefineMacro("internalBinDir", path);
    }

    /* Define ${cimservermain} */

    DefineMacro("cimservermain", CIMSERVERMAIN);

    /* Define ${cimprovagt} */

    DefineMacro("cimprovagt", CIMPROVAGT);

    /* Define ${cimshutdown} */

    DefineMacro("cimshutdown", CIMSHUTDOWN);

    /* Define ${cimservera} */

    DefineMacro("cimservera", CIMSERVERA);

    /* Define ${cimservermainPath} */
    {
        char path[EXECUTOR_BUFFER_SIZE];

        if (ExpandMacros("${internalBinDir}/${cimservermain}", path) != 0)
            Fatal(FL, "failed to resolve cimservermainPath");

        DefineMacro("cimservermainPath", path);
    }

    /* Define ${cimprovagtPath} */
    {
        char path[EXECUTOR_BUFFER_SIZE];

        if (ExpandMacros("${internalBinDir}/${cimprovagt}", path) != 0)
            Fatal(FL, "failed to resolve cimprovagtPath");

        DefineMacro("cimprovagtPath", path);
    }

    /* Define ${cimshutdownPath} */
    {
        char path[EXECUTOR_BUFFER_SIZE];

        if (ExpandMacros("${internalBinDir}/${cimshutdown}", path) != 0)
            Fatal(FL, "failed to resolve cimshutdownPath");

        DefineMacro("cimshutdownPath", path);
    }

    /* Define ${cimserveraPath} */
    {
        char path[EXECUTOR_BUFFER_SIZE];

        if (ExpandMacros("${internalBinDir}/${cimservera}", path) != 0)
            Fatal(FL, "failed to resolve cimserveraPath");

        DefineMacro("cimserveraPath", path);
    }

    /* Define ${shutdownTimeout} */

    {
        char buffer[EXECUTOR_BUFFER_SIZE];

        if (GetConfigParam("shutdownTimeout", buffer) != 0)
            Strlcpy(buffer, "5", sizeof(buffer));

        DefineMacro("shutdownTimeout", buffer);
    }

    /* Define ${currentConfigFilePath} */
    {
        char path[EXECUTOR_BUFFER_SIZE];

        if (GetHomedPath(PEGASUS_CURRENT_CONFIG_FILE_PATH, path) != 0)
        {
            Fatal(FL, "GetHomedPath() failed on \"%s\"", 
                PEGASUS_CURRENT_CONFIG_FILE_PATH);
        }

        DefineMacro("currentConfigFilePath", path);
    }

    /* Define ${repositoryDir} */

    if (DefineConfigPathMacro("repositoryDir", PEGASUS_REPOSITORY_DIR) != 0)
        Fatal(FL, "missing \"repositoryDir\" configuration parameter.");

    /* Define ${passwordFilePath} */

    if (DefineConfigPathMacro("passwordFilePath", "cimserver.passwd") != 0)
        Fatal(FL, "missing \"passwordFilePath\" configuration parameter.");

    /* Define ${traceFilePath} */

    if (DefineConfigPathMacro("traceFilePath", NULL) != 0)
        Fatal(FL, "missing \"traceFilePath\" configuration parameter.");

    /* Define ${sslKeyFilePath} */

    if (DefineConfigPathMacro("sslKeyFilePath", "file.pem") != 0)
        Fatal(FL, "missing \"sslKeyFilePath\" configuration parameter.");

    /* Define ${sslTrustStore} */

    if (DefineConfigPathMacro("sslTrustStore", "cimserver_trust") != 0)
        Fatal(FL, "missing \"sslTrustStore\" configuration parameter.");

    /* Define ${crlStore} */

    if (DefineConfigPathMacro("crlStore", "crl") != 0)
        Fatal(FL, "missing \"crlStore\" configuration parameter.");

    /* Define ${privilegedUser} */

    DefineMacro("privilegedUser", "root");

    /* Define ${cimserverUser} */

    DefineMacro("cimserverUser", PEGASUS_CIMSERVERMAIN_USER);
}

/*
**==============================================================================
**
** main()
**
**==============================================================================
*/

int main(int argc, char** argv)
{
    const char* cimservermainPath;
    int pair[2];
    char username[EXECUTOR_BUFFER_SIZE];
    int childPid;
    const char* repositoryDir;
    struct Options options;

    /* Get options. */

    GetOptions(&argc, &argv, &options);

    /* Save argc-argv as globals. */

    globals.argc = argc;
    globals.argv = argv;

    /* Define macros needed by the executor. */

    DefineExecutorMacros();

    /* If shuting down, then run CIMSHUTDOWN client. */

    if (options.shutdown)
        ExecShutdown();

    /* Process --policy and --macros options. */

    if (options.policy || options.macros)
    {
        if (options.policy)
            DumpPolicy(1);

        if (options.macros)
            DumpMacros();

        exit(0);
    }

    /* Get absolute CIMSERVERMAIN program name. */

    if ((cimservermainPath = FindMacro("cimservermainPath")) == NULL)
        Fatal(FL, "Failed to locate %s program", CIMSERVERMAIN);

    /* If CIMSERVERMAIN is already running, warn and exit now, unless one of
     * the following options are given: -v, --version, -h, --help (these are
     * passed through to CIMSERVERMAIN).
     */

    if (!options.version && !options.help && TestCimServerProcess() == 0)
    {
        fprintf(stderr,
            "%s: cimserver is already running (the PID found in the file "
            "\"%s\" corresponds to an existing process named \"%s\").\n\n",
            globals.argv[0], PEGASUS_CIMSERVER_START_FILE, CIMSERVERMAIN);

        exit(1);
    }

    /* Get enableAuthentication configuration option. */

    {
        char buffer[EXECUTOR_BUFFER_SIZE];

        if (GetConfigParam("enableAuthentication", buffer) == 0 &&
            strcasecmp(buffer, "true") == 0)
        {
            globals.enableAuthentication = 1;
        }
    }

    /* Create a socket pair for communicating with the child process. */

    if (CreateSocketPair(pair) != 0)
        Fatal(FL, "Failed to create socket pair");

    CloseOnExec(pair[1]);

    /* Get the log-level from the configuration parameter. */

    GetLogLevel(argc, argv);

    /* Open the log. */

    OpenLog("cimexecutor", options.perror);

    Log(LL_INFORMATION, "starting");

    /* Be sure this process is running as root (otherwise fail). */

    if (setuid(0) != 0 || setgid(0) != 0)
        Fatal(FL, "attempted to run program as non-root user");

    /* Warn if authentication not enabled (suspicious use of executor). */

    if (!globals.enableAuthentication)
        Log(LL_WARNING, "authentication is NOT enabled");
    else
        Log(LL_TRACE, "authentication is enabled");

    /* Print user info. */

    if (GetUserName(getuid(), username) != 0)
        Fatal(FL, "cannot resolve user from uid=%d", getuid());

    Log(LL_TRACE, "running as %s (uid=%d, gid=%d)",
        username, (int)getuid(), (int)getgid());

    /* Determine user for running CIMSERVERMAIN. */

    GetServerUser(&globals.childUid, &globals.childGid);

    /* Get repositoryDir for child. */

    if ((repositoryDir = FindMacro("repositoryDir")) == NULL)
        Fatal(FL, "failed to find repositoryDir macro");

    /* Fork child process. */

    childPid = fork();

    if (childPid == 0)
    {
        /* Child. */
        close(pair[1]);
        Child(argc, argv, cimservermainPath, globals.childUid, 
            globals.childGid, pair[0], repositoryDir);
    }
    else if (childPid > 0)
    {
        /* Parent. */
        close(pair[0]);
        Parent(pair[1], childPid);
    }
    else
    {
        Fatal(FL, "fork() failed");
    }

    return 0;
}
