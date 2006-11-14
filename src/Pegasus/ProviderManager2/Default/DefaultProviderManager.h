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

#ifndef Pegasus_DefaultProviderManager_h
#define Pegasus_DefaultProviderManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/OperationContextInternal.h>

#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/ProviderManager2/ProviderName.h>

#include <Pegasus/ProviderManager2/Default/ProviderMessageHandler.h>
#include <Pegasus/ProviderManager2/Default/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_DEFPM_LINKAGE DefaultProviderManager : public ProviderManager
{
public:
    DefaultProviderManager();
    virtual ~DefaultProviderManager();

    virtual Message* processMessage(Message* message);

    virtual Boolean hasActiveProviders();
    virtual void unloadIdleProviders();

    // This function creates an instance of DefaultProviderManager. It is
    // typically passed to either the ProviderManagerService constructor
    // or the BasicProviderManagerRouter constructor as a way of decoupling
    // the pegprovidermanager and DefaultProviderManager libraries (otherwise
    // each library would contain a reference to the other).
    static ProviderManager* createDefaultProviderManagerCallback();

private:
    CIMResponseMessage* _handleDisableModuleRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleEnableModuleRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleSubscriptionInitCompleteRequest(
        CIMRequestMessage* message);

    ProviderName _resolveProviderName(const ProviderIdContainer& providerId);

    ProviderOperationCounter _getProvider(
        const String& moduleFileName,
        const String& providerName);

    ProviderMessageHandler* _lookupProvider(const String& providerName);

    ProviderMessageHandler* _initProvider(
        ProviderMessageHandler* provider,
        const String& moduleFileName);

    ProviderModule* _lookupModule(const String& moduleFileName);

    void _shutdownAllProviders();

    Sint16 _disableProvider(const String& providerName);

    void _unloadProvider(ProviderMessageHandler* provider);

    /**
        The _providerTableMutex must be locked whenever accessing the
        _providers table or the _modules table.  It is okay to lock a
        ProviderStatus::_statusMutex while holding the _providerTableMutex,
        but one should never lock the _providerTableMutex while holding
        a ProviderStatus::_statusMutex.
     */
    Mutex _providerTableMutex;

    typedef HashTable<String, ProviderMessageHandler*,
        EqualFunc<String>,  HashFunc<String> > ProviderTable;

    typedef HashTable<String, ProviderModule*,
        EqualFunc<String>, HashFunc<String> > ModuleTable;

    ProviderTable _providers;
    ModuleTable _modules;
};

PEGASUS_NAMESPACE_END

#endif
