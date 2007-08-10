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


///////////////////////////////////////////////////////////////////////////////
//  Interop Provider - This provider services those classes from the
//  DMTF Interop schema in an implementation compliant with the SMI-S v1.1
//  Server Profile
//
//  Please see PG_ServerProfile20.mof in the directory
//  $(PEGASUS_ROOT)/Schemas/Pegasus/InterOp/VER20 for retails regarding the
//  classes supported by this control provider.
//
//  Interop forces all creates to the PEGASUS_NAMESPACENAME_INTEROP 
//  namespace. There is a test on each operation that returns 
//  the Invalid Class CIMDError
//  This is a control provider and as such uses the Tracer functions
//  for data and function traces.  Since we do not expect high volume
//  use we added a number of traces to help diagnostics.
///////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"
#include "Guid.h"

#include <Pegasus/Common/StatisticalData.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

const String CIMXMLProtocolVersion = "1.0";

// Property names for ObjectManager Class
//#define OM_PROPERTY_NAME COMMON_PROPERTY_NAME
#define OM_PROPERTY_ELEMENTNAME COMMON_PROPERTY_ELEMENTNAME
#define OM_PROPERTY_CREATIONCLASSNAME COMMON_PROPERTY_CREATIONCLASSNAME
//const CIMName OM_PROPERTY_GATHERSTATISTICALDATA("GatherStatisticalData");
const CIMName OM_PROPERTY_DESCRIPTION("Description");
const CIMName OM_PROPERTY_COMMUNICATIONMECHANISM("CommunicationMechanism");
const CIMName OM_PROPERTY_FUNCTIONALPROFILESSUPPORTED(
    "FunctionalProfilesSupported");
const CIMName OM_PROPERTY_FUNCTIONALPROFILEDESCRIPTIONS(
    "FunctionalProfileDescriptions");
const CIMName OM_PROPERTY_AUTHENTICATIONMECHANISMSSUPPORTED(
    "AuthenticationMechanismsSupported");
const CIMName OM_PROPERTY_AUTHENTICATIONMECHANISMDESCRIPTIONS(
    "AuthenticationMechanismDescriptions");
const CIMName OM_PROPERTY_MULTIPLEOPERATIONSSUPPORTED(
    "MultipleOperationsSupported");
const CIMName OM_PROPERTY_VERSION("Version");
const CIMName OM_PROPERTY_OPERATIONALSTATUS("OperationalStatus");
const CIMName OM_PROPERTY_STARTED("Started");

// Property Names for CIMXML CommunicationMechanism
const CIMName CIMXMLCOMMMECH_PROPERTY_CIMVALIDATED("CIMValidated");
const CIMName CIMXMLCOMMMECH_PROPERTY_COMMUNICATIONMECHANISM(
        "CommunicationMechanism");
const CIMName CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILESSUPPORTED(
        "FunctionalProfilesSupported");
const CIMName CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILEDESCRIPTIONS(
        "FunctionalProfileDescriptions");
const CIMName CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMSSUPPORTED(
        "AuthenticationMechanismsSupported");
const CIMName CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMDESCRIPTIONS(
        "AuthenticationMechanismDescriptions");
const CIMName CIMXMLCOMMMECH_PROPERTY_MULTIPLEOPERATIONSSUPPORTED(
        "MultipleOperationsSupported");
const CIMName CIMXMLCOMMMECH_PROPERTY_VERSION("Version");
const CIMName CIMXMLCOMMMECH_PROPERTY_NAMESPACETYPE("namespaceType");
const CIMName CIMXMLCOMMMECH_PROPERTY_NAMESPACEACCESSPROTOCOL(
    "namespaceAccessProtocol");
const CIMName CIMXMLCOMMMECH_PROPERTY_IPADDRESS("IPAddress");
#define CIMXMLCOMMMECH_PROPERTY_ELEMENTNAME OM_PROPERTY_ELEMENTNAME
#define CIMXMLCOMMMECH_PROPERTY_OPERATIONALSTATUS OM_PROPERTY_OPERATIONALSTATUS
#define CIMXMLCOMMMECH_PROPERTY_NAME COMMON_PROPERTY_NAME
#define CIMXMLCOMMMECH_PROPERTY_CREATIONCLASSNAME OM_PROPERTY_CREATIONCLASSNAME
const CIMName CIMXMLCOMMMECH_PROPERTY_ADVERTISETYPES("AdvertiseTypes");

