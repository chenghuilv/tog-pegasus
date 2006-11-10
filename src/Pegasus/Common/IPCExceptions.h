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

#ifndef Pegasus_IPCExceptions_h
#define Pegasus_IPCExceptions_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Threads.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE IPCException
{
public:
    IPCException(ThreadType owner): _owner(owner) { }
    inline ThreadType get_owner() { return _owner; }
private:
    ThreadType _owner;
};

class PEGASUS_COMMON_LINKAGE Deadlock: public IPCException
{
public:
    Deadlock(ThreadType owner) : IPCException(owner) {}
private:
    Deadlock();
};

class PEGASUS_COMMON_LINKAGE AlreadyLocked: public IPCException
{
public:
    AlreadyLocked(ThreadType owner) : IPCException(owner) {}
private:
    AlreadyLocked();
};

class PEGASUS_COMMON_LINKAGE TimeOut: public IPCException
{
public:
    TimeOut(ThreadType owner) : IPCException(owner) {}
private:
    TimeOut();
};

class PEGASUS_COMMON_LINKAGE Permission: public IPCException
{
public:
    Permission(ThreadType owner) : IPCException(owner) {}
private:
    Permission();
};

class PEGASUS_COMMON_LINKAGE WaitFailed: public IPCException
{
public:
    WaitFailed(ThreadType owner) : IPCException(owner) {}
private:
    WaitFailed();
};

class PEGASUS_COMMON_LINKAGE WaitInterrupted: public IPCException
{
public:
    WaitInterrupted(ThreadType owner) : IPCException(owner) {}
private:
    WaitInterrupted();
};

class PEGASUS_COMMON_LINKAGE TooManyReaders: public IPCException
{
public:
    TooManyReaders(ThreadType owner) : IPCException(owner) { }
private:
    TooManyReaders();
};


class PEGASUS_COMMON_LINKAGE ListFull: public IPCException
{
public:
    ListFull(Uint32 count) : IPCException(Threads::self())
    {
        _capacity = count;
    }

    Uint32 get_capacity() const throw()
    {
        return _capacity;
    }

private:
    ListFull();
    Uint32 _capacity;
};

class PEGASUS_COMMON_LINKAGE ListClosed: public IPCException
{
public:
    ListClosed() : IPCException(Threads::self())
    {
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_IPCExceptions_h */
