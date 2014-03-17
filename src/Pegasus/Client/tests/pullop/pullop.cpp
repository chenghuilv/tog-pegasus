//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%////////////////////////////////////////////////////////////////////////////

/*
    Command line test client specifically to test the Pegasus pull operations.
    The command line defines specific pull operations and the targets,
    parameters and options for those operations.  For each operation there
    is a function that performs the complete pull operation (with parameters
    for maxObjectCount, interOperationDelay, operation parameters, etc.) and
    attempts to test the result (ex. returned object count corresponds to
    maxObjectCount, etc.) in addition, there is an option for each operation
    to perform the equivalent non-pull operation and compare the results
    for equality.  Thus for the OpenEnumerationInstances/PullInstancesWithPath
    sequence the corresponding non-pull operation is enumerateInstances.
    If the compare command line option is requested (-C) the array output of
    the pull sequence is compared with the array output of the enumerate for
    complete equality.
    There are functions for
    1. EnumerateInstances sequence
    2. EnumerateInstancePaths sequence
    3. Reference and ReferencePaths sequence
    4. Associator and AssociatorPaths sequence.

    In addition special operations are included to:

    1. all - Conduct the above tests on all instances in a namespace.

    The goal of this test program is to be able to test 99% of the functionality
    of the pull operations excluding primarily error tests and tests
    specifically for the format of the requests and responses (which are
    best done by wbemexc and the pegasus/test test cases)
*/
/*
    FUTURE - Expand concept of max objects to array so we can have a sequence.
             Could have either rotation through the array or remove used
             from top.  Add code to say which (repeat or something). This
             means we could remove the -p option in favor of use of the
             array (Low Priority)
           - Set timeout to spec default of NULL. MaxObjectCount to correspond
             to spec defaults. 0 for open, required for Pulls.
           - Figure way to do whole sequence of operations. Note that
             expanding maxobjects and timeout part of that. This would add
             repeat opt with corresponding do{...} while loop.
           - Expand tools for displaying timing. Want
             to be able to see overall and detailed results of each operation
             and somehow specify each.  For details we can do an array of
             a struct and then do a single accumulator for the total which
             would be the accumulated time of each operation.
           - Get info from host if it exists for display also.
           - Expand classname to array (i.e. multiple use of option) with
             for loop for processing of the list.
           - Modify input form to drop -c and simply put the target(s) on
             the command line.  easier to input, similar to cimcli and
             not as error prone.
           - Drop the existing defaults in favor of namespace = root/cimv2
             and no target default.
           - Expand object compare to more details (maybe)
           - Finish connect code so we can connect to other systems.  Needs
             to be tested and determine if we need security.
           - Combine verbose and verbose_opt.  i.e. Use a particular level
             of verbose_opt for verbose. Better just delete the old
             verbose option.  Not of real value here or use it to set
             a particular level of display output.
           - Review the other options for other parameters. I think we now have
             almost all operations defined.
           - Add concurrent repeat operation test mode (while loop around)
             whole setup and thread for each open, do, close with inner loop
             for repeat of particular open-pull sequence.
*/

/*
    Known issues
    -  In a compare of all classes, if the instances change during the
       operation or between the pull and other operation, we will report
       differences that are actually real differences, not errors.  The
       best way around that might be a mechanism to exclude certain classes
       from the all request via a parameter to provide a list of classes to
       be excluded.
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>

#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/Print.h>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <string.h>
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define LOCAL_MIN(a, b) ((a < b) ? a : b)
#define LOCAL_MAX(a, b) ((a > b) ? a : b)

// Development debug tool.
//#define TRACE cout << "Line " << __LINE__ << endl;
#define TRACE

static const char * _toCharP(Boolean x)
{
    return (x? "true" : "false");
}

String _toString(Boolean x)
{
    return (x? "true" : "false");
}

// Convert a CIMPropertyList parameter to CIM String
String _toString(const CIMPropertyList& pl)
{
    String rtn;
    Array<CIMName> pls = pl.getPropertyNameArray();
    if (pl.isNull())
        return("NULL");

    if (pl.size() == 0)
        return("EMPTY");

    for (Uint32 i = 0 ; i < pls.size() ; i++)
    {
        if (i != 0)
        {
            rtn.append(", ");
        }
        rtn.append(pls[i].getString());
    }
    return(rtn);
}

// Convert a CIMPropertyList parameter to CIM String
String _toStringProperties(const CIMInstance& inst)
{
    String rtn;

    for (size_t i = 0 ; i < inst.getPropertyCount() ; i++)
    {
        if (i > 0)
        {
            rtn.append(" ");
        }
        CIMConstProperty p = inst.getProperty(i);
        rtn.append(p.getName().getString());
    }

    return(rtn);
}
Uint32 stringToUint32(const char * str)
{
    Uint64 x;
    if (StringConversion::decimalStringToUint64(str,x, true))
    {
        if (StringConversion::checkUintBounds(x,CIMTYPE_UINT32))
        {
            return (Uint32)x;
        }
        else
        {
            cout << "Integer parameter " << str
                << " too large. Terminating." << endl;
            exit(1);
        }
    }
    else
    {
        cout << "Error in converting "  << str
            << " to Uint32. Terminating" << endl;
        exit(1);
    }
}

//***************************************************************************
//
//     Comand line option parameters
//
//***************************************************************************
static char * verbose;

// Defines for the various levels of output.
// The level of detail of output will depend on the level defined for
// the verbose_opt. Specifying a level on input gets that level and lower.
/*
    Definition of what is displayed at each level
    7 - returned objects
    6 - Step by Step through operations.
    5 - Details of operations (i.e. data returned)
    4 - Overview of each operation (send and rtn parameters)
    3 - Statistical information on operations
    2 - Add Warning messages
    1 - Error messages
    0 - None of the VCOUT calls
*/
#define VCOUT if (verbose) cout
#define VCOUT1 if (verbose_opt >= 1) cout
#define VCOUT2 if (verbose_opt >= 2) cout
#define VCOUT3 if (verbose_opt >= 3) cout
#define VCOUT4 if (verbose_opt >= 4) cout
#define VCOUT5 if (verbose_opt >= 5) cout
#define VCOUT6 if (verbose_opt >= 6) cout
#define VCOUT7 if (verbose_opt >= 7) cout

Uint32 warnings = 0;
Uint32 errors = 0;

String namespace_opt = "";
String host_opt = "";
String user_opt = "";
String password_opt = "";

// Spec default is null. We are using 0 for the moment.
// should be without the (0) which would get null state.
Uint32Arg timeout_opt(0);
Uint32 maxObjectsOnOpen_opt = 16;
Uint32 maxObjectsOnPull_opt =16;
String objectName_opt = "";
bool compare_opt = false;
Uint32 sleep_opt = 0;
Uint32 verbose_opt = 2;
bool timeOperation_opt = false;
bool continueOnError_opt = false;
bool reverseExitCode_opt = false;
bool deepInheritance_opt = true;
bool requestClassOrigin_opt = false;
bool errorsAsWarnings_opt = false;

// if not NULL defines a client timeout (to be set by the CIMClient.setTimeout
// function.  However, the input is in seconds so the call to the client
// function must multiply by 1000. The default is zero which m
Uint32Arg clientTimeoutSeconds_opt;

// Actual property list to be used. This should be null unless a property
// list is provided.
CIMPropertyList propertyList_opt;

/*
    Number of times to repeat the operation.  Note that 0 means
    do not repeat and is the default.
*/
Uint32 repeat_opt = 0;
/*
    Allows setting the filter parameters as part of testing.
    Sets it to the parameter provided with the option. The value
    of this option is set into the filter parameter
*/
String filterQuery_opt = "";
String filterQueryLanguage_opt = "";
/*
    Count of number of operations in enumeration sequence that we will
    issue before executing a close.  This is a means to force the close
    operation in testing with this client.
*/
Uint32Arg maxOperationsBeforeCloseCount_opt;

Uint32 pullCounter = 0;
Uint32 exitCode = 0;