//
// Fills in the CIMOperation functional profiles and corresponding description
// array.  This function is closely linked to compile and configuration
// features in the CIM Server to determine if certain features are 
// enabled and/or compiled.  Definitions correspond to the DMTF SLP template
// version 1.0.
// @param Array<Uint16> profiles provides an array for the profiles
// @param Array<String> with the corresponding profile text descriptions
//
void getFunctionalProfiles(
    Array<Uint16> & profiles,
    Array<String> & profileDescriptions)
{
    // Note that zero and 1 are unknown and other. Not used by us
    // 2 - 5 are not optional in Pegasus
    profiles.append(2);
    profileDescriptions.append("Basic Read");

    profiles.append(3);
    profileDescriptions.append("Basic Write");

    profiles.append(4);
    profileDescriptions.append("Schema Manipulation");

    profiles.append(5);
    profileDescriptions.append("Instance Manipulation");

    ConfigManager* configManager = ConfigManager::getInstance();
    if (String::equal(configManager->getCurrentValue(
        "enableAssociationTraversal"), "true"))
    {
        profiles.append(6);
        profileDescriptions.append("Association Traversal");
    }
#ifndef PEGASUS_DISABLE_EXECQUERY
    profiles.append(7);
    profileDescriptions.append("Query Execution");
#endif
    profiles.append(8);
    profileDescriptions.append("Qualifier Declaration");

    if (String::equal(configManager->getCurrentValue(
        "enableIndicationService"), "true"))
    {
        profiles.append(9);
        profileDescriptions.append("Indications");
    }
}

//
// Build a single instance of the CIMXMLCommunicationMechanism class using the
// parameters provided. Builds the complete instance and sets its object path.
//
CIMInstance InteropProvider::buildCIMXMLCommunicationMechanismInstance(
            const String& namespaceType,
            const Uint16& accessProtocol,
            const String& IPAddress,
            const CIMClass & targetClass)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::buildCIMXMLCommunicationMechanismInstance()");
    CIMInstance instance = targetClass.buildInstance(false, false,
        CIMPropertyList());

    setCommonKeys(instance);

    // CreationClassName property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_CREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM.getString());

    // Name Property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_NAME,
        (String("PEGASUSCOMM") + namespaceType));

    // CommunicationMechanism Property - Force to 2.
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_COMMUNICATIONMECHANISM,
        Uint16(2));

    //Functional Profiles Supported Property.
    Array<Uint16> profiles;
    Array<String> profileDescriptions;
    getFunctionalProfiles(profiles, profileDescriptions);

    // Set functional profiles for the instance
    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILESSUPPORTED, profiles);

    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILEDESCRIPTIONS,
        profileDescriptions);

    // MultipleOperationsSupported Property
    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_MULTIPLEOPERATIONSSUPPORTED, false);

    // AuthenticationMechanismsSupported Property
    Array<Uint16> authentications;
    Array<String> authenticationDescriptions;

    //TODO - get from system.
    authentications.append(3);
    authenticationDescriptions.append("Basic");

    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMSSUPPORTED,
        authentications);

    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMDESCRIPTIONS,
        authenticationDescriptions);

    // Version Property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_VERSION,
        CIMXMLProtocolVersion);

    // NamespaceType Property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_NAMESPACETYPE,
        namespaceType);

    // NamespaceAccessProtocol property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_NAMESPACEACCESSPROTOCOL,
        accessProtocol);

    // IPAddress property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_IPADDRESS,
        IPAddress);

    // ElementName property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_ELEMENTNAME,
        String("Pegasus CIMXML Communication Mechanism"));

    // CIMValidated property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_CIMVALIDATED,
        Boolean(false));

    // OperationalStatus property
    Array<Uint16> opStatus;
    opStatus.append(2); // "OK"
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_OPERATIONALSTATUS,
        opStatus);

    // AdvertiseTypes property
    Array<Uint16> advertiseTypes;
    ConfigManager* configManager = ConfigManager::getInstance();
    if (String::equal(configManager->getCurrentValue("slp"), "true"))
    {
        advertiseTypes.append(3); // Advertised via SLP
    }
    else
    {
        advertiseTypes.append(2); // Not advertised
    }
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_ADVERTISETYPES,
        advertiseTypes);

    // build the instance path and set into instance
    CIMObjectPath objPath = instance.buildPath(targetClass);
    objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    objPath.setHost(hostName);
    instance.setPath(objPath);

    PEG_METHOD_EXIT();
    return instance;
}

