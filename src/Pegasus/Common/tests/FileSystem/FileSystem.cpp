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
//              (carolann_graves@hp.com)
//              Amit K Arora, IBM (amita@in.ibm.com) for PEP#101
//              Sean Keenan, Hewlett-Packard Company (sean.keenan@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <fstream>
#include <iostream>
#include <cstdio>


#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static char * verbose;

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        tmpDir = ".";
    }
    String path;
    PEGASUS_TEST_ASSERT(FileSystem::getCurrentDirectory(path));
    // Need to add test to confirm that the directory
    // is indeed FileSystem. 
    PEGASUS_TEST_ASSERT(FileSystem::exists("FileSystem.cpp"));
    PEGASUS_TEST_ASSERT(!FileSystem::exists("NoSuchFile.dat"));
    // PEGASUS_TEST_ASSERT(!FileSystem::canExecute("FileSytem.cpp"));
    PEGASUS_TEST_ASSERT(!FileSystem::canRead("NoSuchFile.dat"));
    PEGASUS_TEST_ASSERT(!FileSystem::canWrite("NoSuchFile.dat"));
    // PEGASUS_TEST_ASSERT(!FileSystem::canRead("NoCanRead.dat"));
    PEGASUS_TEST_ASSERT(FileSystem::isDirectory(".."));
    PEGASUS_TEST_ASSERT(FileSystem::isDirectory("."));
    PEGASUS_TEST_ASSERT(!FileSystem::isDirectory("FileSystem.cpp"));
    PEGASUS_TEST_ASSERT(FileSystem::isDirectory("./testdir"));

    Array<String> paths;
    PEGASUS_TEST_ASSERT( FileSystem::getDirectoryContents("./testdir", paths) );

    for (Uint32 i = 0; i < paths.size(); i++)
    {
	if (String::equal(paths[i], "CVS"))
	{
	    paths.remove(i);
	    break;
	}
    }

    String realName;
    PEGASUS_TEST_ASSERT(FileSystem::existsNoCase("filesystem.cpp", realName));
    PEGASUS_TEST_ASSERT(String::equal(realName, "FileSystem.cpp"));

    PEGASUS_TEST_ASSERT(FileSystem::existsNoCase(
	"../FileSystem/filesystem.cpp", realName));
    PEGASUS_TEST_ASSERT(String::equal(realName, "../FileSystem/FileSystem.cpp"));

    BubbleSort(paths);
    PEGASUS_TEST_ASSERT(paths.size() == 3);
    PEGASUS_TEST_ASSERT(String::equal(paths[0], "a"));
    PEGASUS_TEST_ASSERT(String::equal(paths[1], "b"));
    PEGASUS_TEST_ASSERT(String::equal(paths[2], "c"));

    // Test for getCurrentDirectory
    // Go to testdir, test for file "a"
    // Then return and test for file
    {
	String saveDir;
	PEGASUS_TEST_ASSERT(FileSystem::getCurrentDirectory(saveDir));
	PEGASUS_TEST_ASSERT(FileSystem::changeDirectory("testdir"));
	PEGASUS_TEST_ASSERT(FileSystem::exists("a"));
	FileSystem::changeDirectory(saveDir);
	String newSaveDir;
	PEGASUS_TEST_ASSERT(FileSystem::getCurrentDirectory(newSaveDir));
	PEGASUS_TEST_ASSERT(saveDir == newSaveDir);
	PEGASUS_TEST_ASSERT(FileSystem::exists("FileSystem.cpp"));
    }
    // Test the Create and delete functions
    // Creates directories and files and deletes them.
    {
        String t (tmpDir);
        t.append("/TestDirectory");
        String t1 (tmpDir);
        t1.append("/TestDirectory2");
        String tf (tmpDir);
        tf.append("/TestFile.txt");
        CString f = tf.getCString();
        String tf1 (tmpDir);
        tf1.append("/TestFile1.txt");
        CString f1 = tf1.getCString();

	FileSystem::makeDirectory(t);
	PEGASUS_TEST_ASSERT(FileSystem::isDirectory(t));
	FileSystem::removeDirectory(t);
	PEGASUS_TEST_ASSERT(!FileSystem::isDirectory(t));

	// Tests for remove hierarchy command
	// ATTN: Removed following until next test ks
	// because remove hiearchy does not work yet.
	FileSystem::makeDirectory(t);

	String save_cwd;
	FileSystem::getCurrentDirectory(save_cwd);

	// create some files in new directory
	if (!FileSystem::changeDirectory(t))
	    // ATTN: what is valid error return?
	    return -1;

  	ofstream of1(f);
	of1 << "test" << endl;
	of1.close();
	PEGASUS_TEST_ASSERT(FileSystem::exists(tf));

  	ofstream of2(f1);
	of2 << "test" << endl;
	of2.close();
	PEGASUS_TEST_ASSERT(FileSystem::exists(tf1));

	// Create a second level directory
	FileSystem::makeDirectory(t1);

	// Create files in this dir
	if (!FileSystem::changeDirectory(t1))
	    return -1;

	ofstream of3("testfile3.txt");
	of3 << "test" << endl;
	of3.close();

    	ofstream of4("testfile4.txt");
	of4 << "test" << endl;
	of4.close();

	// Go back to top level directory

	FileSystem::changeDirectory(save_cwd);
        PEGASUS_TEST_ASSERT(FileSystem::isDirectory(t));
	FileSystem::removeDirectoryHier(t);
	// be sure directory is removed
	PEGASUS_TEST_ASSERT(!FileSystem::isDirectory(t));
        
    }
    // Test renameFile:
    {
        String FILE1 (tmpDir);
        FILE1.append("/file1.txt");
        String FILE2 (tmpDir);
        FILE2.append("/file2.txt");

  	ofstream of1(FILE1.getCString());
	of1 << "test" << endl;
	of1.close();
	PEGASUS_TEST_ASSERT(FileSystem::exists(FILE1));

	PEGASUS_TEST_ASSERT(FileSystem::exists(FILE1));
	PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE2));
	PEGASUS_TEST_ASSERT(FileSystem::renameFile(FILE1, FILE2));

	PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE1));

	PEGASUS_TEST_ASSERT(FileSystem::exists(FILE2));
	PEGASUS_TEST_ASSERT(FileSystem::renameFile(FILE2, FILE1));
	PEGASUS_TEST_ASSERT(FileSystem::exists(FILE1));
	PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE2));
        PEGASUS_TEST_ASSERT(FileSystem::removeFile(FILE1));
        PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE1));
    }
     // Test getFileNameFromPath
    {
        FileSystem::changeDirectory(path);
    	String pathName = FileSystem::getAbsoluteFileName("./testdir","a");
	PEGASUS_TEST_ASSERT(pathName.size()!=0);	// It should be there.
	pathName = FileSystem::getAbsoluteFileName("./testdir","#$@#(@$#!");
	PEGASUS_TEST_ASSERT(pathName.size()==0);	// It should not be there.	
    }
    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
