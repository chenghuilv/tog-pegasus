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

#ifndef _Executor_Executor_h
#define _Executor_Executor_h

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define EXECUTOR_BUFFER_SIZE 4096

#define EXECUTOR_RESTART(F, X) while (((X = (F)) == -1) && (errno == EINTR))

//==============================================================================
//
// RequestCode
//
//==============================================================================

enum RequestCode
{
    EXECUTOR_PING_REQUEST = 1,
    EXECUTOR_OPEN_FILE_REQUEST,
    EXECUTOR_START_PROVIDER_AGENT_REQUEST,
    EXECUTOR_DAEMONIZE_EXECUTOR_REQUEST,
    EXECUTOR_CHANGE_OWNER_REQUEST,
    EXECUTOR_REMOVE_FILE_REQUEST,
    EXECUTOR_RENAME_FILE_REQUEST,
};

//==============================================================================
//
// struct ExecutorRequestHeader
//
//==============================================================================

struct ExecutorRequestHeader
{
    unsigned int code;
};

//==============================================================================
//
// EXECUTOR_PING_REQUEST
//
//==============================================================================

#define EXECUTOR_PING_MAGIC 0x9E5EACB6

struct ExecutorPingResponse
{
    unsigned int magic;
};

//==============================================================================
//
// EXECUTOR_OPEN_FILE_REQUEST
//
//==============================================================================

struct ExecutorOpenFileRequest
{
    char path[EXECUTOR_BUFFER_SIZE];
    // ('r' = read, 'w' = write)
    int mode;
};

struct ExecutorOpenFileResponse
{
    int status;
};

//==============================================================================
//
// EXECUTOR_REMOVE_FILE_REQUEST
//
//==============================================================================

struct ExecutorRemoveFileRequest
{
    char path[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorRemoveFileResponse
{
    int status;
};

//==============================================================================
//
// EXECUTOR_RENAME_FILE_REQUEST
//
//==============================================================================

struct ExecutorRenameFileRequest
{
    char oldPath[EXECUTOR_BUFFER_SIZE];
    char newPath[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorRenameFileResponse
{
    int status;
};

//==============================================================================
//
// EXECUTOR_START_PROVIDER_AGENT_REQUEST
//
//==============================================================================

struct ExecutorStartProviderAgentRequest
{
    char module[EXECUTOR_BUFFER_SIZE];
    int uid;
    int gid;
};

struct ExecutorStartProviderAgentResponse
{
    int status;
    int pid;
};

//==============================================================================
//
// EXECUTOR_DAEMONIZE_EXECUTOR_REQUEST
//
//==============================================================================

struct ExecutorDaemonizeExecutorResponse
{
    int status;
};

//==============================================================================
//
// EXECUTOR_CHANGE_OWNER_REQUEST
//
//==============================================================================

struct ExecutorChangeOwnerRequest
{
    char path[EXECUTOR_BUFFER_SIZE];
    char owner[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorChangeOwnerResponse
{
    int status;
};

//==============================================================================
//
// Strlcpy()
//
//     This is an original implementation of the strlcpy() function as described
//     by Todd C. Miller in his popular security paper entitled "strlcpy and 
//     strlcat - consistent, safe, string copy and concatenation".
//
//     Note that this implementation favors readability over efficiency. More
//     efficient implemetations are possible but would be to complicated
//     to verify in a security audit.
//
//==============================================================================

static size_t Strlcpy(char* dest, const char* src, size_t size)
{
    size_t i;

    for (i = 0; src[i] && i + 1 < size; i++)
        dest[i] = src[i];

    if (size > 0)
        dest[i] = '\0';

    while (src[i])
        i++;

    return i;
}

//==============================================================================
//
// Strlcat()
//
//     This is an original implementation of the strlcat() function as described
//     by Todd C. Miller in his popular security paper entitled "strlcpy and 
//     strlcat - consistent, safe, string copy and concatenation".
//
//     Note that this implementation favors readability over efficiency. More
//     efficient implemetations are possible but would be to complicated
//     to verify in a security audit.
//
//==============================================================================

static size_t Strlcat(char* dest, const char* src, size_t size)
{
    size_t i;
    size_t j;

    // Find dest null terminator.

    for (i = 0; i < size && dest[i]; i++)
        ;

    // If no-null terminator found, return size.

    if (i == size)
        return size;

    // Copy src characters to dest.

    for (j = 0; src[j] && i + 1 < size; i++, j++)
        dest[i] = src[j];

    // Null terminate size non-zero.

    if (size > 0)
        dest[i] = '\0';

    while (src[j])
    {
        j++;
        i++;
    }

    return i;
}

#endif /* _Executor_Executor_h */