const char USAGE[] = " \
\n\
Test client for Pegasus Pull operations.\n\
Usage: %s [OPERATION_TYPE] [OPTIONS]\n\
\n\
WHERE OPERATION_TYPE is one of the following pull operation sequences types:\n\
    e    enumerateInstances,\n\
    en   enumerate instance names,\n\
    r    reference, rn - referenceNames,\n\
    a    associators\n\
    an   assocatornames\n\
    all  test of all classes in one namespace\n\
\n\
Example:\n\
    $ pullop e -c Sample_InstanceProviderClass -C \n\
          Execute open,pull operations on class Sample_InstanceProviderClass\n\
          and compare results with original enumerateInstances operation.\n\
\n\
OPTIONS:\n\
    -V              Print version.\n\
    -v  LEVEL       Integer defining level of displays (0 - 6)\n\
                    0 - none, 1 - Errors, 2, warnings, 3 - statistical info,\n\
                    4 - operation overview, 5 - Operation details, 6 - Steps\n\
                    7 - display returned objects\n\
    -h              Print this help message.\n\
    --help          Print this help message.\n\
    -n NAMESPACE    The operation namespace(default is \"root/cimv2\").\n\
    -c OBJECTNAME   Name of class or object instance to process.\n\
    -H HOST         Connect to this HOST where format for host is \n\
                    hostName [:port]. Default is to use connectLocal.\n\
                    If hostName specified witout port(default is 5988)\n\
    -u USER         String. Connect as this USER.\n\
    -p PASSWORD     String. Connect with this PASSWORD.\n\
    -t TIMEOUT      Integer interoperation timeout. (Default 0)\n\
    -s seconds      Time to sleep between operations. Used to test timeouts\n\
    -T              Show results of detailed timing of the operations\n\
    -x              ContinueOnError flag set to true.\n\
    -o              Request ClassOrigin flag true where used.\n\
                    Default classOrigin = false \n\
    -d              Set deepInheritance false where used (i.e enumerate\n\
                    instances. Default: deepInheritance = true.\n\
    -M MAXOBJECTS   Integer Max objects for open operation.\n\
                    (Default 16).\n\
    -N MAXOBJECTS   Integer Max objects per pull operation.\n\
                    (Default 16).\n\
    -P PROPERTY     Property to include in propertyFilter.  May be repeated\n\
                    to create a multiproperty property list.\n\
                    Use \"\" to represent an empty property list\n\
    -C              Compare the result against old equivalent CIM operation.\n\
    -f QUERY FILTER String value for queryFilter parameterFilter.\n\
                    Note not supported in pegasus now. Default \"\".\n\
    -l QUERYFILTERLANG String value for queryFilterLanguage parameter.\n\
                    Note - This parameter not supported in Pegasus now.\n\
                    Default \"\" which tells server no value.\n\
    -r REPEAT       Integer count of number of times to repeat pull sequence.\n\
                    This is support for stress testing. Repeats are\n\
                    serial today, not concurrent.\n\
    -R              Reverse Exit code. Useful to add error tests to\n\
                    Makefiles where we can conduct tests that expect\n\
                    exception returns. If return code is zero this option\n\
                    sets it to nonzero and viceversa.\n\
    -y TIME         Pegasus client timeout in seconds.  Default is to not\n\
                    set a timeout. TIME is number of seconds until client\n\
                    times out the connection.\n\
    -X MAXOPERATIONS Integer. Maximum operations in enumeration sequence\n\
                    before close executed. Default is not used. If set\n\
                    close will be executed after that number of operations.\n\
                    Zero(0) Not allowed since has no meaning. One(1) means\n\
                    execute only open operation. This is not the same\n\
                    as the MAXOBJECTS parameters.\n\
    -W              Boolean. Treat Errors in Compare as warnings.\n\
\n";

/*************************************************************************
**
**    Structure to control use of a Uint32Arg based countdown
**    Allows countdown based on the initialized value of the counter
**    but only if the counter is initialized non-null.
**    This allows simple count-down testing with the option to
**    disable the test results simply by having the counter null.
**    Constructed from integer or another Uint32Arg.
**    The countdown is counter.ok which returns true and decrements the counter
**    for each call until it reaches zero where it returns false. If
**    counter is disabled it returns true.
**
**************************************************************************/
struct uint32Counter
{
    // The value component.
    Uint32Arg counter;
    /*
        Constructor. sets NULL
    */
    uint32Counter()
    {
        // do nothing.  Already setup as null
    }
    /*
        Set to value of Uint32 input. This sets nonNull
    */
    uint32Counter(Uint32 in)
    {
        counter.setValue(in);
    }
    /*
        construct from existing counter.
    */
    uint32Counter(Uint32Arg in)
    {
        counter = in;
    }
    ~uint32Counter()
    {
    }
    /*
        Return true if counter not used or value is gt zero and decrement.
        Allows single call test and decrement of counter
    */
    bool ok()
    {
        if (counter.isNull())
        {
            return true;
        }
        counter.setValue(counter.getValue() -1);
        if (counter.getValue() == 0)
        {
            return false;
        }
        else
            return true;
    }
    String toString()
    {
        return counter.toString();
    }
    // diagnostic only.
    void print()
    {
        cout << "uint32 counter state = " << counter.toString() << endl;;
    }
};

// Show the time and units.  We can expand this to adjust units for
// size of the time value (ex. us or ms or sec.
String _showTime(Uint64 time)
{
    String result;
    Uint64 fraction;
    char temp[100];
    // Display as sec, millisec, etc depending on size
    if (time >= 1000000)
    {
        fraction = time - ((time/1000000) * 1000000);
        sprintf(temp, "%lu.%lu sec", (long unsigned)time/1000000,
                (long unsigned)fraction/1000);
    }
    else if (time >= 1000)
    {
        fraction = time - ((time/1000000) * 1000000);
        sprintf(temp, "%lu.%lu Ms", (long unsigned)time/1000,
                (long unsigned)fraction/1000);
    }
    else
    {
        sprintf(temp, "%lu Us", (long unsigned)time);
    }
    result.append(temp);
    return result;
}

String _showTimeSint64(Sint64 time)
{
    String result;
    if (time < 0)
    {
        Uint64 ltime = -time;
        result.append("-");
        result.append(_showTime(ltime));
    }
    else
    {
        result.append(_showTime(Uint64(time)));
    }
    return result;
}

// if the timer option is set, display the time between the entries
void displayTimeDiff(const String& operationName,
    Stopwatch& pullTime,
    Stopwatch& enumTime,
    Stopwatch& elapsedPullTime,
    Uint32 responseSize)
{
    if (!timeOperation_opt)
    {
        return;
    }

    Uint64 enumResult = enumTime.getElapsedUsec();
    Uint64 pullResult = pullTime.getElapsedUsec();
    Uint64 elapsedPullResult = elapsedPullTime.getElapsedUsec();
    if (compare_opt)
    {
        // Display basic times and differences for the operation

        Sint64 diff = pullResult - enumResult;

        VCOUT3 << "PullStat " << operationName
             << " ops=" << _showTime(pullResult)
             << " elap=" << _showTime(elapsedPullResult)
             << " enum=" << _showTime(enumResult)
             << " diff=" << _showTimeSint64(diff)
             << " objectCount=" << responseSize
             << " for "  << pullCounter <<  " pull ops. "
             << " maxCtrs=" << maxObjectsOnOpen_opt << "/"
             << maxObjectsOnPull_opt;

        // display more detailed difference info between the
        // pull and original operations.
        if (diff != 0)
        {
            if (enumResult != 0)
            {
                cout << " " << ( (abs(diff) * 100)/enumResult) << "%"
                     << ((diff < 0)? " faster" : " slower") << endl;
            }
            else
            {
                cout << " No compare" << endl;
            }
        }
    }
    else    // no comparison requested so no enum times available.
    {
        VCOUT3 << "PullStat ops="
             << _showTime(pullResult)
             << " Pull Elapsed " << _showTime(elapsedPullResult)
             << " for " << pullCounter << " pull ops."
             << " maxCtrs=" << maxObjectsOnOpen_opt << "/"
             << maxObjectsOnPull_opt
             << endl;
    }
}
//------------------------------------------------------------------------------
//
// Class, Instance, Object, QualifierDecl Array Sorts
// Included from cimcli to allow sorting of returned arrays for comparisons
//
//------------------------------------------------------------------------------

static inline int _compareCIMNames(const CIMName& c1, const CIMName& c2)
{
    return String::compareNoCase(
        c1.getString(),
        c2.getString());
}

// Function to compare two object paths.   Returns a negative integer if p1 is
// lexographically less than p2, 0 if p1 and p2 are equal,
// and a positive integer otherwise.

// Compare values compares the value elements of two CIMObject Paths based
// on their types. Strings are compared directly, integers compared as
// integers, references are compared as CIMObjectPaths.

// Compare a single KeyBinding.  Compares names as CIMName and values based
// on type.
static int _compareObjectPaths(const CIMObjectPath& p1,
                               const CIMObjectPath& p2);

static int _compareKeyBinding(const CIMKeyBinding& kb1,
                              const CIMKeyBinding& kb2)
{
    int rtn;

    if ((rtn = _compareCIMNames(kb1.getName(), kb2.getName())) == 0)
    {
        switch (kb1.getType())
        {
            case CIMKeyBinding::REFERENCE:
                // Convert to paths and recurse through compare.
                try
                {
                    CIMObjectPath p1(kb1.getValue());
                    CIMObjectPath p2(kb2.getValue());
                    rtn = _compareObjectPaths(p1,p2);
                }
                catch (Exception&)
                {
                    // ignore if parsing fails
                    cerr << "Reference Path parsing failed" << endl;
                    rtn = 0;
                }
                break;

            case CIMKeyBinding::BOOLEAN:
                // Compare as no case strings
                rtn = String::compareNoCase(kb1.getValue(), kb1.getValue());
                break;

            case CIMKeyBinding::NUMERIC:
                // convert to numeric values and compare
                Uint64 uValue1;
                Sint64 sValue1;
                if (StringConversion::stringToUnsignedInteger(
                    kb1.getValue().getCString(),
                    uValue1))
                {
                    Uint64 uValue2;
                    if (StringConversion::stringToUnsignedInteger(
                        kb2.getValue().getCString(),
                        uValue2))
                    {
                        rtn = (uValue2 - uValue1);
                    }
                    else
                    {
                        // ignore error where we cannot convert both
                        rtn = 0;
                    }
                }
                // Next try converting to signed integer
                else if (StringConversion::stringToSignedInteger(
                    kb1.getValue().getCString(),
                    sValue1))
                {
                    Sint64 sValue2;
                    if (StringConversion::stringToSignedInteger(
                        kb2.getValue().getCString(),
                        sValue2))
                    {
                        rtn = (sValue2 - sValue1);
                    }
                    else
                    {
                        rtn = 0;
                    }
                }
                break;
            default:
                // no conversion required.  Compare as Strings.
                rtn = String::compare(kb1.getValue(), kb2.getValue());
                break;
        }
    }
    return rtn;
}

