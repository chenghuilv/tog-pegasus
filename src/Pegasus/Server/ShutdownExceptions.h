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


////////////////////////////////////////////////////////////////////////////////
//  This file contains the exception classes used in the Shutdown Service.
////////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ShutdownExceptions_h
#define Pegasus_ShutdownExceptions_h

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    InvalidOperationTimeoutException class
*/
class PEGASUS_SERVER_LINKAGE InvalidOperationTimeoutException: public Exception
{
public:
    InvalidOperationTimeoutException()
        : Exception(MessageLoaderParms(
              "Server.ShutdownExceptions.INVALID_TIMEOUT",
              "Invalid operation timeout value."))
    {
    }
};

/**
    UnableToResumeServerException class
*/
class PEGASUS_SERVER_LINKAGE UnableToResumeServerException: public Exception
{
public:
    UnableToResumeServerException()
        : Exception(MessageLoaderParms(
              "Server.ShutdownExceptions.UNABLE_TO_RESUME",
              "Unable to resume CIM server."))
    {
    }
};

/**
    ServerResumedException class
*/
class PEGASUS_SERVER_LINKAGE ServerResumedException: public Exception
{
public:
    ServerResumedException()
        : Exception(MessageLoaderParms(
              "Server.ShutdownExceptions.TIMEOUT_EXPIRED",
              "Timeout has expired.  Server has resumed operation."))
    {
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ShutdownExceptions_h */
