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

#include "OperationContextInternal.h"

#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
    defined(PEGASUS_PLATFORM_DARWIN_PPC_GNU) || \
    defined(PEGASUS_PLATFORM_DARWIN_IX86_GNU)
# define PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
#endif

PEGASUS_NAMESPACE_BEGIN

//
// LocaleContainer
//

const String LocaleContainer::NAME = "LocaleContainer";

LocaleContainer::LocaleContainer(const OperationContext::Container& container)
{
    const LocaleContainer* p = dynamic_cast<const LocaleContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}

LocaleContainer::LocaleContainer(const String& languageId)
{
    _languageId = languageId;
}

LocaleContainer::~LocaleContainer()
{
}

LocaleContainer& LocaleContainer::operator=(const LocaleContainer&container)
{
    if (this == &container)
    {
        return *this;
    }

    _languageId = container._languageId;

    return *this;
}

String LocaleContainer::getName() const
{
    return NAME;
}

OperationContext::Container* LocaleContainer::clone() const
{
    return new LocaleContainer(*this);
}

void LocaleContainer::destroy()
{
    delete this;
}

String LocaleContainer::getLanguageId() const
{
    return _languageId;
}

//
// ProviderIdContainer
//

const String ProviderIdContainer::NAME = "ProviderIdContainer";

ProviderIdContainer::ProviderIdContainer(
    const OperationContext::Container& container)
{
    const ProviderIdContainer* p =
        dynamic_cast<const ProviderIdContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}

ProviderIdContainer::ProviderIdContainer(
    const CIMInstance& module,
    const CIMInstance& provider,
    Boolean isRemoteNameSpace,
    const String& remoteInfo)
    : _module(module),
    _provider(provider),
    _isRemoteNameSpace(isRemoteNameSpace),
    _remoteInfo(remoteInfo)
{
}

ProviderIdContainer::~ProviderIdContainer()
{
}

ProviderIdContainer& ProviderIdContainer::operator=(
    const ProviderIdContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _module = container._module;
    _provider = container._provider;
    _isRemoteNameSpace = container._isRemoteNameSpace;
    _remoteInfo = container._remoteInfo;

    return *this;
}

String ProviderIdContainer::getName() const
{
    return NAME;
}

OperationContext::Container* ProviderIdContainer::clone() const
{
    return new ProviderIdContainer(*this);
}

void ProviderIdContainer::destroy()
{
    delete this;
}

CIMInstance ProviderIdContainer::getModule() const
{
    return _module;
}

CIMInstance ProviderIdContainer::getProvider() const
{
    return _provider;
}

Boolean ProviderIdContainer::isRemoteNameSpace() const
{
    return _isRemoteNameSpace;
}

String ProviderIdContainer::getRemoteInfo() const
{
    return _remoteInfo;
}

//
// CachedClassDefinitionContainer
//

const String CachedClassDefinitionContainer::NAME =
    "CachedClassDefinitionContainer";

CachedClassDefinitionContainer::CachedClassDefinitionContainer(
    const OperationContext::Container& container)
{
    const CachedClassDefinitionContainer* p =
        dynamic_cast<const CachedClassDefinitionContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}


CachedClassDefinitionContainer::CachedClassDefinitionContainer(
    const CIMClass& cimClass)
    : _cimClass(cimClass)
{
}

CachedClassDefinitionContainer::~CachedClassDefinitionContainer()
{
}

CachedClassDefinitionContainer& CachedClassDefinitionContainer::operator=(
    const CachedClassDefinitionContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _cimClass = container._cimClass;

    return *this;
}

String CachedClassDefinitionContainer::getName() const
{
    return NAME;
}

OperationContext::Container* CachedClassDefinitionContainer::clone() const
{
    return new CachedClassDefinitionContainer(*this);
}

void CachedClassDefinitionContainer::destroy()
{
    delete this;
}

CIMClass CachedClassDefinitionContainer::getClass() const
{
    return _cimClass;
}

//
// NormalizerContextContainer
//

const String NormalizerContextContainer::NAME = "NormalizerContextContainer";

NormalizerContextContainer::NormalizerContextContainer(
    const OperationContext::Container& container)
{
    const NormalizerContextContainer* p =
        dynamic_cast<const NormalizerContextContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}


NormalizerContextContainer::NormalizerContextContainer(
    AutoPtr<NormalizerContext>& context) : normalizerContext(context.get())
{
  context.release();
}

NormalizerContextContainer::NormalizerContextContainer(
    const NormalizerContextContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    if (this != &container)
    {
        normalizerContext.reset(container.normalizerContext->clone());
    }
}

NormalizerContextContainer::~NormalizerContextContainer()
{
}

NormalizerContextContainer& NormalizerContextContainer::operator=(
    const NormalizerContextContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    normalizerContext.reset(container.normalizerContext->clone());

    return *this;
}

String NormalizerContextContainer::getName() const
{
    return NAME;
}

OperationContext::Container* NormalizerContextContainer::clone() const
{
    return new NormalizerContextContainer(*this);
}

void NormalizerContextContainer::destroy()
{
    delete this;
}

NormalizerContext* NormalizerContextContainer::getContext() const
{
    return normalizerContext.get();
}

PEGASUS_NAMESPACE_END
