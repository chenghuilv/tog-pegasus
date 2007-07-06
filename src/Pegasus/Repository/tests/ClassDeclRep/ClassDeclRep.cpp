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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;

void Test01(Uint32 mode)
{
    String repositoryRoot;
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }

    repositoryRoot.append("/repository");

    CIMRepository r (repositoryRoot, mode);

    // Create a namespace:

    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("zzz");
    r.createNameSpace(NAMESPACE);

    // Create a qualifier (and read it back):

    CIMQualifierDecl q1(CIMName ("abstract"), false, CIMScope::CLASS);
    r.setQualifier(NAMESPACE, q1);

    CIMConstQualifierDecl q2 = r.getQualifier(NAMESPACE, CIMName ("abstract"));
    PEGASUS_TEST_ASSERT(q1.identical(q2));

    // Create two simple classes:

    CIMClass class1(CIMName ("Class1"));
    class1.addQualifier(CIMQualifier(CIMName ("abstract"), true));
    CIMClass class2(CIMName ("Class2"), CIMName ("Class1"));

    r.createClass(NAMESPACE, class1);
    r.createClass(NAMESPACE, class2);

    // Enumerate the class names:
    Array<CIMName> classNames =
    r.enumerateClassNames(NAMESPACE, CIMName (), true);

    BubbleSort(classNames);

    PEGASUS_TEST_ASSERT(classNames.size() == 2);
    // ATTN-RK-20020729: Remove CIMName cast when Repository uses CIMName
    PEGASUS_TEST_ASSERT(CIMName(classNames[0]).equal(CIMName ("Class1")));
    PEGASUS_TEST_ASSERT(CIMName(classNames[1]).equal(CIMName ("Class2")));

    // Get the classes and determine if they are identical with input

    CIMClass c1 =  r.getClass(NAMESPACE, CIMName ("Class1"), true, true, true);
    CIMClass c2 =  r.getClass(NAMESPACE, CIMName ("Class2"), true, true, true);

    PEGASUS_TEST_ASSERT(c1.identical(class1));
    PEGASUS_TEST_ASSERT(c1.identical(class1));

    Array<CIMClass> classes =
    r.enumerateClasses(NAMESPACE, CIMName (), true, true, true);

    // Attempt to delete Class1. It should fail since the class has
    // children.

    try
    {
    r.deleteClass(NAMESPACE, CIMName ("Class1"));
    }
    catch (CIMException& e)
    {
    PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_CLASS_HAS_CHILDREN);
    }

    // Delete all classes created here:

    r.deleteClass(NAMESPACE, CIMName ("Class2"));
    r.deleteClass(NAMESPACE, CIMName ("Class1"));

    // Be sure the class files are really gone:

    Array<String> tmp;
    String classesDir (repositoryRoot);
    classesDir.append("/zzz/classes");
    PEGASUS_TEST_ASSERT(FileSystem::getDirectoryContents (classesDir, tmp));
    PEGASUS_TEST_ASSERT(tmp.size() == 0);
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;
    try
    {
      Uint32 mode;
      if (!strcmp(argv[1],"XML") )
    {
      mode = CIMRepository::MODE_XML;
      if (verbose) cout << argv[0]<< ": using XML mode repository" << endl;
    }
      else if (!strcmp(argv[1],"BIN") )
    {
      mode = CIMRepository::MODE_BIN;
      if (verbose) cout << argv[0]<< ": using BIN mode repository" << endl;
    }
      else
    {
      cout << argv[0] << ": invalid argument: " << argv[1] << endl;
      return 0;
    }

    Test01(mode);
    }
    catch (Exception& e)
    {
    cout << e.getMessage() << endl;
    exit(1);
    }

    cout << argv[0] << " " << argv[1] << " +++++ passed all tests" << endl;

    return 0;
}
