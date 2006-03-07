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
// This example of how to call lib$initialize from C++ was taken 
// from the internal notes file: cxxc_bugs 11466.4
//
#include <unixlib.h>      // decc$feature_get_index(), set_value()
#include <stdio.h>        // perror()
#include <errno.h>        // errno

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*fptr_t)(void);

void LIB$INITIALIZE();

//
// Sets current value for a feature
//
static void set(char *name, int value)
{
  int index;
  errno = 0;

  index = decc$feature_get_index(name);

  if (index == -1 || 
      (decc$feature_set_value(index, 1, value) == -1 && 
       errno != 0))
  {
    perror(name);
  }
}

static void set_coe(void)
{
  set ("DECC$ARGV_PARSE_STYLE", TRUE);
  set ("DECC$ENABLE_GETENV_CACHE", TRUE);
  set ("DECC$FILE_SHARING", TRUE);
  set ("DECC$DISABLE_TO_VMS_LOGNAME_TRANSLATION", TRUE);
  set ("DECC$EFS_CASE_PRESERVE", TRUE);
  set ("DECC$EFS_CHARSET", TRUE);
  set ("DECC$EFS_FILE_TIMESTAMPS", TRUE);
  set ("DECC$FILENAME_UNIX_NO_VERSION", TRUE);
#if !defined FILENAME_UNIX_REPORT_FALSE
  set ("DECC$FILENAME_UNIX_REPORT", TRUE);
#else
  set ("DECC$FILENAME_UNIX_REPORT", FALSE);
#endif
  set ("DECC$FILE_OWNER_UNIX", TRUE);
  set ("DECC$FILE_PERMISSION_UNIX", TRUE);
  set ("DECC$READDIR_DROPDOTNOTYPE", TRUE);
  set ("DECC$FILENAME_UNIX_ONLY", FALSE);
  set ("DECC$UMASK", 027);
}


fptr_t x = LIB$INITIALIZE;

#pragma extern_model save

#pragma extern_model strict_refdef "LIB$INITIALIZD_" gbl,noexe,nowrt,noshr,long
  extern const fptr_t y = set_coe;

#pragma extern_model restore

#ifdef __cplusplus
} // extern "C"
#endif

