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
//%////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>      // gethostname()
#include <sys/socket.h>  // gethostbyname()
#include <netinet/in.h>  // gethostbyname()
#include <netdb.h>       // gethostbyname()
#include <time.h>        // localtime_r()
#include <sys/param.h>   // MAXHOSTNAMELEN
#include <fstream>

#include "ComputerSystemProvider.h"
#include "ComputerSystem.h"
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Constants.h>

#define MODEL "Model"
#define DMI_FILE "/var/dmi/mif/C/hpuxci.parms"
#define GEN_INFO_GROUP_ID 2

//Needed to compile on HPUX 11.00
#ifndef _CS_MACHINE_SERIAL
# define _CS_MACHINE_SERIAL 10005
#endif
#ifndef _CS_MACHINE_IDENT 
# define _CS_MACHINE_IDENT 10003
#endif

#define HPUX_DEFAULT_OPERATIONAL_STATUS 2
#define HP_UX_DEFAULT_STATUS_DESCRIPTIONS \
    "One or more components that make up this computer system have an " \
    "OperationalStatus value of OK or Completed."
#define TIMEOUT 360000

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static CIMValue _serialNumber = CIMValue(CIMTYPE_STRING, false, 0);
static String _hostName;
static String _model;
static CIMValue _uuid = CIMValue(CIMTYPE_STRING, false, 0);
static CIMDateTime _installDate;
static String _primaryOwnerName;
static String _primaryOwnerContact;
static String _primaryOwnerPager;
static String _secondaryOwnerName;
static String _secondaryOwnerContact;
static String _secondaryOwnerPager;
String _status;
Array<Uint16> _operationalStatus;
Array<String> _statusDescriptions;

ComputerSystem::ComputerSystem()
{
}

ComputerSystem::~ComputerSystem()
{
}

// Routines to obtain property values from system

Boolean ComputerSystem::getCaption(CIMProperty& p)
{
    // return model string for this property
    p = CIMProperty(PROPERTY_CAPTION, _model);
    return true;
}

Boolean ComputerSystem::getDescription(CIMProperty& p)
{
    // return model string for this property
    p = CIMProperty(PROPERTY_DESCRIPTION, _model);
    return true;
}

Boolean ComputerSystem::getInstallDate(CIMProperty& p)
{
    // set in initialize()
    p = CIMProperty(PROPERTY_INSTALL_DATE, _installDate);
    return true;
}

Boolean ComputerSystem::getCreationClassName(CIMProperty& p)
{
    // can vary, depending on class
    p = CIMProperty(
        PROPERTY_CREATION_CLASS_NAME,
        String(CLASS_CIM_COMPUTER_SYSTEM));
    return true;
}

Boolean ComputerSystem::getName(CIMProperty& p)
{
    // set in initialize()
    p = CIMProperty(PROPERTY_NAME, _hostName);
    return true;
}

Boolean ComputerSystem::getStatus(CIMProperty& p)
{
    // return status string for this property
    p = CIMProperty(PROPERTY_STATUS, _status);
    return true;
}

Boolean ComputerSystem::getOperationalStatus(CIMProperty& p)
{
    // return operational status array for this property
    p = CIMProperty(PROPERTY_OPERATIONAL_STATUS, _operationalStatus);
    return true;
}

Boolean ComputerSystem::getStatusDescriptions(CIMProperty& p)
{
    // return status descriptions array for this property
    p = CIMProperty(PROPERTY_STATUS_DESCRIPTIONS, _statusDescriptions);
    return true;
}

Boolean ComputerSystem::getNameFormat(CIMProperty& p)
{
    // hardcoded
    p = CIMProperty(PROPERTY_NAME_FORMAT, String(NAME_FORMAT));
    return true;
}

Boolean ComputerSystem::getPrimaryOwnerName(CIMProperty& p)
{
    // set in initialize() from DMI
    p = CIMProperty(PROPERTY_PRIMARY_OWNER_NAME, _primaryOwnerName);
    return true;
}