//
// Retrieves all of the instances of CIMXMLCommunicationMechanism for the
// CIMOM.
//
Array<CIMInstance> InteropProvider::enumCIMXMLCommunicationMechanismInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::enumCIMXMLCommunicationMechanismInstances");

    ConfigManager* configManager = ConfigManager::getInstance();
    Boolean enableHttpConnection = String::equal(
        configManager->getCurrentValue("enableHttpConnection"), "true");
    Boolean enableHttpsConnection = String::equal(
        configManager->getCurrentValue("enableHttpsConnection"), "true");

    Array<CIMInstance> instances;
    Uint32 namespaceAccessProtocol;
    String namespaceType;

    CIMClass commMechClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM, false, true, false);

    if (enableHttpConnection)
    {
        // Build the CommunicationMechanism instance for the HTTP protocol
        namespaceAccessProtocol = 2;
        namespaceType = "http";
        String httpPort = configManager->getCurrentValue("httpPort");
        if (httpPort == String::EMPTY)
        {
            Uint32 portNumberHttp = System::lookupPort(
                WBEM_HTTP_SERVICE_NAME, WBEM_DEFAULT_HTTP_PORT);
            char buffer[32];
            sprintf(buffer, "%u", portNumberHttp);
            httpPort.assign(buffer);
        }
        CIMInstance instance = 
            buildCIMXMLCommunicationMechanismInstance(
                namespaceType,
                namespaceAccessProtocol,
                getHostAddress(hostName, namespaceAccessProtocol, httpPort),
                commMechClass);
        instances.append(instance);
    }

    if (enableHttpsConnection)
    {
        // Build the CommunicationMechanism instance for the HTTPS protocol
        namespaceAccessProtocol = 3;
        namespaceType = "https";
        String httpsPort = configManager->getCurrentValue("httpsPort");
        if (httpsPort == String::EMPTY)
        {
            Uint32 portNumberHttps = System::lookupPort(
                WBEM_HTTPS_SERVICE_NAME, WBEM_DEFAULT_HTTPS_PORT);
            char buffer[32];
            sprintf(buffer, "%u", portNumberHttps);
            httpsPort.assign(buffer);
        }
        CIMInstance instance = 
            buildCIMXMLCommunicationMechanismInstance(
                namespaceType,
                namespaceAccessProtocol,
                getHostAddress(hostName, namespaceAccessProtocol, httpsPort),
                commMechClass);

        instances.append(instance);
    }

    PEG_METHOD_EXIT();
    return instances;
}

//
// Get the instance of the CIM_ObjectManager class, creating the instance if it
// does not already exist in the repository.
//
// @param includeQualifiers Boolean
// @param includeClassOrigin Boolean
// @param propertylist CIMPropertyList
//
// @return CIMInstance with a single built instance of the class
//
// @exception repository instances if exception to enumerateInstances
// for this class.
//
CIMInstance InteropProvider::getObjectManagerInstance()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getObjectManagerInstance");

    // Try to get the instance from the repository.
    CIMInstance instance;
    bool found = false;
    Array<CIMInstance> tmpInstances = repository->enumerateInstancesForClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_OBJECTMANAGER, false, false, false,
        CIMPropertyList());
    Uint32 numInstances = tmpInstances.size();
    if(numInstances == 1)
    {
        instance = tmpInstances[0];
    }
    PEGASUS_ASSERT(numInstances <= 1);


    if(instance.isUninitialized())
    {
        //
        // No instance in the repository. Build new instance and save it.
        //
        CIMClass omClass;
        instance = buildInstanceSkeleton(PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_OBJECTMANAGER, omClass);

        // Set the common key properties
        setCommonKeys(instance);

        setPropertyValue(instance, OM_PROPERTY_CREATIONCLASSNAME,
            PEGASUS_CLASSNAME_PG_OBJECTMANAGER.getString());
        setPropertyValue(instance, OM_PROPERTY_NAME,
            String(PEGASUS_INSTANCEID_GLOBAL_PREFIX) + ":" + Guid::getGuid());
        setPropertyValue(instance, OM_PROPERTY_ELEMENTNAME, String("Pegasus"));
        Array<Uint16> operationalStatus;
        operationalStatus.append(2);
        setPropertyValue(instance, OM_PROPERTY_OPERATIONALSTATUS,
            operationalStatus);
        setPropertyValue(instance, OM_PROPERTY_STARTED,
            CIMValue(Boolean(true)));

        //
        // Description property this object manager instance.
        // If PEGASUS_CIMOM_DESCRIPTION is non-zero length, use it.
        // Otherwise build form the components below, as defined in
        // PegasusVersion.h.
        String descriptionStatus;
        String pegasusProductStatus(PEGASUS_PRODUCT_STATUS);
        if(pegasusProductStatus.size() > 0)
            descriptionStatus = " " + pegasusProductStatus;

        String description = (String(PEGASUS_CIMOM_DESCRIPTION).size() != 0) ?
                String(PEGASUS_CIMOM_DESCRIPTION)
            :
                String(PEGASUS_CIMOM_GENERIC_NAME) + " " +
                String(PEGASUS_PRODUCT_NAME) + " Version " +
                String(PEGASUS_PRODUCT_VERSION) +
                descriptionStatus;

        setPropertyValue(instance, OM_PROPERTY_DESCRIPTION, description);

        // Property GatherStatisticalData. Initially this is set to false
        // and can then be modified by a modify instance on the instance.
        Boolean gatherStatDataFlag = false;
        setPropertyValue(instance, OM_PROPERTY_GATHERSTATISTICALDATA,
            gatherStatDataFlag);

        // Set the statistics property into the Statisticaldata class so that
        // it can perform statistics gathering if necessary.
#ifndef PEGASUS_DISABLE_PERFINST
        StatisticalData* sd = StatisticalData::current();
        sd->setCopyGSD(gatherStatDataFlag);
#endif

        // write instance to the repository
        CIMObjectPath instancePath = repository->createInstance(
            PEGASUS_NAMESPACENAME_INTEROP, instance);
        instance.setPath(instancePath);
    }

    CIMObjectPath currentPath = instance.getPath();
    currentPath.setHost(hostName);
    currentPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    instance.setPath(currentPath);
    PEG_METHOD_EXIT();
    return instance;
}

