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
//==============================================================================
//
//%/////////////////////////////////////////////////////////////////////////////
*/

#ifndef Executor_PAMAuth_h
#define Executor_PAMAuth_h

#if !defined(PEGASUS_PAM_AUTHENTICATION)
# error "Do not include this file without defining PEGASUS_PAM_AUTHENTICATION"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <Executor/Strlcpy.h>
#include <Executor/Strlcat.h>
#include <security/pam_appl.h>
#include "Defines.h"

/*
**==============================================================================
**
**     This program is used to authenticate users with the "Basic PAM 
**     Authentication" scheme. It was originally written to isolate memory
**     PAM module errors to an external process.
**
**     This header defines two functions that may be called by clients of this
**     process (the parent process).
**
**         CimserveraAuthenticate()
**         CimserveraValidateUser()
**
**     Each functions forks and executes a child process that carries out
**     the request and then exits immediately. The parent and child proceses
**     communicate over a local domain socket, created by the parent just
**     before executing the client program.
**
**     Both of the functions above are defined in the header to avoid the need
**     to link a separate client library.
**
**     CAUTION: This program must not depend on any Pegasus libraries since
**     it is used by the executor process.
**
**==============================================================================
*/

/*
**==============================================================================
**
** CimserveraSend()
**
**     Sends *size* bytes on the given socket.
**
**==============================================================================
*/

static ssize_t CimserveraSend(int sock, void* buffer, size_t size)
{
    size_t r = size;
    char* p = (char*)buffer;

    while (r)
    {
        ssize_t n;
        EXECUTOR_RESTART(write(sock, p, r), n);

        if (n == -1)
            return -1;
        else if (n == 0)
            return size - r;

        r -= n;
        p += n;
    }

    return size - r;
}

/*
**==============================================================================
**
** CimserveraStart()
**
**     Starts the CIMSERVERA program, returning a socket used to communicate
**     with it.
**
**==============================================================================
*/

static int CimserveraStart(int* sock)
{
    int pair[2];
    int pid;

    /* Get absolute path of CIMSERVERA program. */

    char path[EXECUTOR_BUFFER_SIZE];

    if (PEGASUS_PAM_STANDALONE_PROC_NAME[0] == '/')
        Strlcpy(path, PEGASUS_PAM_STANDALONE_PROC_NAME, EXECUTOR_BUFFER_SIZE);
    else
    {
        /* Flawfinder: ignore */
        const char* home = getenv("PEGASUS_HOME");

        if (!home)
            return -1;

        Strlcpy(path, home, EXECUTOR_BUFFER_SIZE);
        Strlcat(path, "/", EXECUTOR_BUFFER_SIZE);
        Strlcat(path, PEGASUS_PAM_STANDALONE_PROC_NAME, EXECUTOR_BUFFER_SIZE);
    }

    /* Create socket pair for communicating with child process. */

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0)
        return -1;

    /* Fork child: */

    pid = fork();

    if (pid < 0)
        return -1;

    /* Child process: */

    if (pid == 0)
    {
        char sockStr[32];
        const char* argv[3];

        close(pair[1]);

        /* Convert socket number to string. */

        sprintf(sockStr, "%d", pair[0]);

        /* Build arguments for execv(). */

        argv[0] = CIMSERVERA;
        argv[1] = sockStr;
        argv[2] = 0;

        /* Execute child: */

        /* Flawfinder: ignore */
        execv(path, (char**)argv);
        close(pair[0]);
        _exit(0);
    }

    /* Parent process: */

    close(pair[0]);

    *sock = pair[1];
    return pid;
}

/*
**==============================================================================
**
** CimserveraRequest
**
**==============================================================================
*/

typedef struct CimserveraRequestStruct
{
    char arg0[EXECUTOR_BUFFER_SIZE];
    char arg1[EXECUTOR_BUFFER_SIZE];
    char arg2[EXECUTOR_BUFFER_SIZE];
}
CimserveraRequest;

/*
**==============================================================================
**
** CimserveraAuthenticate()
**
**==============================================================================
*/

static int CimserveraAuthenticate(const char* username, const char* password)
{
    int sock;
    int pid;
    int status;

    /* Create the CIMSERVERA process. */

    pid = CimserveraStart(&sock);

    if (pid == -1)
        return -1;

    /* Send request, get response. */

    status = 0;

    do
    {
        CimserveraRequest request;
        int childStatus;

        /* Send request to CIMSERVERA process. */

        memset(&request, 0, sizeof(request));
        Strlcpy(request.arg0, "authenticate", EXECUTOR_BUFFER_SIZE);
        Strlcpy(request.arg1, username, EXECUTOR_BUFFER_SIZE);
        Strlcpy(request.arg2, password, EXECUTOR_BUFFER_SIZE);

        if (CimserveraSend(sock, &request, sizeof(request)) != sizeof(request))
        {
            status = -1;
            break;
        } 

        /* Get exit status from CIMSERVERA program. */

        waitpid(pid, &childStatus, 0);

        if (!WIFEXITED(childStatus) || WEXITSTATUS(childStatus) != 0)
        {
            status = -1;
            break;
        }
    }
    while (0);

    close(sock);

    return status;
}

/*
**==============================================================================
**
** CimserveraAuthenticate()
**
**==============================================================================
*/