/*
    Compare two object paths. Compares by running compare on all of
    the component parts, host name, namespace name, class name, and the
    value component of each of the keybindings.
    NOTE: This assumes that the keybindings are previously sorted which is
    the case today for the Pegasus CIMObjectPath constructor.
    @return int negative if considered lt, 0 if eaual and positive if
    the compares result and of the components og p2 gt p1.
*/
static int _compareObjectPaths(const CIMObjectPath& p1,
    const CIMObjectPath& p2)
{
    int rtn;
    if ((rtn = String::compareNoCase(p1.getHost(),
                                     p2.getHost())) != 0)
    {
        return rtn;
    }
    if ((rtn = String::compareNoCase(p1.getNameSpace().getString(),
                                     p2.getNameSpace().getString())) != 0)
    {
        return rtn;
    }
    if ((rtn = _compareCIMNames(p1.getClassName(),
                                p2.getClassName())) != 0)
    {
        return rtn;
    }

    Array<CIMKeyBinding> kb1 = p1.getKeyBindings();
    Array<CIMKeyBinding> kb2 = p2.getKeyBindings();

    for (Uint32 i = 0 ; i < kb1.size() ; i++)
    {
        int rtn1;

        if ((rtn1 = _compareKeyBinding(kb1[i], kb2[i])) != 0)
        {
            return rtn1;
        }
    }
    // success, return successful
    return 0;
}
// Sort an array of object paths
static inline int _compareObjectPaths(const void* p1, const void* p2)
{
    const CIMObjectPath* pa = (const CIMObjectPath*)p1;
    const CIMObjectPath* pb = (const CIMObjectPath*)p2;
    return _compareObjectPaths(*pa , *pb);
}
static void _Sort(Array<CIMObjectPath>& x)
{
    CIMObjectPath* data = (CIMObjectPath*)x.getData();
    Uint32 size = x.size();

    if (size > 1)
    {
        qsort((void*)data, size, sizeof(CIMObjectPath), _compareObjectPaths);
    }
}

// Sort an array of classes using the class names as the comparator
static inline int _compareClasses(const void* p1, const void* p2)
{
    const CIMClass* c1 = (const CIMClass*)p1;
    const CIMClass* c2 = (const CIMClass*)p2;
    return String::compareNoCase(
        c1->getClassName().getString(),
        c2->getClassName().getString());
}
static void _Sort(Array<CIMClass>& x)
{
    CIMClass* data = (CIMClass*)x.getData();
    Uint32 size = x.size();

    if (size > 1)
    {
        qsort((void*)data, size, sizeof(CIMClass), _compareClasses);
    }
}

// Sort an Array of instances using the paths as the compare data.
// This compare used only by sort.
static int _compareInstances(const void* p1, const void* p2)
{
    const CIMInstance* c1 = (const CIMInstance*)p1;
    const CIMInstance* c2 = (const CIMInstance*)p2;

    return _compareObjectPaths(c1->getPath() , c2->getPath());
}

static void _Sort(Array<CIMInstance>& x)
{
    CIMInstance* data = (CIMInstance*)x.getData();
    Uint32 size = x.size();

    if (size > 1)
    {
        qsort((void*)data, size, sizeof(CIMInstance), _compareInstances);
    }
}
// Sort an Array of instances using the paths as the compare data.
static int _compareObjects(const void* p1, const void* p2)
{
    const CIMObject* c1 = (const CIMObject*)p1;
    const CIMObject* c2 = (const CIMObject*)p2;

    return _compareObjectPaths(c1->getPath(), c2->getPath());
}
static void _Sort(Array<CIMObject>& x)
{
    CIMInstance* data = (CIMInstance*)x.getData();
    Uint32 size = x.size();

    if (size > 1)
    {
        qsort((void*)data, size, sizeof(CIMInstance), _compareObjects);
    }
}

// Sleep for time determined by seconds
void doSleep(Uint32 seconds)
{
    if (seconds != 0)
    {
        VCOUT5 << "Sleeping " << seconds << " seconds between operations"
               << endl;
        sleep(seconds);
    }
}

/***************************************************************************
**
**      Compare objects and instance functions
**
****************************************************************************/

void clearHostAndNamespace(CIMObjectPath& p)
{

        p.setHost("");
        p.setNameSpace(CIMNamespaceName());

}

// KS-TODO - See other detailed compare above.  Should have only one
bool comparePath(CIMObjectPath& p1, CIMObjectPath p2, bool ignoreHost = true)
{
    if (ignoreHost)
    {
        CIMObjectPath p1x = p1;
        CIMObjectPath p2x = p2;
        if (ignoreHost)
        {
            clearHostAndNamespace(p1x);
            clearHostAndNamespace(p2x);
        }
        if (p1x != p2x)
        {
            VCOUT1 << "ERROR: pull path(NoHost)= " << p1x.toString() << endl
                 << "Enumerate path  (NoHost)= " << p2x.toString() << endl;
            return false;
        }
    }
    else
    {
        if (p1 != p2)
        {
            VCOUT1 << "ERROR: pull path=" << p1.toString() <<endl
                 << "Enumerate path  ="   << p2.toString()  << endl;
            return false;
        }
    }
    return true;
}


/* compare two instances. document all differences
*/
void clearHostAndNamespace(CIMInstance& inst)
{
    CIMObjectPath p1 = inst.getPath();
    p1.setHost("");
    p1.setNameSpace(CIMNamespaceName());
    inst.setPath(p1);
}

/*
    Compare two instances.  The ignorehost parameter allows us to
    clear the host and namespace fields since they will be different
    at least between enumerates and pull enumerates.

    // FUTURE: extend this so shows which is pull operation
*/
bool compareInstance(const String& s1, const String& s2,
    const CIMInstance& inst1,
    const CIMInstance& inst2,
    bool ignoreHost = true,
    bool verboseCompare=false)
{
    CIMInstance i1 = inst1.clone();
    CIMInstance i2 = inst2.clone();
    if (ignoreHost)
    {
        clearHostAndNamespace(i1);
        clearHostAndNamespace(i2);
    }
    if (i1.identical(i2))
    {
        return true;
    }

    if (i1.getPath() != i2.getPath())
    {
        cerr << "WARN: Paths Not identical " << s1 << " "
               << i1.getPath().toString()
               << " " << s2 << " " << i2.getPath().toString()
               << endl;
        warnings++;
    }

    else
    {
        cerr << "ERROR: Instances Not identical for path= "
            << i1.getPath().toString() << endl;

        if (i1.getQualifierCount() != i2.getQualifierCount())
        {
            cerr << "ERROR: Instance Qualifier Counts differ "
                << s1 << " " << i1.getQualifierCount() << " "
                << s2 << " " << i2.getQualifierCount() << endl;
        }
        bool switchInstances = false;
        if (i1.getPropertyCount() != i2.getPropertyCount())
        {
            cerr << "ERROR: PropertyCounts differ " << s1 << " "
                <<  i1.getPropertyCount() << " "
                << s2 << " " << i2.getPropertyCount() << endl;

            VCOUT1 << s1 << ": " << _toStringProperties(i1) << endl;
            VCOUT1 << s2 << ": " << _toStringProperties(i2) << endl;

            if (i2.getPropertyCount() > i1.getPropertyCount())
            {
                switchInstances = true;
            }
        }

        // Test individual properties for differences
        // Set so the s1,i1 definition has more properties
        String s11 = s1;
        String s21 = s2;
        if (switchInstances)
        {
            s11 = s2;
            s21 = s1;

            CIMInstance itemp = i1;
            i1 = i2;
            i2 = itemp;
        }

        for (Uint32 i = 0 ; i < i1.getPropertyCount() ; i++)
        {
            CIMConstProperty p1 = i1.getProperty(i);
            CIMConstProperty p2;
            bool found = false;

            // Note that property ordering may not be the same so we
            // search for the property in i2 by name.
            for (size_t j = 0 ; j < i2.getPropertyCount() ; j++)
            {
                p2 = i2.getProperty(j);
                if (p2.getName() == p1.getName())
                {
                    found = true;
                    break;
                }
            }

            // Property from p1 not fount in p2
            if (!found)
            {
                cerr << "Property " << p1.getName().getString()
                       << " not found in " << s21 << endl;
                continue;
            }

            // If properties not identical test for individual attribute
            // equality
            if (!p1.identical(p2))
            {
                if (p1.getName() != p2.getName())
                {
                    cerr << "ERROR: Property Names differ. Property " << i
                         << " "
                         << s11 << "= " << p1.getName().getString() << " "
                         << s21 << "= " << p2.getName().getString() << endl;
                }

                else if (p1.getType() != p2.getType())
                {
                    cerr << "ERROR: Property Types differ "
                         <<  p1.getName().getString() << " "
                         << s11 << "= " << p1.getType() << " "
                         << s21 << "= " << p2.getType() << endl;
                }
                else if(p1.isArray() != p2.isArray())
                {
                    cerr << "WARNING: Property Array flags differ "
                         << p1.getName().getString() << endl;
                }
                else if(p1.getArraySize() != p2.getArraySize())
                {
                    cerr << "ERROR: Property Array size parameters differ "
                         <<  p1.getName().getString() <<  " "
                         << s11 << "= " << p1.getArraySize()
                         << " " << s21 << "= " <<  p2.getArraySize() << endl;
                }
                else if (p1.getQualifierCount() != p2.getQualifierCount())
                {
                    cerr << "ERROR: Property Qualifier counts not identical "
                         << p1.getName().getString() <<  " "
                         << s11 << "= " << p1.getQualifierCount()
                         << " " << s21 << "= "
                         <<  p2.getQualifierCount() << endl;

                }
                else if (p1.getReferenceClassName() !=
                         p2.getReferenceClassName())
                {
                    cerr << "ERROR: ReferenceClass Names Differ "
                         << p1.getName().getString() <<  " "
                         << s11 << "= "
                         << p1.getReferenceClassName().getString()
                         << " "<< s21 << "= "
                         << p2.getReferenceClassName().getString() << endl;
                }

                else if(p1.getClassOrigin() != p2.getClassOrigin())
                {
                    cerr << "ERROR: ClassOrigin Names Differ "
                         << p1.getName().getString() <<  "  "
                         << s11 << "= "
                         << p1.getClassOrigin().getString()
                         << " "<< s21 << "= "
                         << p2.getClassOrigin().getString() << endl;
                }
                else
                {
                    CIMValue v1 = p1.getValue();
                    CIMValue v2 = p2.getValue();
                    if (!v1.equal(v2))
                    {
                        cerr << "ERROR: Property values differ for "
                             << p1.getName().getString() << " "
                             << s11 << "= " <<  v1.toString() << " "
                             << s21 << "= " << v2.toString() << endl;
                    }
                    else
                    {
                        cerr << "ERROR: Property " << p1.getName().getString()
                             << " Not identical in parameters"
                                " other than type, value, array type, and size "
                            << i1.getPath().toString() << endl;

                        // display the two properties that differ
                        cerr << "Display complete instances that differ"
                             << endl;
                        cerr << "Property 1" << endl;
                        PrintProperty(cout, p1);
                        cerr << "Property 2 " << endl;
                        PrintProperty(cout,p2);
                    }
                }

                if (verbose_opt > 0)
                {
                    cout << "Instance from " << s11 << endl;
                    PrintInstance(cout, i1);
                    cout << "Instance from " << s21 << endl;
                    PrintInstance(cout, i2);

                }
            }  // end if not equal
        }
    }
    return false;
}

