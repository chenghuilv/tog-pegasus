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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/ObjectNormalizer.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static char * verbose = 0;

void Test01(void)
{
    if(verbose)
    {
		cout << "Test01 " << endl;
    }
	
    String repositoryRoot = String(getenv("PEGASUS_HOME")) + String("/repository");

    CIMRepository * repository = new CIMRepository(repositoryRoot);

    ObjectNormalizer normalizer(*repository);

    {
        CIMClass cimClass("CIM_ManagedElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                false,
                false,
                false,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

    {
        CIMClass cimClass("CIM_ManagedElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                false,
                false,
                true,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

    {
        CIMClass cimClass("CIM_ManagedElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                false,
                true,
                true,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

}

void Test02(void)
{
    if(verbose)
    {
		cout << "Test02 " << endl;
    }
	
    String repositoryRoot = String(getenv("PEGASUS_HOME")) + String("/repository");

    CIMRepository * repository = new CIMRepository(repositoryRoot);

    ObjectNormalizer normalizer(*repository);

    {
        CIMClass cimClass("CIM_SoftwareElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                false,
                false,
                false,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

    {
        CIMClass cimClass("CIM_SoftwareElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                false,
                false,
                true,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

    {
        CIMClass cimClass("CIM_SoftwareElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                false,
                true,
                true,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

    {
        CIMClass cimClass("CIM_SoftwareElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        cimClass.addProperty(CIMProperty("Caption", CIMValue(String("Default Value"))));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                true,
                false,
                false,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }

    {
        CIMClass cimClass("CIM_SoftwareElement");

        cimClass.setPath(CIMObjectPath("", "root/cimv2", cimClass.getClassName()));

        CIMClass newClass =
            normalizer.normalizeClass(
                cimClass,
                true,
                true,
                true,
                CIMPropertyList());

        XmlWriter::printClassElement(newClass);
    }
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

	try
    {
		Test01();
		Test02();
    }
    catch(Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;

        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
