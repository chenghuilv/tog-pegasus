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

#include "ProviderManager.h"
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Config/ConfigManager.h>

PEGASUS_NAMESPACE_BEGIN

ProviderManager::ProviderManager()
{
}

ProviderManager::~ProviderManager()
{
}

String ProviderManager::_resolvePhysicalName(String physicalName)
{
#if defined(PEGASUS_OS_VXWORKS)
    // VxWorks uses static linking so we just return libraries of the
    // form "lib<physicalName>.a".
    return String("lib") + physicalName + String(".a");
#endif

#if defined(PEGASUS_OS_VMS)
    String fileName =
        FileSystem::buildLibraryFileName(physicalName) + String(".exe");
#else
    String fileName = FileSystem::buildLibraryFileName(physicalName);
#endif

    fileName = FileSystem::getAbsoluteFileName(
        ConfigManager::getHomedPath(
            ConfigManager::getInstance()->getCurrentValue("providerDir")),
        fileName);

    return fileName;
}

void ProviderManager::setIndicationCallback(
        PEGASUS_INDICATION_CALLBACK_T indicationCallback)
{
    _indicationCallback = indicationCallback;
}

void ProviderManager::setResponseChunkCallback(
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
{
    _responseChunkCallback = responseChunkCallback;
}

void ProviderManager::setSubscriptionInitComplete
    (Boolean subscriptionInitComplete)
{
    _subscriptionInitComplete = subscriptionInitComplete;
}

PEGASUS_NAMESPACE_END