// Test the return counts for over or under expectations and issue
// messages. Also displays the values before the tests.
// return true if the ERROR condition (i.e. too many) is returned.

bool displayAndTestReturns(const String& op, Boolean endOfSequence,
    Uint32 returnedCount, Uint32 expectedCount)
{
    // Display the returned counters and msg type
    VCOUT4 <<  op << " Requested = " << expectedCount
        << " Returned = " << returnedCount
        << " endOfSequence = " << _toCharP(endOfSequence)
        << endl;

    // KS_TODO - Account for NULL expectedCount.
    if (!endOfSequence && (returnedCount < expectedCount))
    {
        VCOUT2 << "WARN: Delivered less than requested instances. "
             << " Expected " << expectedCount
             << ". Delivered " << returnedCount << endl;
        warnings++;
        return false;
    }
    if (returnedCount > expectedCount)
    {
        VCOUT1 << "ERROR: Delivered more objects than requested."
                 << " Expected " << expectedCount
                 << ". Delivered " << returnedCount << endl;
        return true;
    }

    // unreachable
    PEGASUS_UNREACHABLE(return true);
}

void displayInstances(const Array<CIMInstance>& instances)
{
    for (Uint32 i = 0; i < instances.size(); i++)
    {
        cout << i << ". " << instances[i].getPath().toString()
             << endl;
    }
}

void displayObjectPaths(const Array<CIMObjectPath>& paths)
{
    for (Uint32 i = 0; i < paths.size(); i++)
    {
        cout << i << ". " << paths[i].toString()
             << endl;
    }
}

// Compare instances between two arrays and display any differences.
// return false if differences exist.
Boolean compareInstances(const String& s1, const String s2,
    Array<CIMInstance>& inst1,
    Array<CIMInstance>& inst2,
    bool verbose = false)
{
    VCOUT6 << "Comparing Arrays of Instances" << endl;
    Boolean rtn = true;
    if (inst1.size() != inst2.size())
    {
        VCOUT1 << "Instance Arrays count mismatch "
            << s1 << "= " <<  inst1.size()
            << " instances " << s2 << "= " << inst2.size() << " instances"
            << endl;
        errors++;
        rtn = (errorsAsWarnings_opt)? true : false;
    }

    // sort the two arrays since there is no guarantee that they
    // returned with same ordering.

    _Sort(inst1);
    _Sort(inst2);
    //// TODO. We need to account that if there was a size difference
    //// above inst1 may not be the largest and that we may not have
    //// equal paths.
    for (Uint32 i = 0 ; i < inst1.size() ; i++)
    {
        bool localRtn = compareInstance(s1, s2,
            inst1[i], inst2[i], true, verbose);

        if (!localRtn)
        {
            errors++;
            rtn = (errorsAsWarnings_opt)? true : false;
        }
    }
    VCOUT4 << "compareInstances " << (rtn? "OK" : "Failed") << endl;
    return rtn;
}

/*
    Alternate compare between an instance array and object array.
    Required because the original operations returned objects and the
    new operations return instances.
*/
Boolean compareInstances(const String& s1, const String s2,
    Array<CIMInstance>& instances,
    Array<CIMObject>& objects,
    bool verbose = false)
{
    VCOUT6 << "Comparing Instances to Objects" << endl;
    Boolean rtn = true;

    if (instances.size() != objects.size())
    {
        VCOUT1 << s1 << " ERROR: count mismatch of Instances/Objects "
            << "Pull sequence " <<  instances.size()
            << " " << s2 << " " << objects.size()
            << endl;
        errors++;
        rtn = false;
    }

    // sort the two arrays since there is no guarantee that the
    // returned with same ordering.
    _Sort(instances);
    _Sort(objects);
    for (Uint32 i = 0 ; i < instances.size() ; i++)
    {
        CIMInstance instLocal = (CIMInstance)objects[i];
        bool localRtn = compareInstance(s1, s2,
                                        instances[i],
                                        instLocal,
                                        true, verbose);
        if (!localRtn)
        {
            errors++;
            if (errorsAsWarnings_opt)
            {
                rtn = true;
            }
            else
            {
                rtn = false;
            }
        }
    }
    VCOUT4 << "compareInstances " << (rtn? "OK" : "Failed") << endl;
    return rtn;
}


// Compare instances between two arrays and display any differences.
// return false if differences exist.

Boolean compareObjectPaths(
    const String& s1,
    const String s2,
    Array<CIMObjectPath>& p1,
    Array<CIMObjectPath>& p2,
    bool ignoreHost = true)
{
    VCOUT6 << "Comparing ObjectPaths" << endl;
    Boolean rtn = true;
    if (p1.size() != p2.size())
    {
        VCOUT1 << s1 << " ERROR: count mismatch of ObjectPaths "
            << s2 << ". Pull sequence rtnd " <<  p1.size()
            << " " << ". Old Operation rtnd " << p2.size()
            << endl;
        errors++;
        for (Uint32 i = 0; i < p1.size(); i++)
        {
            cout << "Pull " << i << " " << p1[i].toString() << endl;
        }
        cout << endl;
        for (Uint32 i = 0; i < p2.size(); i++)
        {
            cout << "Orig " << i << " " << p2[i].toString() << endl;
        }

        rtn = false;
    }

    // sort the paths to assure that same order.
    _Sort(p1);
    _Sort(p2);

    for (Uint32 i = 0 ; i < p1.size() ; i++)
    {
        bool localRtn = comparePath(p1[i], p2[i], ignoreHost);
        if (!localRtn)
        {
            rtn = false;
        }
    }

    VCOUT4 << "compare paths " << (rtn? "OK" : "Failed") << endl;
    if (errorsAsWarnings_opt)
    {
        if (!rtn)
        {
            rtn = true;
        }
    }
    return rtn;
}

void displayRtnSizes(const char * op, Uint32Arg& maxObjectCount,
    Uint32 returnedSize, Boolean endOfSequence)
{
    VCOUT4 <<  op << " Requested " << maxObjectCount.toString()
        << " Returned " << returnedSize
        << " endOfSequence = " << _toCharP(endOfSequence)
        << endl;
}
/******************************************************************************
**
**   Test Execution functions. There is a function for each Open.
**   They conduct a complete test (open, pull (possibly close) based on
**   input parameters.  Each also includes a test of the results against
**   an equivalent non-pull operation to be sure getting same number and
**   same information.
**
*******************************************************************************/