//
// Modify the existing Object Manager instance.  Only a single property
// modification is allowed, the statistical data setting.  Any other change is
// rejected with an exception.
//
void InteropProvider::modifyObjectManagerInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::modifyObjectManagerInstance");

    // Modification only allowed when Performance staticistics are active
#ifndef PEGASUS_DISABLE_PERFINST
    Uint32 propListSize = propertyList.size();
    if(propListSize == 0 && !propertyList.isNull())
        return;

    if(propertyList.size() != 1 ||
        propertyList[0] != OM_PROPERTY_GATHERSTATISTICALDATA)
    {
        throw CIMNotSupportedException(String("Only modification of ") +
            OM_PROPERTY_GATHERSTATISTICALDATA.getString() + " allowed");
    }

    Boolean statisticsFlag;
    CIMInstance omInstance;

    // We modify only if this property exists.
    // could either use the property from modifiedIns or simply replace
    // value in property from object manager.
    if (modifiedIns.findProperty(OM_PROPERTY_GATHERSTATISTICALDATA) !=
        PEG_NOT_FOUND)
    {
        omInstance = getObjectManagerInstance();
        if(omInstance.isUninitialized())
        {
            throw CIMObjectNotFoundException(instanceReference.toString());
        }
        statisticsFlag = getPropertyValue(modifiedIns,
            OM_PROPERTY_GATHERSTATISTICALDATA, false);
        // set the changed property into the instance
        setPropertyValue(omInstance, OM_PROPERTY_GATHERSTATISTICALDATA,
            statisticsFlag);
    }
    else
    {
        // if statistics property not in place, simply exit. Nothing to do
        // not considered an error
        PEG_METHOD_EXIT();
        return;
    }
    // Modify the instance on disk
    repository->modifyInstance(instanceReference.getNameSpace(),
        omInstance, false,  propertyList);
    Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
        "Interop Provider Set Statistics gathering in CIM_ObjectManager: $0",
        (statisticsFlag? "true" : "false"));
    StatisticalData* sd = StatisticalData::current();
    sd->setCopyGSD(statisticsFlag);
    PEG_METHOD_EXIT();
    return;

#else
    PEG_METHOD_EXIT();
    throw CIMNotSupportedException
        (OM_PROPERTY_GATHERSTATISTICALDATA.getString() + 
                " modify operation not supported by Interop Provider");
#endif
}


