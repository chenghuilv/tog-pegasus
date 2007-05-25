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
#include "LocalAuth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "Defines.h"
#include "Strlcpy.h"
#include "Strlcat.h"
#include "User.h"
#include "Random.h"
#include "Log.h"
#include "User.h"

#define TOKEN_LENGTH 40

/*
**==============================================================================
**
** CreateLocalAuthFile()
**
**     This function creates a local authentication file for the given *user*.
**     it populates the *path* argument and return 0 on success. The file has
**     the following format.
**
**         PEGASUS_LOCAL_AUTH_DIR/cimclient_<user>_<timestamp>_<seq>
**
**     For example:
**
**
**     The algorithm:
**
**         1. Form the path name as shown above.
**            (e.g., /tmp/cimclient_jsmith_1_232).
**
**         2. Generate a random token
**            (e.g., 8F85CB1129B2B93F77F5CCA16850D659CCD16FE0).
**
**         3. Create the file (owner=root, permissions=0400).
**
**         4. Write random token to file.
**
**         5. Change owner of file to *user*.
**
**==============================================================================
*/

static int CreateLocalAuthFile(
    const char* user,
    char path[EXECUTOR_BUFFER_SIZE])
{
    static unsigned int _nextSeq = 1;
    static pthread_mutex_t _nextSeqMutex = PTHREAD_MUTEX_INITIALIZER;
    unsigned int seq;
    struct timeval tv;
    char buffer[EXECUTOR_BUFFER_SIZE];
    char token[TOKEN_LENGTH+1];
    int fd;
    int uid;
    int gid;

    /* Assign next sequence number. */

    pthread_mutex_lock(&_nextSeqMutex);
    seq = _nextSeq++;
    pthread_mutex_unlock(&_nextSeqMutex);

    /* Get microseconds elapsed since epoch. */

    gettimeofday(&tv, NULL);

    /* Build path: */

    Strlcpy(path, PEGASUS_LOCAL_AUTH_DIR, EXECUTOR_BUFFER_SIZE);
    Strlcat(path, "/cimclient_", EXECUTOR_BUFFER_SIZE);
    Strlcat(path, user, EXECUTOR_BUFFER_SIZE);
    sprintf(buffer, "_%u_%u", seq, (int)(tv.tv_usec / 1000));
    Strlcat(path, buffer, EXECUTOR_BUFFER_SIZE);

    /* Generate random token. */

    {
        unsigned char data[TOKEN_LENGTH/2];
        FillRandomBytes(data, sizeof(data));
        RandBytesToHexASCII(data, sizeof(data), token);
    }

    /* If file already exists, remove it. */

    /* Flawfinder: ignore */
    if (access(path, F_OK) == 0 && unlink(path) != 0)
        return -1;

    /* Create the file as read-only by user. */

    fd = open(path, O_WRONLY | O_EXCL | O_CREAT | O_TRUNC, S_IRUSR);

    if (fd < 0)
        return -1;

    /* Write the random token. */

    if (write(fd, token, TOKEN_LENGTH) != TOKEN_LENGTH)
    {
        close(fd);
        unlink(path);
        return -1;
    }

    /* Change owner of file. */

    if (GetUserInfo(user, &uid, &gid) != 0)
    {
        close(fd);
        unlink(path);
        return -1;
    }

    if (fchown(fd, uid, gid) != 0)
    {
        close(fd);
        unlink(path);
        return -1;
    }

    close(fd);
    return 0;
}

/*
**==============================================================================
**
** CheckLocalAuthToken()
**
**     Compare the *token* with the token in the given file. Return 0 if they
**     are identical.
**
**==============================================================================
*/

static int CheckLocalAuthToken(
    const char* path,
    const char* token)
{
    char buffer[TOKEN_LENGTH+1];
    int fd;

    /* Open the file: */

    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;

    /* Read the token. */

    if (read(fd, buffer, TOKEN_LENGTH) != TOKEN_LENGTH)
    {
        close(fd);
        return -1;
    }

    buffer[TOKEN_LENGTH] = '\0';

    /* Compare the token. */

    if (strcmp(token, buffer) != 0)
    {
        close(fd);
        return -1;
    }

    /* Okay! */
    close(fd);
    return 0;
}

/*
**==============================================================================
**
** StartLocalAuthentication()
**
**     Initiate first phase of local authentication.
**
**==============================================================================
*/

int StartLocalAuthentication(
    const char* user,
    char challenge[EXECUTOR_BUFFER_SIZE])
{
    /* Get uid: */

    int uid;
    int gid;

    if (GetUserInfo(user, &uid, &gid) != 0)
        return -1;

    /* Create the local authentication file. */

    if (CreateLocalAuthFile(user, challenge) != 0)
        return -1;

    return 0;
}

/*
**==============================================================================
**
** FinishLocalAuthentication()
**
**     Initiate second and last phase of local authentication. Else return
**     negative one.
**
**==============================================================================
*/

int FinishLocalAuthentication(
    const char* challenge,
    const char* response)
{
    /* Check token against the one in the file. */

    int rc = CheckLocalAuthToken(challenge, response);

    if (challenge)
        unlink((char*)challenge);

    return rc;
}