static int CimserveraValidateUser(const char* username)
{
    int sock;
    int pid;
    int status;

    /* Create the CIMSERVERA process. */

    pid = CimserveraStart(&sock);

    if (pid == -1)
        return -1;

    /* Send request, get response. */

    status = 0;

    do
    {
        CimserveraRequest request;
        int childStatus;

        /* Send request to CIMSERVERA process. */

        memset(&request, 0, sizeof(request));
        Strlcpy(request.arg0, "validateUser", EXECUTOR_BUFFER_SIZE);
        Strlcpy(request.arg1, username, EXECUTOR_BUFFER_SIZE);

        if (CimserveraSend(sock, &request, sizeof(request)) != sizeof(request))
        {
            status = -1;
            break;
        }

        /* Get exit status from CIMSERVERA program. */

        waitpid(pid, &childStatus, 0);

        if (!WIFEXITED(childStatus) || WEXITSTATUS(childStatus) != 0)
        {
            status = -1;
            break;
        }
    }
    while (0);

    close(sock);

    return status;
}

/*
**==============================================================================
**
** struct PAMData
**
**     Client data passed to PAM routines.
**
**==============================================================================
*/

typedef struct PAMDataStruct
{
    const char* password;
}
PAMData;

/*
**==============================================================================
**
** PAMAuthenticateCallback()
**
**     Callback used by PAMAuthenticate().
**
**==============================================================================
*/

static int PAMAuthenticateCallback(
    int num_msg, 
#if defined(PEGASUS_OS_LINUX)
    const struct pam_message** msg,
#else
    struct pam_message** msg,
#endif
    struct pam_response** resp, 
    void* appdata_ptr)
{
    PAMData* data = (PAMData*)appdata_ptr;
    int i;

    if (num_msg > 0)
    {
        *resp = (struct pam_response*)calloc(
            num_msg, sizeof(struct pam_response));

        if (*resp == NULL) 
            return PAM_BUF_ERR;
    }
    else 
        return PAM_CONV_ERR;

    for (i = 0; i < num_msg; i++) 
    {
        switch (msg[i]->msg_style) 
        {
            case PAM_PROMPT_ECHO_OFF:
            {
                resp[i]->resp = (char*)malloc(PAM_MAX_MSG_SIZE);
                Strlcpy(resp[i]->resp, data->password, PAM_MAX_MSG_SIZE);
                resp[i]->resp_retcode = 0;
                break;
            }

            default:
                return PAM_CONV_ERR;
        }
    }

    return PAM_SUCCESS;
}

/*
**==============================================================================
**
** PAMValidateUserCallback()
**
**     Callback used by PAMValidateUser().
**
**==============================================================================
*/

static int PAMValidateUserCallback(
    int num_msg, 
#if defined(PEGASUS_OS_LINUX)
    const struct pam_message** msg,
#else
    struct pam_message** msg,
#endif
    struct pam_response** resp, 
    void* appdata_ptr)
{
    /* Unused */
    msg = 0;

    /* Unused */
    appdata_ptr = 0;

    if (num_msg > 0)
    {
        *resp = (struct pam_response*)calloc(
            num_msg, sizeof(struct pam_response));

        if (*resp == NULL)
            return PAM_BUF_ERR;
    }
    else
        return PAM_CONV_ERR;

    return PAM_SUCCESS;
}

/*
**==============================================================================
**
** PAMAuthenticateInProcess()
**
**     Peforms basic PAM authentication on the given username and password.
**
**==============================================================================
*/

static int PAMAuthenticateInProcess(
    const char* username, const char* password)
{
    PAMData data;
    struct pam_conv pconv;
    pam_handle_t* handle;

    data.password = password;
    pconv.conv = PAMAuthenticateCallback;
    pconv.appdata_ptr = &data;


    if (pam_start("wbem", username, &pconv, &handle) != PAM_SUCCESS)
        return -1;

    if (pam_authenticate(handle, 0) != PAM_SUCCESS) 
    {
        pam_end(handle, 0);
        return -1;
    }

    if (pam_acct_mgmt(handle, 0) != PAM_SUCCESS) 
    {
        pam_end(handle, 0);
        return -1;
    }

    pam_end(handle, 0);

    return 0;
}

/*
**==============================================================================
**
** PAMValidateUserInProcess()
**
**     Validate that the *username* refers to a valid PAM user.
**
**==============================================================================
*/

static int PAMValidateUserInProcess(const char* username)
{
    PAMData data;
    struct pam_conv pconv;
    pam_handle_t* phandle;

    pconv.conv = PAMValidateUserCallback;
    pconv.appdata_ptr = &data;

    if (pam_start("wbem", username, &pconv, &phandle) != PAM_SUCCESS)
        return -1;

    if (pam_acct_mgmt(phandle, 0) != PAM_SUCCESS)
    {
        pam_end(phandle, 0);
        return -1;
    }

    pam_end(phandle, 0);

    return 0;
}

/*
**==============================================================================
**
** PAMAuthenticate()
**
**     Peforms basic PAM authentication on the given username and password.
**
**==============================================================================
*/

static int PAMAuthenticate(const char* username, const char* password)
{
#ifdef PEGASUS_USE_PAM_STANDALONE_PROC
    return CimserveraAuthenticate(username, password);
#else
    return PAMAuthenticateInProcess(username, password);
#endif
}

/*
**==============================================================================
**
** PAMValidateUser()
**
**     Validate that the *username* refers to a valid PAM user.
**
**==============================================================================
*/

static int PAMValidateUser(const char* username)
{
#ifdef PEGASUS_USE_PAM_STANDALONE_PROC
    return CimserveraValidateUser(username);
#else
    return PAMValidateUserInProcess(username);
#endif
}

#endif /* Executor_PAMAuth_h */
