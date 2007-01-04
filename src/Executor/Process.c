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
#include <string.h>
#include "Process.h"
#include "Strlcpy.h"

#if defined(PEGASUS_OS_HPUX)
# include <sys/pstat.h>
#endif

/*
**==============================================================================
**
** GetProcessName()
**
**==============================================================================
*/

#if defined(PEGASUS_OS_HPUX)

int GetProcessName(int pid, char name[EXECUTOR_BUFFER_SIZE])
{
    struct pst_status psts;

    if (pstat_getproc(&psts, sizeof(psts), 0, pid) == -1)
        return -1;

    Strlcpy(name, pstru.pst_ucomm, EXECUTOR_BUFFER_SIZE);

    return 0;
}

#elif defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)

int GetProcessName(int pid, char name[EXECUTOR_BUFFER_SIZE])
{
    FILE* is;
    char* start;
    char* end;

    /* Read the process name from the file. */

    static char buffer[1024];
    sprintf(buffer, "/proc/%d/stat", pid);

    if ((is = fopen(buffer, "r")) == NULL)
        return -1;

    /* Read the first line of the file. */

    if (fgets(buffer, sizeof(buffer), is) == NULL)
    {
        fclose(is);
        return -1;
    }

    fclose(is);

    /* Extract the PID enclosed in parentheses. */

    start = strchr(buffer, '(');

    if (!start)
        return -1;

    start++;

    end = strchr(start, ')');

    if (!end)
        return -1;

    if (start == end)
        return -1;

    *end = '\0';

    Strlcpy(name, start, EXECUTOR_BUFFER_SIZE);

    return 0;
}

#else
# error "not implemented on this platform."
#endif /* PEGASUS_PLATFORM_LINUX_GENERIC_GNU */