//
// Get an instance of the PG_ComputerSystem class produced by the
// ComputerSystem provider in the root/cimv2 namespace.
//
// @param includeQualifiers Boolean
// @param includeClassOrigin Boolean
// @param propertylist CIMPropertyList
//
// @return CIMInstance of PG_ComputerSystem class.
//
// @exception ObjectNotFound exception if a ComputerSystem instance cannot
//     be retrieved.
//
CIMInstance InteropProvider::getComputerSystemInstance()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getComputerSystemInstance");

    CIMInstance instance;
    AutoMutex mut(interopMut);
    Array<CIMInstance> tmpInstances = cimomHandle.enumerateInstances(
        OperationContext(),
        PEGASUS_NAMESPACENAME_CIMV2,
        PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM, true, false, false, false,
        CIMPropertyList());
    Uint32 numInstances = tmpInstances.size();
    PEGASUS_ASSERT(numInstances <= 1);
    if(numInstances > 0)
    {
        instance = tmpInstances[0];
        CIMObjectPath tmpPath = instance.getPath();
        tmpPath.setHost(hostName);
        tmpPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
        instance.setPath(tmpPath);
    }

    if(instance.isUninitialized())
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND,
            "Could not find ComputerSystem instance");
    }

    PEG_METHOD_EXIT();
    return instance;
}

//
// Returns an instance of the HostedObjectManager association linking the
// ObjectManager and ComputerSystem instances managed by this provider.
//
CIMInstance InteropProvider::getHostedObjectManagerInstance()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getHostedObjectManagerInstance");

    // Try to get the current object.  If true then it is already created.
    CIMInstance instance;
    bool found = false;

    CIMObjectPath csPath = getComputerSystemInstance().getPath();
    CIMObjectPath omPath = getObjectManagerInstance().getPath();
    String csPathString = csPath.toString();
    String omPathString = omPath.toString();

    CIMClass hostedOMClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_HOSTEDOBJECTMANAGER,
        false, true, false);

    instance = hostedOMClass.buildInstance(false, false, CIMPropertyList());

    setPropertyValue(instance, PROPERTY_ANTECEDENT,
        CIMValue(csPath));
    setPropertyValue(instance, PROPERTY_DEPENDENT,
        CIMValue(omPath));

    instance.setPath(instance.buildPath(hostedOMClass));

    PEG_METHOD_EXIT();
    return instance;
}

//
// Returns an array containing all of the HostedAccessPoint association
// instances for this CIMOM. One will be produced for every instance of
// CIMXMLCommunicatiomMechanism managed by this provider.
//
Array<CIMInstance> InteropProvider::enumHostedAccessPointInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumHostedAccessPointInstance");
    Array<CIMInstance> instances;

    CIMObjectPath csPath = getComputerSystemInstance().getPath();
    Array<CIMInstance> commMechs = enumCIMXMLCommunicationMechanismInstances();
    CIMClass hapClass = repository->getClass(PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_HOSTEDACCESSPOINT, false, true, false);
    for(Uint32 i = 0, n = commMechs.size(); i < n; ++i)
    {
        CIMInstance & currentCommMech = commMechs[i];
        CIMInstance hapInstance = hapClass.buildInstance(false, false,
            CIMPropertyList());
        setPropertyValue(hapInstance, PROPERTY_ANTECEDENT, csPath);
        setPropertyValue(hapInstance, PROPERTY_DEPENDENT,
            currentCommMech.getPath());
        hapInstance.setPath(hapInstance.buildPath(hapClass));
        instances.append(hapInstance);
    }

    PEG_METHOD_EXIT();
    return instances;
}


//
// Returns an array containing all of the CommMechanismForManager association
// instances for this CIMOM. One will be produced for every instance of
// CIMXMLCommunicatiomMechanism managed by this provider.
//
Array<CIMInstance> InteropProvider::enumCommMechanismForManagerInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumCommMechanismForManagerInstances");

    Array<CIMInstance> commInstances =
        enumCIMXMLCommunicationMechanismInstances();

    CIMInstance instanceObjMgr = getObjectManagerInstance();

    CIMObjectPath refObjMgr = instanceObjMgr.getPath();

    Array<CIMInstance> assocInstances;
    CIMClass targetClass;
    CIMInstance instanceskel = buildInstanceSkeleton(
        PEGASUS_NAMESPACENAME_INTEROP, 
        PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER, targetClass);
    for (Uint32 i = 0, n = commInstances.size(); i < n; ++i)
    {
        CIMInstance instance = instanceskel.clone();

        setPropertyValue(instance, PROPERTY_ANTECEDENT, refObjMgr);

        setPropertyValue(instance, PROPERTY_DEPENDENT,
          commInstances[i].getPath());

        instance.setPath(instance.buildPath(targetClass));
        assocInstances.append(instance);
    }

    PEG_METHOD_EXIT();
    return assocInstances;
}

PEGASUS_NAMESPACE_END

// END OF FILE
