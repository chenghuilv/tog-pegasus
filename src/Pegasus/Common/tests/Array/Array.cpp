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
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#define NEED_STRING_EQUAL

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/String.h>
#include "Str.h"
#include "Int.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

template<class T>
void Print(const Array<T>& arr)
{
    for (Uint32 i = 0; i < arr.size(); i++)
        cout << arr[i] << endl;
    cout << "-- end" << endl;
}

template<class STR>
void test01(STR*)
{
    Array<STR> arr(3, STR("Hello"));
    PEGASUS_TEST_ASSERT(arr.size() == 3);
    PEGASUS_TEST_ASSERT(arr[0] == STR("Hello"));
    PEGASUS_TEST_ASSERT(arr[1] == STR("Hello"));
    PEGASUS_TEST_ASSERT(arr[2] == STR("Hello"));
    if(getenv("PEGASUS_TEST_VERBOSE"))
        Print(arr);
}

template<class STR>
void test02(STR*)
{
    Array<STR> arr;
    PEGASUS_TEST_ASSERT(arr.size() == 0);

    arr.append("three");
    arr.append("four");
    arr.prepend("one");
    arr.prepend("zero");
    arr.insert(2, "two");

    PEGASUS_TEST_ASSERT(arr.size() == 5);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "two");
    PEGASUS_TEST_ASSERT(arr[3] == "three");
    PEGASUS_TEST_ASSERT(arr[4] == "four");

    arr.remove(2);
    PEGASUS_TEST_ASSERT(arr.size() == 4);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "three");
    PEGASUS_TEST_ASSERT(arr[3] == "four");
}

template<class T>
void test03(const T*)
{
    Array<T> arr;

    Uint32 tmp1[] = { 1, 2, 3 };

    arr.insert(0, 2);
    arr.insert(0, 1);
    arr.insert(0, 0);
    arr.append(3);
    arr.insert(4, 4);

    Array<T> arr2 = arr;

    Array<T> arr3;
    arr3 = arr2;
    arr = arr3;

    PEGASUS_TEST_ASSERT(arr.size() == 5);
    PEGASUS_TEST_ASSERT(arr[0] == 0);
    PEGASUS_TEST_ASSERT(arr[1] == 1);
    PEGASUS_TEST_ASSERT(arr[2] == 2);
    PEGASUS_TEST_ASSERT(arr[3] == 3);
    PEGASUS_TEST_ASSERT(arr[4] == 4);

    arr.remove(4);
    PEGASUS_TEST_ASSERT(arr.size() == 4);
    PEGASUS_TEST_ASSERT(arr[0] == 0);
    PEGASUS_TEST_ASSERT(arr[1] == 1);
    PEGASUS_TEST_ASSERT(arr[2] == 2);
    PEGASUS_TEST_ASSERT(arr[3] == 3);

    arr.remove(0);
    PEGASUS_TEST_ASSERT(arr.size() == 3);
    PEGASUS_TEST_ASSERT(arr[0] == 1);
    PEGASUS_TEST_ASSERT(arr[1] == 2);
    PEGASUS_TEST_ASSERT(arr[2] == 3);

    arr.remove(0);
    arr.remove(1);
    PEGASUS_TEST_ASSERT(arr.size() == 1);
    PEGASUS_TEST_ASSERT(arr[0] == 2);

    arr.remove(0);
    PEGASUS_TEST_ASSERT(arr.size() == 0);
}

void test04()
{
    Array<String> arr(3, "Hello");
    PEGASUS_TEST_ASSERT(arr.size() == 3);
    PEGASUS_TEST_ASSERT(arr[0] == "Hello");
    PEGASUS_TEST_ASSERT(arr[1] == "Hello");
    PEGASUS_TEST_ASSERT(arr[2] == "Hello");
}

void test05()
{
    Array<String> arr;
    PEGASUS_TEST_ASSERT(arr.size() == 0);

    arr.append("three");
    arr.append("four");
    arr.prepend("one");
    arr.prepend("zero");
    arr.insert(2, "two");

    PEGASUS_TEST_ASSERT(arr.size() == 5);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "two");
    PEGASUS_TEST_ASSERT(arr[3] == "three");
    PEGASUS_TEST_ASSERT(arr[4] == "four");

    arr.remove(2);
    PEGASUS_TEST_ASSERT(arr.size() == 4);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "three");
    PEGASUS_TEST_ASSERT(arr[3] == "four");
}

void test06()
{
    Boolean exceptionCaught;

    // Test constructor memory overflow
    exceptionCaught = false;
    try
    {
        Array<Uint32> arr(0xffff0000);
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test constructor memory overflow
    exceptionCaught = false;
    try
    {
        Array<Uint32> arr(0xffff0000, 100);
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test constructor memory overflow
    exceptionCaught = false;
    try
    {
        Uint32 myInt = 50;
        Array<Uint32> arr(&myInt, 0xffff0000);
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test reserveCapacity memory overflow
    {
        Array<Uint32> arr(128);
        PEGASUS_TEST_ASSERT(arr.getCapacity() == 128);
        exceptionCaught = false;
        try
        {
            arr.reserveCapacity(0xffff0000);
        }
        catch (const PEGASUS_STD(bad_alloc)&)
        {
            exceptionCaught = true;
        }
        PEGASUS_TEST_ASSERT(exceptionCaught);
        PEGASUS_TEST_ASSERT(arr.getCapacity() == 128);
    }
}

int main(int argc, char** argv)
{
    try
    {
        test01((Str*)0);
        test02((Str*)0);
        test01((String*)0);
        test02((String*)0);
        test03((Int*)0);
        test03((int*)0);
        test04();
        test05();
        test06();
        PEGASUS_TEST_ASSERT(Int::_count == 0);
        PEGASUS_TEST_ASSERT(Str::_constructions == Str::_destructions);
    }
    catch(const Exception& e)
    {
        cerr << argv[0] << " Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
