//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Yi Zhou, Hewlett-Packard Company(yi_zhou@hp.com)
//              Mike Day, IBM (mdday@us.ibm.com)
//              Dan Gorey, IBM djgorey@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Provider_h
#define Pegasus_Provider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/IPC.h>

#include <Pegasus/Provider/CIMOMHandle.h>

#include <Pegasus/ProviderManager2/Default/ProviderModule.h>
#include <Pegasus/ProviderManager2/Default/ProviderFacade.h>

#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

// The Provider class represents the logical provider extracted from a
// provider module. It is wrapped in a facade to stabalize the interface
// and is directly tied to a module.

class PEGASUS_SERVER_LINKAGE Provider : public ProviderFacade
{
public:
    enum Status
    {
        UNINITIALIZED,
        INITIALIZED
    };

public:
    typedef ProviderFacade Base;

    Provider(const String & name,
        ProviderModule *module,
        CIMProvider *pr);

    virtual ~Provider(void);

    virtual void initialize(CIMOMHandle & cimom);
    virtual void terminate(void);

    Status getStatus(void);
    String getName(void) const;

    ProviderModule *getModule(void) const;

    // << Mon Oct 14 15:42:24 2002 mdd >> for use with DQueue template
    // to allow conversion from using Array<>
    Boolean operator == (const void *key) const;
    Boolean operator == (const Provider & prov) const;

    virtual void get_idle_timer(struct timeval *);
    virtual void update_idle_timer(void);
    virtual Boolean pending_operation(void);
    virtual Boolean unload_ok(void);

    //   force provider manager to keep in memory
    virtual void protect(void);
    // allow provider manager to unload when idle
    virtual void unprotect(void);

    void set(ProviderModule *module,
            CIMProvider *base,
            CIMOMHandle *cimomHandle);

    void reset();

protected:
    Status _status;
    ProviderModule *_module;
    CIMProvider *getCIMProvider();
private:
    friend class LocalProviderManager;
    friend class ProviderManagerService;
    friend class OpProviderHolder;
    CIMOMHandle *_cimom_handle;
    String _name;
    AtomicInt _no_unload;
    Uint32 _quantum;
    Mutex _statusMutex;
};


//
// Used to encapsulate the incrementing/decrementing of the _current_operations
// for a Provider so it won't be unloaded during operations.
//

class OpProviderHolder
{
private:
    Provider* _provider;

public:
    OpProviderHolder(): _provider( NULL )
    {
    }
    OpProviderHolder( const OpProviderHolder& p ): _provider( NULL )
    {
        SetProvider( p._provider );
    }
    OpProviderHolder( Provider* p ): _provider( NULL )
    {
        SetProvider( p );
    }
    ~OpProviderHolder()
    {
        UnSetProvider();
    }

    Provider& GetProvider()
    {
        return(*_provider);
    }

    CIMProvider* GetCIMProvider()
    {
        return _provider->getCIMProvider();
    }

    OpProviderHolder& operator=( const OpProviderHolder& x )
    {
        if(this == &x)
            return(*this);
        SetProvider( x._provider );

        return(*this);
    }

    void SetProvider( Provider* p )
    {
        UnSetProvider();
        if(p)
        {
            _provider = p;
            _provider->_current_operations++;
        }
    }

    void UnSetProvider()
    {
        if(_provider)
        {
            _provider->_current_operations--;
            _provider = NULL;
        }
    }
};

PEGASUS_NAMESPACE_END

#endif
