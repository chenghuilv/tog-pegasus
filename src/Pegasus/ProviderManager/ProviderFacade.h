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
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//              Mike Day, IBM (mdday@us.ibm.com)
//              Yi Zhou, Hewlett-Packard Company (yi_zhou@hp.com)
//              Amit K Arora, (amita@in.ibm.com) for PEP101
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ProviderFacade_h
#define Pegasus_ProviderFacade_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/IPC.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Provider/CIMQueryProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>
#include <Pegasus/Provider/CIMIndicationConsumerProvider.h>
#include <Pegasus/ProviderManager/SimpleResponseHandler.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN


// The ProviderModule class wraps a provider pointer extracted from a provider
// module to ensure it is complete and well behaved. So, regardless of the
// method supported by a "real" provider, it can be placed inside a reliable
// facade with a known interface.
class PEGASUS_SERVER_LINKAGE ProviderFacade :
    public CIMInstanceProvider,
    public CIMAssociationProvider,
    public CIMMethodProvider,
    public CIMQueryProvider,
    public CIMIndicationProvider,
    public CIMIndicationConsumerProvider
{
public:
    ProviderFacade(CIMProvider * provider);
    virtual ~ProviderFacade(void);

    // CIMProvider interface
    virtual void initialize(CIMOMHandle & cimom);
#ifdef PEGASUS_PRESERVE_TRYTERMINATE
    virtual Boolean tryTerminate(void);
#endif
    
    virtual void terminate(void);

    // CIMInstanceProvider interface
    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler);

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler);

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        ObjectPathResponseHandler & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        ResponseHandler & handler);

    // CIMAssociationProvider interface
    virtual void associators(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void associatorNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        ObjectPathResponseHandler & handler);

    virtual void references(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void referenceNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        ObjectPathResponseHandler & handler);

    // Property operation interfaces
    // Note: Property operations are not supported at the provider level.
    // These methods should be removed, and the caller should be changed to
    // use the CIMInstanceProvider interface.
    virtual void getProperty(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMName & propertyName,
        ValueResponseHandler & handler);

    virtual void setProperty(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMName & propertyName,
        const CIMValue & newValue,
        ResponseHandler & handler);

    // CIMMethodProviderFacade
    virtual void invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & objectReference,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParameters,
        MethodResultResponseHandler & handler);

    // CIMQueryProvider interface
    virtual void executeQuery(
        const OperationContext & context,
        const CIMNamespaceName & nameSpace,
        const String & queryLanguage,
        const String & query,
        ObjectResponseHandler & handler);

    // CIMIndicationProvider interface
    virtual void enableIndications(
        IndicationResponseHandler & handler);

    virtual void disableIndications(void);

    virtual void createSubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames,
        const CIMPropertyList & propertyList,
        const Uint16 repeatNotificationPolicy);

    virtual void modifySubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames,
        const CIMPropertyList & propertyList,
        const Uint16 repeatNotificationPolicy);

    virtual void deleteSubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames);

    // CIMIndicationConsumerProvider interface
    virtual void consumeIndication(
        const OperationContext & context,
        const String & destinationPath,
        const CIMInstance& indicationInstance);
    
protected:
    AutoPtr<CIMProvider> _provider; //PEP101
    AtomicInt _current_operations;
    Boolean _indications_enabled;
};


PEGASUS_NAMESPACE_END

#endif