Boolean ComputerSystem::setPrimaryOwnerName(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getPrimaryOwnerContact(CIMProperty& p)
{
    // set in initialize() from DMI
    p = CIMProperty(PROPERTY_PRIMARY_OWNER_CONTACT, _primaryOwnerContact);
    return true;
}

Boolean ComputerSystem::setPrimaryOwnerContact(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getRoles(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getOtherIdentifyingInfo(CIMProperty& p)
{
    // we will return model for this property
    Array<String> s;
    s.append(_model);
    p = CIMProperty(PROPERTY_OTHER_IDENTIFYING_INFO, s);
    return true;
}

Boolean ComputerSystem::getIdentifyingDescriptions(CIMProperty& p)
{
    // hardcoded
    Array<String> s;
    s.append(MODEL);
    p = CIMProperty(PROPERTY_IDENTIFYING_DESCRIPTIONS, s);
    return true;
}

Boolean ComputerSystem::getDedicated(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getResetCapability(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getPowerManagementCapabilities(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getInitialLoadInfo(CIMProperty& p)
{
    // get from /stand/bootconf
    // perhaps this can change dynamically, so don't do it
    // in initialize()
    FILE* s = fopen("/stand/bootconf", "r");
    if (s == 0)
        throw CIMOperationFailedException("/stand/bootconf: can't open");
    char buf[100];
    if (fgets(buf,100,s) == 0)
        throw CIMOperationFailedException("/stand/bootconf: can't read");
    fclose(s);
    Array<String> res;
    res.append(String(buf));
    p = CIMProperty(PROPERTY_INITIAL_LOAD_INFO, res);
    return true;
}

Boolean ComputerSystem::getLastLoadInfo(CIMProperty& p)
{
    // ATTN: not sure yet
    return false;
}

Boolean ComputerSystem::getPowerManagementSupported(CIMProperty& p)
{
    // hardcoded
    p = CIMProperty(PROPERTY_POWER_MANAGEMENT_SUPPORTED, false); // on PA-RISC!!
    return true;
}

Boolean ComputerSystem::getPowerState(CIMProperty& p)
{
    // hardcoded
    /*
        ValueMap {"1", "2", "3", "4", "5", "6", "7", "8"},
        Values {"Full Power", "Power Save - Low Power Mode", 
                "Power Save - Standby", "Power Save - Other", 
                "Power Cycle", "Power Off", "Hibernate", "Soft Off"}
    */
    // Full Power on PA-RISC!!
    p = CIMProperty(PROPERTY_POWER_STATE, Uint16(1));
    return true;
}

Boolean ComputerSystem::getWakeUpType(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getPrimaryOwnerPager(CIMProperty& p)
{
    // set in initialize() from DMI
    p = CIMProperty(PROPERTY_PRIMARY_OWNER_PAGER, _primaryOwnerPager);
    return true;
}

Boolean ComputerSystem::setPrimaryOwnerPager(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSecondaryOwnerName(CIMProperty& p)
{
    // set in initialize() from DMI
    p = CIMProperty(PROPERTY_SECONDARY_OWNER_NAME, _secondaryOwnerName);
    return true;
}

Boolean ComputerSystem::setSecondaryOwnerName(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSecondaryOwnerContact(CIMProperty& p)
{
    // set in initialize() from DMI
    p = CIMProperty(PROPERTY_SECONDARY_OWNER_CONTACT, _secondaryOwnerContact);
    return true;
}

Boolean ComputerSystem::setSecondaryOwnerContact(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSecondaryOwnerPager(CIMProperty& p)
{
    // set in initialize() from DMI
    p = CIMProperty(PROPERTY_SECONDARY_OWNER_PAGER, _secondaryOwnerPager);
    return true;
}

Boolean ComputerSystem::setSecondaryOwnerPager(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSerialNumber(CIMProperty& p)
{
    p = CIMProperty(PROPERTY_SERIAL_NUMBER, _serialNumber); 
    return true;
}

Boolean ComputerSystem::getIdentificationNumber(CIMProperty& p)
{
    // return UUID for this property
    p = CIMProperty(PROPERTY_IDENTIFICATION_NUMBER, _uuid);
    return true;
}

void ComputerSystem::initialize()
{
    // fills in the values of all properties that are not
    // hardcoded (i.e., are obtained from the system) but not
    // going to change dynamically (which is most, for this
    // provider)

    // _hostName
    struct hostent *he;
    char hn[PEGASUS_MAXHOSTNAMELEN + 1];

    // fill hn with what this system thinks is name
    gethostname(hn, sizeof(hn));
    hn[sizeof(hn)-1] = 0;

    // find out what nameservices think is full name
    if (he=gethostbyname(hn))
        _hostName = he->h_name;
    else
        _hostName = hn;

    size_t bufSize;

    // get serial number using confstr  
    bufSize = confstr(_CS_MACHINE_SERIAL, NULL, 0);
    if (bufSize != 0)
    {
        char* serialNumber = new char[bufSize];
        try
        {
            if (confstr(_CS_MACHINE_SERIAL, serialNumber, bufSize) != 0)
            {
                _serialNumber.set(String(serialNumber));
            }
        }
        catch(...)
        {
            delete [] serialNumber;
            throw;
        }
        delete [] serialNumber;
    }

    // get model using command
    FILE* s = popen("/usr/bin/model", "r");
    if (s == 0)
        throw CIMOperationFailedException("/usr/bin/model: command not found");
    char buf[100];
    if (fgets(buf,100,s) == 0)
        throw CIMOperationFailedException("/usr/bin/model: no output");
    pclose(s);
    _model = String(buf);

    // get system UUID using confstr.  
    bufSize = confstr(_CS_MACHINE_IDENT, NULL, 0);
    if (bufSize != 0)
    {
        char* uuid = new char[bufSize];
        try
        {
            if (confstr(_CS_MACHINE_IDENT, uuid, bufSize) != 0)
            {
                _uuid.set(String(uuid));
            }
        }
        catch(...)
        {
            delete [] uuid;
            throw;
        }
        delete [] uuid;
    }

    // InstallDate
    /*
        A CIM date has the following form:
            yyyymmddhhmmss.mmmmmmsutc

        Where

            yyyy = year (0-1999)
            mm = month (1-12)
            dd = day (1-31)
            hh = hour (0-23)
            mm = minute (0-59)
            ss = second (0-59)
            mmmmmm = microseconds.
            s = '+' or '-' to represent the UTC sign.
            utc = UTC offset (same as GMT offset).
    */
    // creation date of /stand/vmunix
    struct stat st;
    // get get modification time of file
    if (0 != stat("/stand/vmunix", &st))
        throw CIMOperationFailedException("/stand/vmunix: can't access");
    // convert to a usable format
    struct tm tmBuffer;
    struct tm* t = localtime_r(&st.st_mtime, &tmBuffer);
    // convert to CIMDateTime format
    char timstr[26];
    sprintf(
        timstr,
        "%04d%02d%02d%02d%02d%02d.000000%c%03ld",
        t->tm_year+1900,
        t->tm_mon+1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        (timezone>0)?'-':'+',
        labs (timezone/60 - (t->tm_isdst? 60:0)));
    _installDate = CIMDateTime(timstr);

    // ----------------------------------------------------------
    // Now set properties obtained from DMI
    // ----------------------------------------------------------

    typedef enum
    {
        GenInfoSysName_E       =  1,
        GenInfoSysLoc_E        =  2,
        GenInfoSysPUser_E      =  3,
        GenInfoSysPPhone_E     =  4,
        GenInfoSysUpTime_E     =  5,
        GenInfoSysDate_E       =  6,
        GenInfoSysSUser_E      =  7,
        GenInfoSysSPhone_E     =  8,
        GenInfoSysPPager_E     =  9,
        GenInfoSysSPager_E     = 10,
        GenInfoSecurity_E      = 11,
        GenInfoModel_E         = 12,
        GenInfoSerialNumber_E  = 13,
        GenInfoSoftwareID_E    = 14
    } GenInfoEnumList;

    char                  inLine[1024];
    char                 *tmpGroupId = NULL;
    char                 *tmpAttrId = NULL;
    char                 *value = NULL;
    int                   groupId;
    int                   attrId;
    char                 *tokp = NULL;

    // open file
    ifstream mParmStream(DMI_FILE);
    // Values will be left blank if can't access file
    if (mParmStream == 0)
        return;

    while (mParmStream.getline(inLine, sizeof(inLine)))
    {
        /* Parse out the line to get the DMI group Id, attribute Id */
        /* and value.                                               */
        tmpGroupId = strtok_r(inLine, "|", &tokp);
        tmpAttrId = strtok_r(NULL, "|", &tokp);
        value = strtok_r(NULL, "\n", &tokp);

        if (NULL != tmpGroupId)
        {
            groupId = atoi(tmpGroupId);
        }
        else
        {
            continue;
        }

        if (NULL != tmpAttrId)
        {
            attrId = atoi(tmpAttrId);
        }
        else
        {
            continue;
        }

        /* Make sure information read in is the right DMI group. */
        if ((groupId != GEN_INFO_GROUP_ID) || (NULL == value))
        {
            continue;
        }
    
        if (attrId == GenInfoSysPUser_E)
        {
            _primaryOwnerName = value;
        }
        else if (attrId == GenInfoSysPPhone_E)
        {
            _primaryOwnerContact = value;
        }
        else if (attrId == GenInfoSysSUser_E)
        {
            _secondaryOwnerName = value;
        }
        else if (attrId == GenInfoSysSPhone_E)
        {
            _secondaryOwnerContact = value;
        }
        else if (attrId == GenInfoSysPPager_E)
        {
            _primaryOwnerPager = value;
        }
        else if (attrId == GenInfoSysSPager_E)
        {
            _secondaryOwnerPager = value;
        }
    }  /* while */

    return;
}

String ComputerSystem::getHostName()
{
    return _hostName;
}

CIMInstance ComputerSystem::buildInstance(const CIMName& className)
{
    Boolean useDefaultStatus = true;
    try
    {
        Array<CIMObjectPath> instanceNames;

        CIMInstance returnedInstance;
        Array <CIMName> propertyNames;
        propertyNames.append (PROPERTY_STATUS);
        propertyNames.append (PROPERTY_OPERATIONAL_STATUS);
        propertyNames.append (PROPERTY_STATUS_DESCRIPTIONS);
        CIMPropertyList propertyList;
        propertyList.set (propertyNames);

        _status.clear();
        _operationalStatus.clear();
        _statusDescriptions.clear();

        CIMValue status;
        CIMValue operationalStatus;
        CIMValue statusDescriptions;

        CIMClient hpcsclient;
        hpcsclient.setTimeout(TIMEOUT);
        hpcsclient.connectLocal();

        instanceNames = hpcsclient.enumerateInstanceNames(
            PEGASUS_NAMESPACENAME_CIMV2,
            CIMName("HP_HealthState"));
        if (instanceNames.size() > 0)
        {
            for (Uint32 i = 0; i < instanceNames.size(); i++)
            {
                if (instanceNames[i].getClassName().equal(
                        CIMName("HP_HealthState")))
                {
                    returnedInstance = hpcsclient.getInstance(
                        PEGASUS_NAMESPACENAME_CIMV2,
                        instanceNames[i],
                        true,
                        false,
                        false,
                        propertyList);

                    Uint32 statusIndex =
                        returnedInstance.findProperty(PROPERTY_STATUS);
                    Uint32 operationalStatusIndex =
                        returnedInstance.findProperty(
                            PROPERTY_OPERATIONAL_STATUS);
                    Uint32 statusDescriptionsIndex =
                        returnedInstance.findProperty(
                            PROPERTY_STATUS_DESCRIPTIONS);
                    if (statusIndex != PEG_NOT_FOUND &&
                        operationalStatusIndex != PEG_NOT_FOUND &&
                        statusDescriptionsIndex != PEG_NOT_FOUND)
                    {
                        status.assign(returnedInstance.getProperty(
                            statusIndex).getValue());
                        operationalStatus.assign(returnedInstance.getProperty(
                            operationalStatusIndex).getValue());
                        statusDescriptions.assign(returnedInstance.getProperty(
                            statusDescriptionsIndex).getValue());
                        if (!status.isNull() &&
                            !operationalStatus.isNull() &&
                            !statusDescriptions.isNull())
                        {
                            // Set the properties, read from HP_HealhState.
                            status.get(_status);
                            operationalStatus.get(_operationalStatus);
                            statusDescriptions.get(_statusDescriptions);
                            useDefaultStatus = false;
                        }
                    }
                    break;
                }
            }
        }
    }
    catch (...)
    {
        // Use the default values if any exception is encountered.
    }

    if (useDefaultStatus)
    {
        _status.clear();
        _operationalStatus.clear();
        _statusDescriptions.clear();

        // hardcoded.
        _status = String(STATUS);
        _operationalStatus.append(Uint16(HPUX_DEFAULT_OPERATIONAL_STATUS));
        _statusDescriptions.append(String(HP_UX_DEFAULT_STATUS_DESCRIPTIONS));
    }

    CIMInstance instance(className);
    CIMProperty p;

    //-- fill in properties for CIM_ComputerSystem
    if (getCaption(p)) instance.addProperty(p);

    if (getDescription(p)) instance.addProperty(p);

    if (getInstallDate(p)) instance.addProperty(p);

    if (getStatus(p)) instance.addProperty(p);

    if (getOperationalStatus(p)) instance.addProperty(p);

    if (getStatusDescriptions(p)) instance.addProperty(p);

    if (getCreationClassName(p)) instance.addProperty(p);

    if (getName(p)) instance.addProperty(p);

    if (getNameFormat(p)) instance.addProperty(p);

    if (getPrimaryOwnerName(p)) instance.addProperty(p);

    if (getPrimaryOwnerContact(p)) instance.addProperty(p);

    if (getRoles(p)) instance.addProperty(p);

    if (getOtherIdentifyingInfo(p)) instance.addProperty(p);

    if (getIdentifyingDescriptions(p)) instance.addProperty(p);

    if (getDedicated(p)) instance.addProperty(p);

    if (getResetCapability(p)) instance.addProperty(p);

    if (getPowerManagementCapabilities(p)) instance.addProperty(p);

    // Done if we are servicing CIM_ComputerSystem
    if (className.equal (CLASS_CIM_COMPUTER_SYSTEM))
        return instance;

    // Fill in properties for CIM_UnitaryComputerSystem
    if (getInitialLoadInfo(p)) instance.addProperty(p);

    if (getLastLoadInfo(p)) instance.addProperty(p);

    if (getPowerManagementSupported(p)) instance.addProperty(p);

    if (getPowerState(p)) instance.addProperty(p);

    if (getWakeUpType(p)) instance.addProperty(p);

    // Done if we are servicing CIM_UnitaryComputerSystem
    if (className.equal (CLASS_CIM_UNITARY_COMPUTER_SYSTEM))
        return instance;

    // Fill in properties for <Extended>_ComputerSystem
    if (className.equal (CLASS_EXTENDED_COMPUTER_SYSTEM))
    {
        if(getPrimaryOwnerPager(p)) instance.addProperty(p);
        if(getSecondaryOwnerName(p)) instance.addProperty(p);
        if(getSecondaryOwnerContact(p)) instance.addProperty(p);
        if(getSecondaryOwnerPager(p)) instance.addProperty(p);
        if(getSerialNumber(p)) instance.addProperty(p);
        if(getIdentificationNumber(p)) instance.addProperty(p);
    }

    return instance;
}
