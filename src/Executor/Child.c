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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <grp.h>
#include "Defines.h"
#include "Globals.h"
#include "Fatal.h"
#include "Path.h"
#include "Log.h"
#include "User.h"

/*
**==============================================================================
**
** Child
**
**     The child process.
**
**==============================================================================
*/

void Child(
    int argc,
    char** argv,
    const char* path,
    const char* userName,
    int uid,
    int gid,
    int sock)
{
    char sockStr[EXECUTOR_BUFFER_SIZE];
    char** execArgv;

    globals.isChildProcess = 1;

    /* Build argument list, adding "--executor-socket <sock>" option if
     * sock non-negative.
     */

    execArgv = (char**)malloc(sizeof(char*) * (argc + 3));
    memcpy(execArgv + 3, argv + 1, sizeof(char*) * argc);

    sprintf(sockStr, "%d", sock);

    execArgv[0] = CIMSERVERMAIN;
    execArgv[1] = "--executor-socket";
    execArgv[2] = strdup(sockStr);

    /*
     * Downgrade privileges by setting the UID and GID of this process. Use
     * the owner of the CIMSERVERMAIN program obtained above.
     */

    if (uid == 0 || gid == 0)
    {
        Fatal(FL, "root may not own %s since the program is run as owner",
            path);
    }

    if (setgid(gid) != 0)
    {
        Fatal(FL, "Failed to set gid to %d", gid);
    }

    if (initgroups(userName, gid) != 0)
    {
        Fatal(FL, "Failed to initialize groups for user %s", userName);
    }

    if (setuid(uid) != 0)
    {
        Fatal(FL, "Failed to set uid to %d", uid);
    }

    if ((int)getuid() != uid ||
        (int)geteuid() != uid ||
        (int)getgid() != gid ||
        (int)getegid() != gid)
    {
        Fatal(FL, "setuid/setgid verification failed\n");
    }

    /* Log user info. */

    Log(LL_TRACE, "%s running as %s (uid=%d, gid=%d)", CIMSERVERMAIN,
        userName, uid, gid);

    /* Exec child process. */

    /* Flawfinder: ignore */
    execv(path, execArgv);

    /* If we are still here, the exec failed. */
    Fatal(FL, "failed to exec %s", path);
}
