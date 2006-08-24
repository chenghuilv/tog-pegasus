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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Client/ClientOpPerformanceDataHandler.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

class CliStat : public ClientOpPerformanceDataHandler
{
public:

    virtual void handleClientOpPerformanceData (const ClientOpPerformanceData & item)
    {
        Boolean flagError = false;
        if (!(0 <= item.operationType) || !(39 >= item.operationType))
        {
            cerr << "Error: operation type " << item.operationType 
                << " is out of expected range in ClientOpPerformanceData " << endl;
            flagError = true;
        }

#ifndef PEGASUS_OS_TYPE_WINDOWS
        if (item.roundTripTime == 0)
        {
            cerr << "Error: roundTripTime is zero in ClientOpPerformanceData " << endl;
            flagError = true;
        }
#endif
        // If the roundTripTime value is *very* large, then it is suspect.
        // 0xF0000000 represents 4026 seconds, or over 1 hour.
        if (item.roundTripTime > (Uint64)0xF0000000)
        {
            cerr << "Error: roundTripTime is suspiciously large in ClientOpPerformanceData " << endl;
            flagError = true;
        }

        if (item.requestSize == 0)
        {
            cerr << "Error: requestSize is zero in ClientOpPerformanceData " << endl;
            flagError = true;
        }

        if (item.responseSize == 0)
        {
            cerr << "Error: responseSize is zero in ClientOpPerformanceData " << endl;
            flagError = true;
        }

        if (item.serverTimeKnown)
        {
#ifndef PEGASUS_OS_TYPE_WINDOWS
            if (item.serverTime == 0)
            {
                cerr << "Error: serverTime is zero in ClientOpPerformanceData " << endl;
                flagError = true;
            }
#endif
            // If the serverTime value is *very* large, then it is suspect.
            // 0xF0000000 represents 4026 seconds, or over 1 hour.
            if (item.serverTime > (Uint64)0xF0000000)
            {
                cerr << "Error: serverTime is suspiciously large in ClientOpPerformanceData " << endl;
                flagError = true;
            }
        }

        if (flagError)
        {
            cerr << "----- error(s) in Pegasus/Client/test/ClientStatistics" << endl;
            exit(1);
        }
    }
};




int main(int argc, char** argv)
{
    cout << "+++++ testing Client Performance Statistics " << endl;

    try
    {
// connecting to server
        CIMClient client;
        client.connect("localhost", 5988, String::EMPTY, String::EMPTY);

// the value CIM_ObjectManager::GatherStatisticalData must be set to true.
// the following code does this
        String className = "CIM_ObjectManager";
        CIMName cN = CIMName(className);
        Boolean loc = true;
        String gath = "GatherStatisticalData";
        CIMName gst = CIMName(gath);
        CIMValue val = CIMValue(loc);
        Array<CIMObjectPath> instanceNames;
        /* EnumerateInstances and
        */
        try
        {
            instanceNames = client.enumerateInstanceNames(PEGASUS_NAMESPACENAME_INTEROP, cN);
        }
        catch (Exception& e)
        {
            cerr << "Exception : " << e.getMessage() << endl;
            exit(1);
        }
        catch (...)
        {
            cout << "enumerateInstancesNames in Client/tests/ClientStatistics has thrown an exception" 
                << endl;
            exit(1);
        }
        // assert that we received  one name
        PEGASUS_TEST_ASSERT(instanceNames.size() == 1);
        /* ModifyInstance
        */
        CIMName gathStatName ("GatherStatisticalData");

        // variables used with gatherstatistics setting.
        CIMInstance instObjectManager;
        Uint32 prop_num;
        Array<CIMName> plA;
        plA.append(gathStatName);
        CIMPropertyList statPropertyList(plA);
        try
        {
            // Create property list that represents correct request
            // get instance.  Get only the gatherstatitistics property
            instObjectManager  = client.getInstance(PEGASUS_NAMESPACENAME_INTEROP, instanceNames[0],
                                                    true, false, false, statPropertyList);
            instObjectManager.setPath(instanceNames[0]);         // set correct path into instance

            prop_num = instObjectManager.findProperty(gathStatName);
            PEGASUS_TEST_ASSERT(prop_num != PEG_NOT_FOUND);

            instObjectManager.getProperty(prop_num).setValue(CIMValue(true));

            client.modifyInstance(PEGASUS_NAMESPACENAME_INTEROP, instObjectManager, false,statPropertyList);
        }
        catch (Exception& e)
        {
            cerr << "Exception : " << e.getMessage() << endl;
            exit(1);
        }

//registering class that has the callback method
        CliStat stat = CliStat();
        client.registerClientOpPerformanceDataHandler(stat);
        String classN = "PG_ComputerSystem";
        Array<CIMObjectPath> instance = client.enumerateInstanceNames(PEGASUS_NAMESPACENAME_CIMV2,
            classN);
        instObjectManager.getProperty(prop_num).setValue(CIMValue(false));
        client.modifyInstance(PEGASUS_NAMESPACENAME_INTEROP, instObjectManager, false,statPropertyList);
    }

    catch (Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        exit(1);
    }

    PEGASUS_STD(cout) << "+++++ passed all tests" << PEGASUS_STD(endl);

    return 0;
}
