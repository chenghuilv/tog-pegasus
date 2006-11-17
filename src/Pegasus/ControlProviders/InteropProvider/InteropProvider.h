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


#ifndef InteropProvider_h
#define InteropProvider_h

///////////////////////////////////////////////////////////////////////////////
//  Interop Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/ControlProviders/InteropProvider/Linkage.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>

PEGASUS_NAMESPACE_BEGIN

/**
 * The InteropProvider services the Interop classes of the DMTF CIM Schema
 * in the root/PG_InterOp namespace (as well as some related cross-namespace
 * associations in other namespaces). Through this implementation, combined
 * with the SLP provider and one or more vendor-supplied SMI providers, the
 * Pegasus WBEM Server is able to provide a fully-functional implementation of
 * the SMI-S Server profile (currently, version 1.1.0).
 *
 * The following is a list of the association and instance classes supported
 * by this provider in the root/PG_InterOp namespace:
 *
 *  PG_CIMXMLCommunicationMechanism (CIM_CIMXMLCommunicationMechanism)
 *  PG_CommMechanismForManager (CIM_CommMechanismForManager)
 *  PG_ComputerSystem (CIM_ComputerSystem)
 *  PG_ElementConformsToProfile (CIM_ElementConformsToProfile)
 *  PG_ElementSoftwareIdentity (CIM_ElementSoftwareIdentity)
 *  PG_HostedAccessPoint (CIM_HostedAccessPoint)
 *  PG_HostedObjectManager (CIM_HostedService)
 *  PG_InstalledSoftwareIdentity (CIM_InstalledSoftwareIdentity)
 *  PG_Namespace (CIM_Namespace)
 *  PG_NamespaceInManager (CIM_NamespaceInManager)
 *  PG_ObjectManager (CIM_ObjectManager)
 *  PG_ReferencedProfile (CIM_ReferencedProfile)
 *  PG_RegisteredProfile (CIM_RegisteredProfile)
 *  PG_RegisteredSubProfile (CIM_RegisteredSubProfile)
 *  PG_SoftwareIdentity (CIM_SoftwareIdentity)
 *  PG_SubProfileRequiredProfile (CIM_SubProfileRequiresProfile)
 *
 */

typedef Array<CIMName> CIMNameArray;
typedef Array<CIMNamespaceName> CIMNamespaceArray;

class PEGASUS_INTEROPPROVIDER_LINKAGE InteropProvider :
        public CIMInstanceProvider, public CIMAssociationProvider
{
public:

    InteropProvider(CIMRepository* repository);
    virtual ~InteropProvider()
    {
        PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::~InteropProvider");
        PEG_METHOD_EXIT();
    }

    // Note:  The initialize() and terminate() methods are not called for
    // Control Providers.
    void initialize(CIMOMHandle& handle) { }
    void terminate() { }

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& myInstance,
        ObjectPathResponseHandler & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath& instanceName,
        ResponseHandler & handler);

    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath& instanceName,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler & handler);

    void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler);


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

private:

    void initProvider();

    CIMInstance buildInstanceSkeleton(
        const CIMNamespaceName & nameSpace,
        const CIMName& className,
        CIMClass& returnedClass);

    CIMInstance buildCIMXMLCommunicationMechanismInstance(
        const String& namespaceType,
        const Uint16& accessProtocol,
        const String& IPAddress,
        const CIMClass & commMechClass);

    Array<CIMInstance> enumCIMXMLCommunicationMechanismInstances();

    Array<CIMInstance> enumHostedAccessPointInstances();

    CIMInstance getObjectManagerInstance();

    CIMInstance getComputerSystemInstance();

    CIMInstance getHostedObjectManagerInstance();

    Array<CIMInstance> enumNamespaceInstances();

    CIMInstance buildNamespaceInstance(const String & nameSpace);

    CIMObjectPath createNamespace(const CIMInstance & namespaceInstance);
    void deleteNamespace(const CIMObjectPath & instanceName);

    Array<CIMInstance> enumNamespaceInManagerInstances();

    Array<CIMInstance> enumCommMechanismForManagerInstances();

    void modifyObjectManagerInstance(const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList);

    void extractSoftwareIdentityInfo(
        const CIMInstance & providerInstance,
        String & moduleName,
        String & providerName,
        String & version,
        String & vendor,
        Uint16 & majorVersion,
        Uint16 & minorVersion,
        Uint16 & revision,
        Uint16 & build,
        bool & extendedVersionSupplied,
        String & interfaceType,
        String & elementName,
        String & caption);

    Array<CIMInstance> enumRegisteredProfileInstances();
    Array<CIMInstance> enumRegisteredSubProfileInstances();
    Array<CIMInstance> enumReferencedProfileInstances();
    Array<CIMInstance> enumElementConformsToProfileInstances(
        const OperationContext & opContext,
        const CIMNamespaceName & opNamespace);
    Array<CIMInstance> enumSubProfileRequiresProfileInstances();
    Array<CIMInstance> enumSoftwareIdentityInstances();
    Array<CIMInstance> enumElementSoftwareIdentityInstances();
    Array<CIMInstance> enumInstalledSoftwareIdentityInstances();

    CIMInstance buildRegisteredProfile(
        const String & instanceId,
        const String & profileName,
        const String & profileVersion,
        Uint16 profileOrganization,
        const String & otherProfileOrganization,
        const CIMClass & profileClass);

    CIMInstance buildDependencyInstance(
        const String & antecedentId,
        const CIMName & antecedentClass,
        const String & dependentId,
        const CIMName & dependentClass,
        const CIMClass & dependencyClass);

    CIMInstance buildSoftwareIdentity(
        const String & module,
        const String & provider,
        const String & vendor,
        const String & version,
        Uint16 majorVersion,
        Uint16 minorVersion,
        Uint16 revisionNumber,
        Uint16 buildNumber,
        bool extendedVersionSupplied,
        const String & interfaceType,
        const String & elementName,
        const String & caption);

    Array<CIMInstance> getProfileInstances(
        const CIMName & profileType,
        const Array<String> & defaultSniaProfiles);

    // The following are internal equivalents of the operations
    // allowing the operations to call one another internally within
    // the provider.
    Array<CIMInstance> localEnumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMPropertyList& propertyList=CIMPropertyList());

    Array<CIMInstance> localReferences(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        String & originRole,
        String & targetRole,
        const CIMPropertyList & propertyList=CIMPropertyList(),
        const CIMName & targetClass=CIMName());

    CIMInstance localGetInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceName,
        const CIMPropertyList & propertyList);

    void cacheProfileRegistrationInfo();
    void verifyCachedInfo();

    bool validAssocClassForObject(
        const CIMName & assocClass, const CIMName & originClass,
        const CIMNamespaceName & opNamespace,
        String & originProperty, String & targetProperty);

    // Repository Instance variable
    CIMOMHandle cimomHandle;
    CIMRepository * repository;
    String objectManagerName;
    String hostName;
    CIMClass profileCapabilitiesClass;
    CIMClass softwareIdentityClass;
    Array<Uint16> providerClassifications;
    Mutex interopMut;
    bool providerInitialized;

    // Registration info to cache
    Array<String> profileIds;
    Array<CIMNameArray> conformingElements;
    Array<CIMNamespaceArray> elementNamespaces;
};

PEGASUS_NAMESPACE_END

#endif // InteropProvider_h
