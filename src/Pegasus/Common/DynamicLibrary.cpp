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

#include <Pegasus/Common/PegasusAssert.h>
#include "DynamicLibrary.h"

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include "DynamicLibraryWindows.cpp"
#elif defined(PEGASUS_OS_HPUX) && !defined(PEGASUS_HPUX_USE_DLOPEN)
# include "DynamicLibraryHPUX.cpp"
#elif defined(PEGASUS_OS_TYPE_UNIX) || \
    defined(PEGASUS_OS_VMS) || \
    defined(PEGASUS_OS_VXWORKS)
# include "DynamicLibraryPOSIX.cpp"
#else
# error "Unsupported platform"
#endif

PEGASUS_NAMESPACE_BEGIN

DynamicLibrary::DynamicLibrary()
    : _handle(0),
      _referenceCount(0)
{
}

DynamicLibrary::DynamicLibrary(const DynamicLibrary& library)
    : _fileName(library._fileName),
      _handle(0),
      _referenceCount(0)
{
    // load the module again, if necessary. this effectively increments the
    // operating system's reference count for the module.
    if (library.isLoaded())
    {
        if (load())
        {
            _referenceCount = library._referenceCount;
        }
    }
}

DynamicLibrary::DynamicLibrary(const String& fileName)
    : _fileName(fileName),
      _handle(0),
      _referenceCount(0)
{
}

DynamicLibrary::~DynamicLibrary()
{
    // Unload the module, if necessary, to keep the operating system's
    // reference count accurate.  One call to _unload() takes care of it
    // no matter how high _referenceCount is.

    if (_referenceCount > 0)
    {
        PEGASUS_ASSERT(_handle != 0);
        _unload();
    }
}

DynamicLibrary& DynamicLibrary::operator=(const DynamicLibrary& library)
{
    if (this == &library)
    {
        return *this;
    }

    while (isLoaded())
    {
        unload();
    }

    _fileName = library._fileName;

    // load the module again, if necessary. this effectively increments the
    // operating system's reference count for the module.
    if (library.isLoaded())
    {
        if (load())
        {
            _referenceCount = library._referenceCount;
        }
    }

    return *this;
}

Boolean DynamicLibrary::isLoaded() const
{
    return _handle != 0;
}

Boolean DynamicLibrary::load()
{
    AutoMutex lock(_loadMutex);

    Boolean loaded = true;

    if (_referenceCount == 0)
    {
        PEGASUS_ASSERT(_handle == 0);
        loaded = _load();
    }

    if (loaded)
    {
        PEGASUS_ASSERT(_handle != 0);
        _referenceCount++;
    }

    return loaded;
}

const String& DynamicLibrary::getLoadErrorMessage() const
{
    return _loadErrorMessage;
}

void DynamicLibrary::unload()
{
    AutoMutex lock(_loadMutex);

    PEGASUS_ASSERT(_referenceCount > 0);
    PEGASUS_ASSERT(_handle != 0);

    _referenceCount--;

    if (_referenceCount == 0)
    {
        _unload();
        _handle = 0;
        _loadErrorMessage.clear();
    }
}

const String& DynamicLibrary::getFileName() const
{
    return _fileName;
}

PEGASUS_NAMESPACE_END