/*
    Local function to execute pullInstance paths until either the maxcount
    is exceeded or the end of sequence received from the host.
    @param openOpName char * name of the open Operation driving the pull
    @param resultArray CIMObjectPath where results are accumulated
    @param maxObjectCount Uint32Arg where maxObjectCount for the pulls.
    @return returns endOfSequence. This may be either true or false depending
    on how the operation ended (exceeding count or receive endOfSequence
    = true from host
*/
bool pullInstancePaths(CIMClient& client,
    const String& openOpName,
    Array<CIMObjectPath>& resultArray,
    Uint32 maxObjectCount,
    uint32Counter& maxOperationsBeforeCloseCounter,
    CIMEnumerationContext& enumerationContext,
    Stopwatch& timer)
{
    Boolean endOfSequence = false;
    while (!endOfSequence  && maxOperationsBeforeCloseCounter.ok())
    {
        pullCounter++;
        VCOUT4 << "Issue pullInstancePaths for " << openOpName
             << " maxObjectCount = " << maxObjectsOnPull_opt
             <<  endl;
        timer.start();
        Array<CIMObjectPath> cimObjectPathsPull = client.pullInstancePaths(
            enumerationContext,
            endOfSequence,
            maxObjectsOnPull_opt);
        timer.stop();
        displayAndTestReturns("PullInstancePaths",endOfSequence,
            cimObjectPathsPull.size(), maxObjectCount);

        doSleep(sleep_opt);

        resultArray.appendArray(cimObjectPathsPull);
    }
    // Terminated the previous loop early (max count) so issue a close.
    if (!endOfSequence)
    {
        VCOUT4 << "Issue closeEnumeration Operation for "
             << openOpName << ". Total Received count " << resultArray.size()
             << " endOfSequence before close=" << _toCharP(endOfSequence)
             << endl;
        timer.start();
        client.closeEnumeration(enumerationContext);
        timer.stop();
    }
    else
    {
        VCOUT4 << "Total ObjectPaths count=" << resultArray.size()
             << "endOfSequence=" << _toCharP(endOfSequence) << endl;
        if (verbose_opt >= 7)
        {
            displayObjectPaths(resultArray);
        }
    }
    return endOfSequence;
}

