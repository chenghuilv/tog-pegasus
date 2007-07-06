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
/*
    Test basic CIMRepository class functions including:
    Create namespace
    delete namespace
    failure of delete of non-existant namespaced
    delete non-empty namespace (should fail)
    putting classes into namespace

*/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;

String repositoryRoot;

void test01(Uint32 mode)
{
  CIMRepository r (repositoryRoot, mode);

    const String NAMESPACE = "aa/bb";

    try
    {
    r.createNameSpace(NAMESPACE);
    }
    catch (AlreadyExistsException&)
    {
    // Ignore this!
    }

    CIMClass c(CIMName ("MyClass"));

    r.setQualifier(
    NAMESPACE, CIMQualifierDecl(CIMName ("key"), true, CIMScope::PROPERTY));

    c.addProperty(
    CIMProperty(CIMName ("key"), Uint32(0))
        .addQualifier(CIMQualifier(CIMName ("key"), true)))
    .addProperty(CIMProperty(CIMName ("ratio"), Real32(1.5)))
    .addProperty(CIMProperty(CIMName ("message"), String("Hello World")));

    r.createClass(NAMESPACE, c);

    CIMConstClass cc;
    cc = r.getClass(CIMNamespaceName ("aa/bb"), CIMName ("MyClass"),
        false,true, true);

    PEGASUS_TEST_ASSERT(c.identical(cc));
    PEGASUS_TEST_ASSERT(cc.identical(c));

}

void test02(Uint32 mode)
{
    //--------------------------------------------------------------------------
    // Create repository:
    //--------------------------------------------------------------------------

  CIMRepository r (repositoryRoot, mode);

    const String NAMESPACE = "aa/bb";
    const CIMName SUPERCLASS = "SuperClass";
    const CIMName SUBCLASS = "SubClass";

    try
    {
    r.createNameSpace(NAMESPACE);
    }
    catch (CIMException& e)
    {
    PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_ALREADY_EXISTS);
    // Ignore this!
    }

    //--------------------------------------------------------------------------
    // Create Class (SuperClass):
    //--------------------------------------------------------------------------

    CIMClass superClass(SUPERCLASS);

    superClass
    .addProperty(CIMProperty(CIMName ("Last"), String())
        .addQualifier(CIMQualifier(CIMName ("key"), true)))
    .addProperty(CIMProperty(CIMName ("First"), String())
        .addQualifier(CIMQualifier(CIMName ("key"), true)))
    .addProperty(CIMProperty(CIMName ("Age"), Uint8(0))
        .addQualifier(CIMQualifier(CIMName ("key"), true)));

    r.createClass(NAMESPACE, superClass);

    //--------------------------------------------------------------------------
    // Create Class (SubClass):
    //--------------------------------------------------------------------------

    CIMClass subClass(SUBCLASS, SUPERCLASS);
    subClass.addProperty(CIMProperty(CIMName ("Role"), String()));
    r.createClass(NAMESPACE, subClass);

    //--------------------------------------------------------------------------
    // Create Instance (of SubClass):
    //--------------------------------------------------------------------------

    CIMInstance subClassInstance(SUBCLASS);
    subClassInstance.addProperty(CIMProperty(CIMName ("Last"),
        String("Smith")));
    subClassInstance.addProperty(CIMProperty(CIMName ("First"),
        String("John")));
    subClassInstance.addProperty(CIMProperty(CIMName ("Age"), Uint8(101)));
    subClassInstance.addProperty(CIMProperty(CIMName ("Role"),
        String("Taylor")));
    r.createInstance(NAMESPACE, subClassInstance);

    //--------------------------------------------------------------------------
    // Get instance back:
    //--------------------------------------------------------------------------

    CIMObjectPath instanceName1 = subClassInstance.buildPath(subClass);

    CIMInstance tmp = r.getInstance(NAMESPACE, instanceName1,false,true,true);

    PEGASUS_TEST_ASSERT(subClassInstance.identical(tmp));

    //--------------------------------------------------------------------------
    // Miscellaneous tests
    //--------------------------------------------------------------------------

    try
    {
    r.execQuery("WQL", "myquery");
    }
    catch (CIMException& e)
    {
        // execQuery operation is not supported yet
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_SUPPORTED);
    }

    // Test to assure that delete of non-existant namespace
    // causes exception.
    Boolean testFailed=false;
    try
    {
        // delete a non-empty namespace
        r.deleteNameSpace(NAMESPACE);
    }
    catch (NonEmptyNameSpace&)
    {
        testFailed=true;
    }
    PEGASUS_TEST_ASSERT(testFailed);

//    ATTN:2.0:ENHANCE:DEFERRED:getProviderName() is not supported.
//    String providerName = r.getProviderName();
//    PEGASUS_TEST_ASSERT (providerName == "repository");

    Array<CIMName> subClassNames;
    r.getSubClassNames(NAMESPACE, SUPERCLASS, true, subClassNames);
    PEGASUS_TEST_ASSERT(subClassNames.size() == 1);
    PEGASUS_TEST_ASSERT(subClassNames[0] == SUBCLASS);

    Array<CIMName> superClassNames;
    r.getSuperClassNames(NAMESPACE, SUBCLASS, superClassNames);
    PEGASUS_TEST_ASSERT(superClassNames.size() == 1);
    PEGASUS_TEST_ASSERT(superClassNames[0] == SUPERCLASS);
}

void test03(Uint32 mode)
{
    const char* home = getenv("PEGASUS_HOME");

    if (!home)
    {
    cerr << "PEGASUS_HOME environment variable not set" << endl;
    exit(1);
    }

    String repositoryRoot = home;
    repositoryRoot.append("/repository");
    CIMRepository r(repositoryRoot, mode);

    Array<CIMObjectPath> names = r.associatorNames(
    CIMNamespaceName ("root/cimv2"),
    CIMObjectPath ("X.key=\"John Smith\""));

    if (verbose)
    {
        for (Uint32 i = 0; i < names.size(); i++)
        {
        cout << "names[i]=[" << names[i].toString() << "]" << endl;
        }
    }
}


int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

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

      test01(mode);
      test02(mode);

      // bug 4206 - test03 removed because it usees the repository
      // in PEGASUS_HOME which should not be done in unit tests.
      // Additionally: There is are unit test under the Pegasus/Compiler
      // CompAssoc.cpp and InstAlias.cpp that test associations.

      // test03(mode);
    }
    catch (Exception& e)
    {
    cout << argv[0] << " " << argv[1] << " " << e.getMessage() << endl;
    exit(1);
    }

    cout << argv[0] << " " << argv[1] << " +++++ passed all tests" << endl;

    return 0;
}
