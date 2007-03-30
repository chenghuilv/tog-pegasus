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
//=============================================================================
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <Pegasus/Common/Dir.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    Array<String> names;

    for (Dir dir("testdir"); dir.more(); dir.next())
    {
    String name = dir.getName();

    if (String::equal(name, ".") || String::equal(name, "..") 
        || String::equal(name, "CVS"))
    {
        continue;
    }

    names.append(name);
    }

    BubbleSort(names);
    PEGASUS_TEST_ASSERT(names.size() == 3);
    PEGASUS_TEST_ASSERT(String::equal(names[0], "a"));
    PEGASUS_TEST_ASSERT(String::equal(names[1], "b"));
    PEGASUS_TEST_ASSERT(String::equal(names[2], "c"));
}

int main(int argc, char** argv)
{
    try
    {
    test01();
    }
    catch (Exception& e)
    {
    cout << e.getMessage() << endl;
    exit(1);
    }

    try 
    {
    Dir dir("noSuchDirectory");
    }
    catch (CannotOpenDirectory&)
    {
    cout << argv[0] << " +++++ passed all tests" << endl;
    exit(0);
    }

    PEGASUS_TEST_ASSERT(0);
    exit(1);

    return 0;
}