bool pullInstancesWithPath(CIMClient& client,
    const String& openOpName,
    Array<CIMInstance>& resultArray,
    Uint32 maxObjectCount,
    uint32Counter& maxOperationsBeforeCloseCounter,
    CIMEnumerationContext& enumerationContext,
    Stopwatch& timer)
{
    Boolean endOfSequence = false;
    while (!endOfSequence  && maxOperationsBeforeCloseCounter.ok())
    {
        pullCounter++;
        VCOUT4 << "Issue pullInstancesWithPath for " << openOpName
             << " maxObjectCount = " << maxObjectsOnPull_opt
             <<  endl;
        timer.start();
        Array<CIMInstance> cimInstancesPull = client.pullInstancesWithPath(
            enumerationContext,
            endOfSequence,
            maxObjectsOnPull_opt);
        timer.stop();
        displayAndTestReturns("PullInstancesWithPath", endOfSequence,
            cimInstancesPull.size(), maxObjectCount);
        doSleep(sleep_opt);
        resultArray.appendArray(cimInstancesPull);
    }
    // Terminated the previous loop early (max count) so issue a close.
    if (!endOfSequence)
    {
        VCOUT4 << "Issue closeEnumeration Operation for "
             << openOpName << ". Total Received count " << resultArray.size()
             << " endOfSequence before close=" << _toCharP(endOfSequence)
             << endl;
        timer.start();
        client.closeEnumeration(enumerationContext);
        timer.stop();
    }
    else
    {
        VCOUT4 << "Total Instances count= " << resultArray.size()
          << " endOfSequence= " << _toCharP(endOfSequence)<< endl;
        if (verbose_opt >= 7)
        {
            displayInstances(resultArray);
        }
    }
    return endOfSequence;
}
/*
    Test all instances in array for valid instancepath
*/
Boolean validateInstancePath(Array<CIMInstance> instances)
{
    Boolean rtn = true;
    for (Uint32 i = 0 ; i < instances.size() ; i++)
    {
        CIMObjectPath p = instances[i].getPath();
        if (p.getHost() == "" && p.getNameSpace() == CIMNamespaceName())
        {
            VCOUT2  << "Error: no Host or Namespace  in " << p.toString()
                << endl;
            errors++;
            rtn = false;
        }
        const Array<CIMKeyBinding> kb = p.getKeyBindings();
        if (kb.size() == 0)
        {
            VCOUT2  << "Warning: no Keys in path " << p.toString()
                << endl;
            warnings++;
            rtn = true;
        }
    }
    return rtn;
}
/*
    Test function for enumerate instances sequence.  Does the complete
    sequence based on input parameters.  Some client call parameters such
    as includeClassOrigin are fixed today.
*/
void testPullEnumerateInstances(CIMClient& client, CIMNamespaceName nameSpace,
    String ClassName )
{
    // initialize deliveryCounter.
    uint32Counter maxOperationsBeforeCloseCounter(
        maxOperationsBeforeCloseCount_opt);

    try
    {
        String operationName = "openEnumerateInstances";
        Boolean includeClassOrigin = requestClassOrigin_opt;

        Boolean endOfSequence = false;
        CIMPropertyList propertyList(propertyList_opt);
        Boolean deepInheritance = deepInheritance_opt;

        Array<CIMInstance> pulledInstances;
        CIMEnumerationContext enumerationContext;

        VCOUT4 << "Issue openEnumerateInstances " << ClassName
               << " namespace=" << nameSpace.getString()
               << " deepInheritance=" << _toString(deepInheritance)
               << " classOrigin=" << _toString(includeClassOrigin)
               << " propertyList=" << _toString(propertyList)
               << " timeout=" << timeout_opt.toString()
               << " filterQueryLanguage=" << filterQuery_opt
               << " filterQuery_opt=" << filterQuery_opt
               << " continueOnError=" << _toString(continueOnError_opt)
               << " maxObjectsOnOpen=" << maxObjectsOnOpen_opt
               << " maxCountBeforeClose for Operation "
                   << maxOperationsBeforeCloseCounter.toString()
               << endl;

        Stopwatch pullTime;
        Stopwatch elapsedPullTime;
        Stopwatch enumTime;

        pullTime.start();
        elapsedPullTime.start();

        pulledInstances = client.openEnumerateInstances(
            enumerationContext,
            endOfSequence,
            nameSpace,
            ClassName,
            deepInheritance,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage_opt,
            filterQuery_opt,
            timeout_opt,
            continueOnError_opt,
            maxObjectsOnOpen_opt);

        pullTime.stop();

        displayAndTestReturns(operationName, endOfSequence,
            pulledInstances.size(), maxObjectsOnOpen_opt);

        doSleep(sleep_opt);

        // issue pulls to get remaning objects. Note that we may close
        // early depending on the maxOperationsBeforeCloseCounter
        if (!endOfSequence)
        {
            endOfSequence = pullInstancesWithPath(client, operationName,
                pulledInstances,
                maxObjectsOnPull_opt,
                maxOperationsBeforeCloseCounter,
                enumerationContext,
                pullTime);
        }

        elapsedPullTime.stop();

        VCOUT6 << "Pull Sequence Complete" << endl;

        // if the compare opt was set, get with enumerate and compare
        if (compare_opt && endOfSequence)
        {
            VCOUT1 << "Comparing Result to enumerateInstances" << endl;
            enumTime.start();
            Array<CIMInstance> enumeratedInstances = client.enumerateInstances(
                nameSpace,
                ClassName,
                deepInheritance,
                true,                        // localonly always true
                false,                       // includeQualifiers always false
                includeClassOrigin,
                propertyList);

            enumTime.stop();

            VCOUT6 << "EnumerateInstances returned "
                   << enumeratedInstances.size()
                   << " instances" << endl;

            compareInstances("PullnstancesWithPath",
                             "EnumerateInstances  ",
                             pulledInstances, enumeratedInstances);
        }
        if (!validateInstancePath(pulledInstances))
        {
            VCOUT1 << "Error in path return of PullInstancesWithPath" << endl;
        }

        displayTimeDiff(operationName, pullTime, enumTime, elapsedPullTime,
                        pulledInstances.size());
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in testEnumerationWithPull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: in testEnumerationWithPull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
}

/*
    Test function for enumerate instancepaths sequence.  Does the complete
    sequence based on input parameters.
*/
void testPullEnumerationInstancePaths(CIMClient& client,
    CIMNamespaceName nameSpace,
    String ClassName )
{
    // initialize deliveryCounter.
    uint32Counter maxOperationsBeforeCloseCounter(
        maxOperationsBeforeCloseCount_opt);
    String operationName = "openEnumerateInstancePaths";
    try
    {
        Uint32 maxObjectCount = maxObjectsOnOpen_opt;
        Boolean endOfSequence = false;
        Uint32Arg operationTimeout(timeout_opt);
        String filterQueryLanguage = String::EMPTY;
        String filterQuery = String::EMPTY;
        Array<CIMObjectPath> cimObjectPaths;
        CIMEnumerationContext enumerationContext;

        VCOUT4 << "Issue openEnumerateInstancesPaths. maxObjectCount = "
            << maxObjectCount
            << ". timeout = " << operationTimeout.toString()
            << " maxCountBeforeClose for Operation "
                << maxOperationsBeforeCloseCounter.toString()
            << endl;

        Stopwatch pullTime;
        Stopwatch elapsedPullTime;
        Stopwatch enumTime;

        pullTime.start();
        elapsedPullTime.start();

        cimObjectPaths = client.openEnumerateInstancePaths(
            enumerationContext,
            endOfSequence,
            nameSpace,
            ClassName,
            filterQueryLanguage_opt,
            filterQuery_opt,
            operationTimeout,
            continueOnError_opt,
            maxObjectCount);

        pullTime.stop();

        displayAndTestReturns(operationName, endOfSequence,
            cimObjectPaths.size(), maxObjectCount);

        doSleep(sleep_opt);

        // issue pulls to get remaning objects. Note that we may close
        // early depending on the maxOperationsBeforeCloseCounter

        if (!endOfSequence)
        {
            endOfSequence = pullInstancePaths(client, operationName,
                cimObjectPaths, maxObjectsOnPull_opt,
                maxOperationsBeforeCloseCounter,
                enumerationContext,
                pullTime);
        }

        elapsedPullTime.stop();

        // if the compare opt was set, get with enumerate and comapre
        if (compare_opt && endOfSequence)
        {
            enumTime.start();
            // get the objects with monolithic operation
            Array<CIMObjectPath> cimObjectPaths2 =
                client.enumerateInstanceNames(
                    nameSpace,
                    ClassName);

            enumTime.stop();

            compareObjectPaths("PullEnumerateInstancePaths",
                         "enumerateInstanceNames",
                         cimObjectPaths, cimObjectPaths2);
        }
        displayTimeDiff(operationName, pullTime, enumTime, elapsedPullTime,
                        cimObjectPaths.size());
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in testEnumerationPaths With Pull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: in testEnumerationPaths With Pull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
}

void testPullReferenceInstances(CIMClient& client, CIMNamespaceName nameSpace,
    String objectPath )
{
    // initialize deliveryCounter.
    uint32Counter maxOperationsBeforeCloseCounter(
        maxOperationsBeforeCloseCount_opt);
    String operationName = "openReferenceInstances";
    try
    {
        CIMObjectPath targetObjectPath(objectPath);
        String role = "";
        CIMName resultClass = CIMName();
        Boolean includeClassOrigin = requestClassOrigin_opt;
        CIMPropertyList cimPropertyList(propertyList_opt);
        CIMEnumerationContext enumerationContext;
        Boolean endOfSequence = false;

        // TODO - Fix up function to test creation of cimobjectPath from
        // input since this can be such a mess.
        // KS_TODO this display not showing everything
        VCOUT4 << "Issue openReferencesInstances for " << objectPath
            << " maxObjects " << maxObjectsOnOpen_opt
            << " Instances."
            << " timeout " << timeout_opt.toString()
            << " maxCountBeforeClose for Operation "
                << maxOperationsBeforeCloseCounter.toString()
        << endl;

        Stopwatch pullTime;
        Stopwatch elapsedPullTime;
        Stopwatch enumTime;

        pullTime.start();
        elapsedPullTime.start();

        Array<CIMInstance> pulledInstances = client.openReferenceInstances(
            enumerationContext,
            endOfSequence,
            nameSpace,
            targetObjectPath,
            resultClass,
            role,
            includeClassOrigin,
            cimPropertyList,
            filterQueryLanguage_opt,
            filterQuery_opt,
            timeout_opt,
            continueOnError_opt,
            maxObjectsOnOpen_opt
            );

        pullTime.stop();

        displayAndTestReturns(operationName, endOfSequence,
            pulledInstances.size(), maxObjectsOnOpen_opt);

        doSleep(sleep_opt);

        // issue pulls to get remaning objects. Note that we may close
        // early depending on the maxOperationsBeforeCloseCounter
        if (!endOfSequence)
        {
            endOfSequence = pullInstancesWithPath(client, operationName,
                                              pulledInstances,
                                              maxObjectsOnPull_opt,
                                              maxOperationsBeforeCloseCounter,
                                              enumerationContext,
                                              pullTime);
        }

        elapsedPullTime.stop();

        // if the compare opt was set, get with enumerate and compare
        if (endOfSequence && compare_opt)
        {
            Boolean includeClassQualifiers = false;

            enumTime.start();
            Array<CIMObject> refdInstances = client.references(
                nameSpace,
                targetObjectPath,
                resultClass,
                role,
                includeClassQualifiers,
                includeClassOrigin,
                cimPropertyList);

            enumTime.stop();

            compareInstances(operationName, "references",
                             pulledInstances, refdInstances);
        }
        displayTimeDiff(operationName, pullTime, enumTime, elapsedPullTime,
                        pulledInstances.size());
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in testEnumerationWithPull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: in testEnumerationWithPull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
}

void testPullReferenceInstancePaths(CIMClient& client,
    CIMNamespaceName nameSpace,
    String objectPath )
{
    // initialize deliveryCounter.
    uint32Counter maxOperationsBeforeCloseCounter(
        maxOperationsBeforeCloseCount_opt);
    String operationName = "openReferencePaths";
    try
    {
        CIMObjectPath targetObjectPath(objectPath);
        String role = "";
        CIMName resultClass = CIMName();
        Array<CIMObjectPath> cimObjectPaths;

        CIMEnumerationContext enumerationContext;
        Boolean endOfSequence;

        VCOUT4 << "Issue openReferencePaths for instance paths. "
            << " maxObjectCount = " << maxObjectsOnOpen_opt
            << " timeout " << timeout_opt.toString()
            << " maxCountBeforeClose for Operation "
                << maxOperationsBeforeCloseCounter.toString()
            << endl;

        Stopwatch pullTime;
        Stopwatch elapsedPullTime;
        Stopwatch enumTime;

        pullTime.start();
        elapsedPullTime.start();

        cimObjectPaths = client.openReferenceInstancePaths(
            enumerationContext,
            endOfSequence,
            nameSpace,
            targetObjectPath,
            resultClass,
            role,
            filterQueryLanguage_opt,
            filterQuery_opt,
            timeout_opt,
            continueOnError_opt,
            maxObjectsOnOpen_opt
            );

        pullTime.stop();
        displayAndTestReturns(operationName, endOfSequence,
            cimObjectPaths.size(), maxObjectsOnOpen_opt);

        doSleep(sleep_opt);

        Uint32 maxObjectCount = maxObjectsOnPull_opt;
        // issue pulls to get remaning objects. Note that we may close
        // early depending on the maxOperationsBeforeCloseCounter
        if (!endOfSequence)
        {
            endOfSequence = pullInstancePaths(client, operationName,
                                              cimObjectPaths, maxObjectCount,
                                              maxOperationsBeforeCloseCounter,
                                              enumerationContext,
                                              pullTime);
        }

        elapsedPullTime.stop();

        // if the compare opt was set, get with enumerate and compare
        if (endOfSequence && compare_opt)
        {
            enumTime.start();
            Array<CIMObjectPath> cimObjectPathsOrig =
                client.referenceNames(
                    nameSpace,
                    targetObjectPath,
                    resultClass,
                    role);

            enumTime.stop();

            compareObjectPaths(operationName,
                "referenceNames", cimObjectPaths, cimObjectPathsOrig);
        }
        displayTimeDiff(operationName, pullTime, enumTime, elapsedPullTime,
                        cimObjectPaths.size());
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in testReferencePaths "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: in testReferencePaths "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
}

void testPullAssociatorInstances(CIMClient& client, CIMNamespaceName nameSpace,
    String objectPath )
{
    // initialize deliveryCounter.
    uint32Counter maxOperationsBeforeCloseCounter(
        maxOperationsBeforeCloseCount_opt);

    String operationName = "openAssociatorInstances";
    try
    {
        CIMObjectPath targetObjectPath(objectPath);
        String role = "";
        String resultRole = "";
        CIMName resultClass = CIMName();
        CIMName assocClass = CIMName();
        Boolean includeClassOrigin = false;
        Uint32 maxObjectCount = maxObjectsOnOpen_opt;
        CIMPropertyList cimPropertyList(propertyList_opt);
        CIMEnumerationContext enumerationContext;
        Boolean endOfSequence = false;

        // TODO - Fix up function to test creation of cimobjectPath from
        // input since this can be such a mess.

        VCOUT4 << "Issue openAssociationInstances for " << objectPath
            << maxObjectCount << " Instances."
            << " timeout " << timeout_opt.toString()
            << " maxCountBeforeClose for Operation "
                << maxOperationsBeforeCloseCounter.toString()
            << endl;

        Stopwatch pullTime;
        Stopwatch elapsedPullTime;
        Stopwatch enumTime;

        pullTime.start();
        elapsedPullTime.start();

        Array<CIMInstance> cimInstances = client.openAssociatorInstances(
            enumerationContext,
            endOfSequence,
            nameSpace,
            targetObjectPath,
            assocClass,
            resultClass,
            role,
            resultRole,
            includeClassOrigin,
            cimPropertyList,
            filterQueryLanguage_opt,
            filterQuery_opt,
            timeout_opt,
            continueOnError_opt,
            maxObjectsOnOpen_opt
            );

        pullTime.stop();
        displayAndTestReturns(operationName, endOfSequence,
            cimInstances.size(), maxObjectCount);

        doSleep(sleep_opt);

        // issue pulls to get remaning objects. Note that we may close
        // early depending on the maxOperationsBeforeCloseCounter

        if (!endOfSequence)
        {
            endOfSequence = pullInstancesWithPath(client, operationName,
                              cimInstances, maxObjectCount,
                              maxOperationsBeforeCloseCounter,
                              enumerationContext,
                              pullTime);
        }

        elapsedPullTime.stop();

        // if the compare opt was set, get with enumerate and compare
        if (endOfSequence && compare_opt)
        {
            enumTime.start();
            Boolean includeClassQualifiers = false;
            Array<CIMObject> cimObjectsOrig = client.associators(
                nameSpace,
                targetObjectPath,
                assocClass,
                resultClass,
                role,
                resultRole,
                includeClassQualifiers,
                includeClassOrigin,
                cimPropertyList);

            enumTime.stop();

            // note that this actually calls compare of instance to object
            compareInstances("PullAssociators", "associators",
                             cimInstances, cimObjectsOrig);
        }
        displayTimeDiff(operationName, pullTime, enumTime, elapsedPullTime,
                        cimInstances.size());
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in testEnumerationWithPull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: in testEnumerationWithPull "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
}


void testPullAssociatorInstancePaths(CIMClient& client,
    CIMNamespaceName nameSpace,
    String objectPath )
{
    // initialize deliveryCounter.
    uint32Counter maxOperationsBeforeCloseCounter(
        maxOperationsBeforeCloseCount_opt);

    String operationName = "openAssociatorInstancePaths";
    try
    {
        CIMObjectPath targetObjectPath(objectPath);
        String role = "";
        String resultRole = "";
        CIMName assocClass = CIMName();
        CIMName resultClass = CIMName();

        Array<CIMObjectPath> cimObjectPaths;

        CIMEnumerationContext enumerationContext;
        Boolean endOfSequence;

        VCOUT4 << "Issue openAssociatorInstancePaths for instance paths. "
            << " maxObjectCount = " << maxObjectsOnOpen_opt
            << " timeout " << timeout_opt.toString()
            << " maxCountBeforeClose for Operation "
                << maxOperationsBeforeCloseCounter.toString()
            << endl;

        Stopwatch pullTime;
        Stopwatch elapsedPullTime;
        Stopwatch enumTime;

        pullTime.start();
        elapsedPullTime.start();

        cimObjectPaths = client.openAssociatorInstancePaths(
            enumerationContext,
            endOfSequence,
            nameSpace,
            targetObjectPath,
            assocClass,
            resultClass,
            role,
            resultRole,
            filterQueryLanguage_opt,
            filterQuery_opt,
            timeout_opt,
            continueOnError_opt,
            maxObjectsOnOpen_opt
            );

        pullTime.stop();
        displayAndTestReturns(operationName, endOfSequence,
            cimObjectPaths.size(), maxObjectsOnOpen_opt);

        doSleep(sleep_opt);

        // issue pulls to get remaning objects. Note that we may close
        // early depending on the maxOperationsBeforeCloseCounter
        if (!endOfSequence)
        {
            endOfSequence = pullInstancePaths(client, operationName,
                                              cimObjectPaths,
                                              maxObjectsOnPull_opt,
                                              maxOperationsBeforeCloseCounter,
                                              enumerationContext,
                                              pullTime);
        }

        elapsedPullTime.stop();

            // if the compare opt was set, get with enumerate and comapre
        if (endOfSequence && compare_opt)
        {
            // get the objects with monolithic operation
            enumTime.start();

            Array<CIMObjectPath> cimObjectPathsOrig =
                client.associatorNames(
                    nameSpace,
                    targetObjectPath,
                    assocClass,
                    resultClass,
                    role,
                    resultRole);

            enumTime.stop();

            compareObjectPaths("PullAssociatorInstancePaths",
                         "associatorNames",
                         cimObjectPaths, cimObjectPathsOrig);
        }
        displayTimeDiff(operationName, pullTime, enumTime, elapsedPullTime,
                        cimObjectPaths.size());
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in testReferencePaths "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: in testAssociatorPaths "
            << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }
}

// find all classes in the defined namespace that have instances and
// execute enumerate enumeratepaths and the 4 associator operations on them
// The other parameters (timing, compare, apply)
void testAllClasses(CIMClient& client, CIMNamespaceName ns,
                    Boolean skipEmpty = true)
{
    // get all classes in the namespace
    Array<CIMName> cn =  client.enumerateClassNames(ns, CIMName(), true);
    VCOUT4 << "Start testAllClasses " << "Enumerated " << cn.size()
           << " classNames" << endl;

    // Enumerate remove any that do not return instanceNames

    Uint32 instanceCount = 0;
    for (Uint32 i = 0; i < cn.size() ; i++)
    {
        Array<CIMObjectPath> instanceNames = client.enumerateInstanceNames(ns,
            cn[i]);

        // remove all classnames that do not have any instances.
        if (skipEmpty && instanceNames.size() == 0)
        {
            cn.remove(i);
        }
        else
        {
            instanceCount += instanceNames.size();
            VCOUT5 << cn[i].getString() << " contains "
                 << instanceNames.size() << " instances" << endl;
        }
    }
    VCOUT4 << "Testing " << cn.size() << " classes with instaces. Total "
        << instanceCount << " instances." << endl;

    Uint32 counter;
    Stopwatch testTime;

    //
    //  Test Pull EnumerateInstances for all classes with instances
    //
    VCOUT4 << "Test Pull Enumerate Instances for each class" << endl;
    testTime.start();
    for (Uint32 i = 0 ; i < cn.size() ; i++)
    {
        VCOUT5 << "Test for class "<< cn[i].getString() << endl;
        testPullEnumerateInstances(client, ns, cn[i].getString());
        counter++;
    }
    testTime.stop();
    VCOUT4 << "Tested PullEnumerationInstances of " << counter
           << " classes in " << _showTime(testTime.getElapsed()) << endl;

    //
    // Test PullEnumerationInstancePaths
    //

    VCOUT5 << "Test Pull Enumerate Instance Paths" << endl;
    testTime.reset();
    testTime.start();
    counter = 0;
    for (Uint32 i = 0 ; i < cn.size() ; i++)
    {
        VCOUT5 << "Test for class "<< cn[i].getString() << endl;
        testPullEnumerationInstancePaths(client, ns, cn[i].getString());
        counter++;
    }
    testTime.stop();
    VCOUT4 << "Tested PullEnumerationInstances of " << counter
           << " classes in " << _showTime(testTime.getElapsed()) << endl;

    // Test association requests against all instances in the system
    // that are not association classes.
    VCOUT4 << "Association Tests" << endl;
    counter = 0;
    for (Uint32 i = 0 ; i < cn.size() ; i++)
    {
        Array<CIMObjectPath> instanceNames = client.enumerateInstanceNames(ns,
            cn[i]);

        for (Uint32 j = 0; j < instanceNames.size() ; j++)
        {
            CIMObjectPath objectName = instanceNames[j];
            CIMClass cl = client.getClass(ns, objectName.getClassName());
            if (!cl.isAssociation())
            {
                counter++;
                VCOUT5 << "Associator Tests for "
                       << objectName.toString()
                       << endl;
                // Modify test functions so the call is CIMObjectPath,
                // not String.
                testPullReferenceInstances(
                    client,ns, objectName.toString());
                testPullReferenceInstancePaths(
                    client,ns,objectName.toString());
                testPullAssociatorInstances(
                    client,ns,objectName.toString() );
                testPullAssociatorInstancePaths(
                    client,ns,objectName.toString());
            }
        }
    }
    VCOUT4 << "Association Tests Complete " << counter << "association calls"
           << endl;
}

// Parse Hostname input into name and port number
Boolean parseHostName(const String arg, String& hostName, Uint32& port)
{
    port = 5988;
    hostName = arg;

    Uint32 pos;
    if (!((pos = arg.reverseFind(':')) == PEG_NOT_FOUND))
    {
        Uint64 temp;
        if (StringConversion::decimalStringToUint64(
            hostName.subString(pos+1).getCString(), temp)
            &&
            StringConversion::checkUintBounds(temp,CIMTYPE_UINT32))
        {
            hostName.remove(pos);
            port = (Uint32)temp;
        }
        else
        {
            return false;
        }
    }
    return true;
}

/******************************************************************************
**
**  Main - parse input options and call function defined by
**         OperationType parameter
**
******************************************************************************/
int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    const char* arg0 = argv[0];

    //  Save input arg list for display
    String argvParams;
    for (int i = 1; i < argc ; i ++)
    {
        argvParams.append(" ");
        argvParams.append(argv[i]);
    }
    /*
        array of property names that is applied at the
        propertylist for the operation.
    */
    Array<CIMName> propertyList_builder;

    /*
        Analyze and set all input options. uses getopt so, by definition
        all options are proceeded by single -. We make a single exception
        to allow --help.
    */
    int opt;
    while ((opt = getopt(argc, argv,
                         "c:hdVv:n:H:u:p:t:M:N:CTf:l:P:r:X:xR-:s:y:")) != -1)
    {
        switch (opt)
        {
            case 'c':           // set objecName
            {
            objectName_opt = optarg;
            break;
            }

            case 'h':           // -h option. Print usage
            {
                fprintf(stderr, (char*)USAGE, argv[0]);
                exit(0);
            }
            case '-':          // special case --help argument
            {
                if (strcmp(optarg, "help") != 0)
                {
                    printf("ERROR: invalid option: --%s\n", optarg);
                    exit(1);
                }
                else
                {
                    fprintf(stderr, (char*)USAGE, argv[0]);
                    exit(0);
                }
            }

            case 'V':               // KS_TODO NOT USED
            {
                printf("%s\n", "Don't know right now");
                exit(0);
            }

            case 'd':               // set deepInheritance = false;
            {
                deepInheritance_opt = false;
                break;
            }
            case 'o':               // set deepInheritance = false;
            {
                requestClassOrigin_opt = true;
                break;
            }
            case 'y' :    // Set client timeout value in seconds
            {
                if (strcasecmp("null", optarg) != 0)
                {
                    clientTimeoutSeconds_opt.setValue(stringToUint32(optarg));
                }
                break;
            }
            case 'v':               // verbose display with integer
            {
                verbose_opt = stringToUint32(optarg);
                if (verbose_opt > 7)
                {
                    cerr << "INPUT ERROR: max verbose level is 5" << endl;
                    exit(1);
                }
                break;
            }
            case 'P':               // set property list
            {
                if (strlen(optarg) == 0)
                {
                    propertyList_builder.clear();
                }
                else
                {
                    propertyList_builder.append(optarg);
                }
                propertyList_opt.set(propertyList_builder);
                break;
            }
            case 'n':               // set namespace
            {
                namespace_opt = optarg;
                break;
            }
            case 'H':           // set hostname
            {
                host_opt = optarg;
                break;
            }
             case 'u':          // set user name
            {
                user_opt = optarg;
                break;
            }
            case 'p':           // set password
            {
                password_opt = optarg;
                break;
            }
            case 'M':           // set maxObjectsOnOpen operation parameter
            {
                //// TODO Use the string conversion to get this right
                maxObjectsOnOpen_opt = stringToUint32(optarg);
                break;
            }
            case 'N':           // set maxObjectsOnPull parameter
            {
                maxObjectsOnPull_opt = stringToUint32(optarg);
                break;
            }
            case 'C':           // set flag to compare pull and non pull ops
            {
                compare_opt = true;
                break;
            }
            case 't':           // set pull timeout parameter
            {
                if (strcasecmp("null", optarg) == 0)
                {
                    timeout_opt.setNullValue();
                }
                else
                {
                    timeout_opt.setValue(stringToUint32(optarg));
                }
                break;
            }
            case 'T':
            {
                timeOperation_opt = true;
                break;
            }
            case 'f':               // filterQuery parameter
            {
                filterQuery_opt = optarg;
                break;
            }
            case 'l':
            {
                filterQueryLanguage_opt = optarg;
                break;
            }
            case 'x':           // set continueOnError flag
            {
                continueOnError_opt = true;
                break;
            }
            case 'r':           // set flag to repeat operation per integer
            {
                repeat_opt = atoi(optarg);
                cout << "Option -r NOT IMPLEMENTED in code" << endl;
                break;
            }
            case 'R':           // set flag to reverse exit code
            {
                reverseExitCode_opt = true;
                break;

            }
            case 'W':           // Tread Errors as Warnings
            {
                errorsAsWarnings_opt = !errorsAsWarnings_opt;
                break;

            }
            case 's':
            {
                sleep_opt = stringToUint32(optarg);
                break;
            }
            case 'X':
            {
                if (strcasecmp("null", optarg) == 0)
                {
                    // KS_TODO - I think this should be illegal
                    // but we leave it for a test. Besides this is
                    // the default if not used.
                    maxOperationsBeforeCloseCount_opt.setNullValue();
                }
                else
                {
                    maxOperationsBeforeCloseCount_opt.setValue(atoi(optarg));
                    if (maxOperationsBeforeCloseCount_opt.getValue() == 0)
                    {
                        printf("ERROR: option %c. parameter value = 0"
                                   " not allowed: %u",
                              opt,maxOperationsBeforeCloseCount_opt.getValue());
                        exit(1);
                    }
                }
                break;
            }
            default:
            {
                printf("ERROR: unknown option: %c", opt);
                exit(1);
                break;
            }
        }
    }

    // Check usage.

    argc -= optind;
    argv += optind;

    if (argc < 1)
    {
        fprintf(stderr, "ERROR: Operation Type required\n");
        fprintf(stderr, (char*)USAGE, arg0);
        exit(1);
    }

    String operation = argv[0];

    // conditional display of input parameters
    VCOUT1 << "START " << arg0 << " " << argvParams << endl;

    CIMClient client;
    try
    {
        if (host_opt == "")
        {
            VCOUT1 << "connectLocal" << endl;
            client.connectLocal();
        }
        else
        {
            String hostName;
            Uint32 port;
            if (parseHostName(host_opt, hostName, port))
            {
                VCOUT1 << " Connect to host "
                       << hostName << ":" << port << endl;
                client.connect(hostName, port, user_opt, password_opt);
            }
            else
            {
                cerr << "Host name parameter error " << hostName << endl;
                PEGASUS_TEST_ASSERT(false);
            }
        }
    }

    catch (CIMException& e)
    { // -1 means ignore the count field)
            cerr << "CIMException Error: in connect " << e.getMessage() << endl;
            PEGASUS_TEST_ASSERT(false);
    }
    catch (Exception& e)
    {
        cerr << "Error: in connect " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

    // if this option not null, set the clientTimeout value in
    // milliseconds
    if (!clientTimeoutSeconds_opt.isNull())
    {
        client.setTimeout(clientTimeoutSeconds_opt.getValue() * 1000);
    }

    // Default class, object, and namespace for this test tool.
    // LS_TODO- These are not really logical options for defaults in the
    // long term.
    CIMNamespaceName nameSpace = "test/TestProvider";
    String ClassName = "Test_Person";
    String ObjectName = "Test_CLITestProviderClass.Id=\"Mike\"";
    if (objectName_opt != "")
    {
        ClassName = objectName_opt;
        ObjectName = objectName_opt;
    }
    if (namespace_opt != "")
    {
        nameSpace = namespace_opt;
    }

    try
    {
        if (operation == "e")
        {
            testPullEnumerateInstances(client, nameSpace, ClassName);
        }
        else if (operation == "en")
        {
            testPullEnumerationInstancePaths(client, nameSpace, ClassName);
        }
        else if (operation == "r")
        {
            testPullReferenceInstances(client,nameSpace, ObjectName);
        }
        else if (operation == "rn")
        {
            testPullReferenceInstancePaths(client,nameSpace,ObjectName );
        }
        else if (operation == "a")
        {
            testPullAssociatorInstances(client,nameSpace,ObjectName );
        }
        else if (operation == "an")
        {
            testPullAssociatorInstancePaths(client,nameSpace,ObjectName );
        }
        else if (operation == "all")
        {
            testAllClasses(client,nameSpace);
        }
        else
        {
            cout << "ERROR: Invalid operation name. " << operation << endl;
            exit(1);
        }
    }
    catch (CIMException& e)
    {
            cerr << "CIMException Error: " << e.getMessage() << endl;
            if (!reverseExitCode_opt)
            {
                PEGASUS_TEST_ASSERT(false);
            }
            exitCode = 1;
    }
    catch (Exception& e)
    {
        cerr << "Exception Error: " << e.getMessage() << endl;
        if (!reverseExitCode_opt)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        exitCode = 1;
    }


    // generate final output message with indication of any warnings, exitCodes
    // or warnings generated

    if (warnings == 0 && errors == 0 && exitCode == 0)
    {
        cout << arg0 << " +++++ passed all tests.";
        if (reverseExitCode_opt)
        {
            cout << " Return failed test exit (-R set)";
        }
        cout << endl;
    }
    else if (exitCode != 0)
    {
        if (reverseExitCode_opt)
        {
            cout << " Return +++ passed tests exit (-R set) "
                << " exitcode=" << exitCode << endl;
        }
        else
        {
            cout << arg0 <<" +++++ failed tests. exit code ="
                << exitCode << endl;
        }
        cout << endl;
    }
    else if (errors != 0)
    {
        cout << arg0 <<" +++++ tests show failed with "
             <<  errors << " error(s).";
        if (reverseExitCode_opt)
        {
            cout << " Return +++ passed tests because exit code"
                    " reversed (-R set) ";
        }
        cout << endl;
        exitCode = 2;
    }
    else
    {
        cout << arg0 <<" +++++ passed all tests with "
             <<  warnings << " warning(s).";
        if (reverseExitCode_opt)
        {
            cout << " Returning Failed tests exit (-R set) ";
        }

        cout << endl;
    }

    // Exit with either current exit code or reversed exit code depending
    // on input variable
    if (exitCode == 0)
    {
        //return (reverseExitCode_opt? 1 : 0);
        if (reverseExitCode_opt)
        {
            return 1;
        }
        else
            return 0;
    }
    else
        //return (reverseExitCode_opt? 0 : 1);
        if (reverseExitCode_opt)
        {
            return 0;
        }
        else
            return 1;
}
