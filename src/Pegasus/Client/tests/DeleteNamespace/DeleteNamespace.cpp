//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 The Open group, BMC Software, Tivoli Systems, IBM
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
// Author: Warren Otsuka (warren_otsuka@hp.com)
//
// Modified By: 
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <cassert>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/OptionManager.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Stopwatch.h>
#include <Pegasus/Common/Exception.h>
#if !defined(PEGASUS_OS_ZOS) && ! defined(PEGASUS_OS_HPUX)
#include <slp/slp.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

String globalNamespace = "root/cimv2";

static const char __NAMESPACE_NAMESPACE [] = "root";
static const char CERTIFICATE[] = "server.pem";
static const char RANDOMFILE[]  = "ssl.rnd";
static const char CLASSNAME[]   = "__Namespace";

/** ErrorExit - Print out the error message as an
    and get out.
    @param - Text for error message
    @return - None, Terminates the program
    @execption - This function terminates the program
    ATTN: Should write to stderr
*/
void ErrorExit(const String& message)
{

    cout << message << endl;
    exit(1);
}


/* Status display of the various steps.  Shows message of function and
time to execute.  Grow this to a class so we have start and stop and time
display with success/failure for each function.
*/
static void testStart(const String& message)
{
    cout << "++++ " << message << " ++++" << endl;

}

static void testEnd(const double elapsedTime)
{
    cout << "In " << elapsedTime << " Seconds\n\n";
}

static Boolean verifyServerCertificate(CertificateInfo &certInfo)
{
    //ATTN-NB-03-05132002: Add code to handle server certificate verification.
    return true;
}

/*****************************************************************
//   Test Namespaces Hierarchy - Relative Path Name
******************************************************************/

static void TestNamespaceHierarchy1 ( CIMClient& client, 
			            Boolean activeTest, 
			            Boolean verboseTest) 
{
    Array<String> namespaces;
    String instanceName;

    namespaces.append( "test1" );
    namespaces.append( "test2" );
    namespaces.append( "test3" );
    namespaces.append( "test4" );
    namespaces.append( "test5" );
    namespaces.append( "test6" );
    namespaces.append( "test1/test2" );
    namespaces.append( "test1/test2/test3" );
    namespaces.append( "test1/test2/test3/test4" );
    namespaces.append( "test1/test2/test3/test4/test5" );
    namespaces.append( "test1/test2/test3/test4/test5/test6" );
    if(verboseTest)
    {
      cout << "++ Cleanup existing test namespaces" << endl;
    }
    for (Sint32 i = namespaces.size()-1; i > -1; i--)
    {
      // Build the instance name for __namespace
      String testNamespaceName = namespaces[i];
      instanceName.clear();
      instanceName.append( CLASSNAME );
      instanceName.append( ".Name=\"");
      instanceName.append(testNamespaceName);
      instanceName.append("\"");
      
      try
	{
	  CIMObjectPath myReference(instanceName);
	  if(verboseTest)
	    cout << "Deleting " << testNamespaceName << endl;
	  client.deleteInstance(__NAMESPACE_NAMESPACE, myReference);
	}
      catch(...)
      {
	  //Ignore errors we are just trying to cleanup
      }
    }

    if(verboseTest)
    {
      cout << "++ Create test namespaces" << endl;
    }
    for (Uint32 i = 0; i < namespaces.size(); i++)
      {
	// Build the instance name for __namespace
	String testNamespaceName = namespaces[i];
	String instanceName = CLASSNAME;
	instanceName.append( ".Name=\"");
	instanceName.append(testNamespaceName);
	instanceName.append("\"");
	if(verboseTest)
	{
	    cout << "Creating " << testNamespaceName << endl;   
	}
	try
	{
	    // Build the new instance
	    CIMInstance newInstance(instanceName);
	    newInstance.addProperty(CIMProperty("name", testNamespaceName));
	    client.createInstance(__NAMESPACE_NAMESPACE, newInstance);
	}
	catch(CIMClientException& e)
	{
	     PEGASUS_STD(cerr) << "CIMClientException NameSpace Creation: "
			<< e.getMessage() << " Creating " << instanceName
		        << PEGASUS_STD(endl);
	     exit(1);
	}
	catch(Exception& e)
	{
	    PEGASUS_STD(cerr) << "Exception NameSpace Creation: " << e.getMessage() << PEGASUS_STD(endl);
	    exit(1);
	}
      }

    for (Sint32 i = namespaces.size()-1; i > -1; i--)
    {
      // Build the instance name for __namespace
      String testNamespaceName = namespaces[i];
      instanceName.clear();
      instanceName.append( CLASSNAME );
      instanceName.append( ".Name=\"");
      instanceName.append(testNamespaceName);
      instanceName.append("\"");
      
      try
	{
	  CIMObjectPath myReference(instanceName);
	  if(verboseTest)
	    cout << "getInstance " << testNamespaceName << endl;
	  CIMInstance namespaceInstance = client.getInstance(__NAMESPACE_NAMESPACE, myReference);
	}
      catch(CIMClientException& e)
	{
	  PEGASUS_STD(cerr) << "CIMClientException NameSpace Deletion1: "
			    << e.getMessage() << " Deleting " << instanceName
			    << PEGASUS_STD(endl);
	  exit(1);
	}
      catch(Exception& e)
	{
	  PEGASUS_STD(cerr) << "Exception NameSpace Deletion2: " << e.getMessage() << PEGASUS_STD(endl);
	  exit(1);
	}
    }

  if(verboseTest)
    cout << "++ Delete test namespaces " << endl;
  
  for (Sint32 i = namespaces.size()-1; i > -1; i--)
    {
      // Build the instance name for __namespace
      String testNamespaceName = namespaces[i];
      instanceName.clear();
      instanceName.append( CLASSNAME );
      instanceName.append( ".Name=\"");
      instanceName.append(testNamespaceName);
      instanceName.append("\"");
      
      try
	{
	  CIMObjectPath myReference(instanceName);
	  if(verboseTest)
	    cout << "Deleting " << testNamespaceName << endl;
	  client.deleteInstance(__NAMESPACE_NAMESPACE, myReference);
	}
      catch(CIMClientException& e)
	{
	  PEGASUS_STD(cerr) << "CIMClientException NameSpace Deletion1: "
			    << e.getMessage() << " Deleting " << instanceName
			    << PEGASUS_STD(endl);
	  exit(1);
	}
      catch(Exception& e)
	{
	  PEGASUS_STD(cerr) << "Exception NameSpace Deletion2: " << e.getMessage() << PEGASUS_STD(endl);
	  exit(1);
	}
    }
}

/*****************************************************************
//   Test Namespaces Hierarchy - Full Path Name
******************************************************************/

static void TestNamespaceHierarchy2 ( CIMClient& client, 
			              Boolean activeTest, 
			              Boolean verboseTest) 
{
    Array<String> namespaces;
    String instanceName;

    namespaces.append( "test1" );
    namespaces.append( "test2" );
    namespaces.append( "test3" );
    namespaces.append( "test4" );
    namespaces.append( "test5" );
    namespaces.append( "test6" );
    namespaces.append( "test1/test2" );
    namespaces.append( "test1/test2/test3" );
    namespaces.append( "test1/test2/test3/test4" );
    namespaces.append( "test1/test2/test3/test4/test5" );
    namespaces.append( "test1/test2/test3/test4/test5/test6" );
    if(verboseTest)
    {
      cout << "++ Cleanup existing test namespaces" << endl;
    }
    for (Sint32 i = namespaces.size()-1; i > -1; i--)
    {
      // Build the instance name for __namespace
      instanceName.clear();
      instanceName.append( CLASSNAME );
      instanceName.append( ".Name=\"\"");
      
      try
	{
	  CIMObjectPath myReference(instanceName);
	  if(verboseTest)
	    cout << "Deleting " << namespaces[i] << endl;
	  client.deleteInstance(namespaces[i], myReference);
	}
      catch(...)
      {
	  //Ignore errors we are just trying to cleanup
      }
    }

    if(verboseTest)
    {
      cout << "++ Create test namespaces" << endl;
    }
    for (Uint32 i = 0; i < namespaces.size(); i++)
      {
	try
	{
	    // Build the new instance
	    CIMInstance newInstance(instanceName);
	    newInstance.addProperty(CIMProperty("name",String::EMPTY));
            if(verboseTest)
            {
              cout << "Creating " << namespaces[i] << endl;
            }
	    client.createInstance(namespaces[i], newInstance);
	}
	catch(CIMClientException& e)
	{
	     PEGASUS_STD(cerr) << "CIMClientException NameSpace Creation: "
			<< e.getMessage() << " Creating " << namespaces[i]
		        << PEGASUS_STD(endl);
	     exit(1);
	}
	catch(Exception& e)
	{
	    PEGASUS_STD(cerr) << "Exception NameSpace Creation: " << e.getMessage() << PEGASUS_STD(endl);
	    exit(1);
	}
      }

  if(verboseTest)
    cout << "++ Delete test namespaces" << endl;
  
  for (Sint32 i = namespaces.size()-1; i > -1; i--)
    {
      // Build the instance name for __namespace
      String testNamespaceName = namespaces[i];
      instanceName.clear();
      instanceName.append( CLASSNAME );
      instanceName.append( ".Name=\"\"");
      
      try
	{
	  CIMObjectPath myReference(instanceName);
	  if(verboseTest)
	    cout << "Deleting " << testNamespaceName << endl;
	  client.deleteInstance(namespaces[i], myReference);
	}
      catch(CIMClientException& e)
	{
	  PEGASUS_STD(cerr) << "CIMClientException NameSpace Deletion1: "
			    << e.getMessage() << " Deleting " << instanceName
			    << PEGASUS_STD(endl);
	  exit(1);
	}
      catch(Exception& e)
	{
	  PEGASUS_STD(cerr) << "Exception NameSpace Deletion2: " << e.getMessage() << PEGASUS_STD(endl);
	  exit(1);
	}
    }
 }

///////////////////////////////////////////////////////////////
//    OPTION MANAGEMENT
///////////////////////////////////////////////////////////////

/** GetOptions function - This function defines the Options Table
    and sets up the options from that table using the option manager.
    const char* optionName;
    const char* defaultValue;
    int required;
    Option::Type type;
    char** domain;
    Uint32 domainSize;
    const char* commandLineOptionName;
    const char* optionHelpMessage;
    
*/
void GetOptions(
    OptionManager& om,
    int& argc,
    char** argv,
    const String& pegasusHome)
{
    static struct OptionRow optionsTable[] =
        //     optionname defaultvalue rqd  type domain domainsize clname hlpmsg
    {
		 {"active", "false", false, Option::BOOLEAN, 0, 0, "a",
		 		      "If set allows test that modify the repository" },
		 
		 {"repeat", "1", false, Option::WHOLE_NUMBER, 0, 0, "r",
		 		       "Specifies a Repeat Count Entire test repeated this many times" },
		 
		 {"namespace", "root/cimv2", false, Option::STRING, 0, 0, "-n",
		 		 		 "specifies namespace to use for test" },

		 {"version", "false", false, Option::BOOLEAN, 0, 0, "v",
		 		 		 "Displays TestClient Version "},

		 {"verbose", "false", false, Option::BOOLEAN, 0, 0, "verbose",
		 		 		 "If set, outputs extra information "},

		 {"help", "false", false, Option::BOOLEAN, 0, 0, "h",
		 		     "Prints help message with command line options "},
		 {"debug", "false", false, Option::BOOLEAN, 0, 0, "d", 
		              "Not Used "},
		 {"slp", "false", false, Option::BOOLEAN, 0, 0, "slp", 
		 		 		 "use SLP to find cim servers to test"},
		 {"ssl", "false", false, Option::BOOLEAN, 0, 0, "ssl", 
		 		 		 "use SSL"}, 

		 {"local", "false", false, Option::BOOLEAN, 0, 0, "local",
		 		 		 "Use local connection mechanism"},
		 {"user", "", false, Option::STRING, 0, 0, "user",
		 		 		 "Specifies user name" },

		 {"password", "", false, Option::STRING, 0, 0, "password",
		 		 		 "Specifies password" }

    };
    const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

    om.registerOptions(optionsTable, NUM_OPTIONS);

    String configFile = pegasusHome + "/cimserver.conf";

    cout << "Config file from " << configFile << endl;

    if (FileSystem::exists(configFile))
		 om.mergeFile(configFile);

    om.mergeCommandLine(argc, argv);

    om.checkRequiredOptions();
}


///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{

  // char connection[50] = "localhost:5988";
  char *address_string = NULL;
  
    Uint32 repetitions = 1;

    // Get environment variables:

    String pegasusHome;
    pegasusHome = "/";
    // GetEnvironmentVariables(argv[0], pegasusHome);

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    OptionManager om;

    try
    {
		 GetOptions(om, argc, argv, pegasusHome);
    }
    catch (CIMClientException& e)
    {
		 cerr << argv[0] << ": " << e.getMessage() << endl;
		 exit(1);
    }
    catch (Exception& e)
    {
		 cerr << argv[0] << ": " << e.getMessage() << endl;
		 exit(1);
    }

    // Check to see if user asked for help (-h otpion):
    if (om.valueEquals("help", "true"))
    {
                String header = "Usage ";
                header.append(argv[0]);
                header.append(" -parameters host [host]");

                String trailer = "Assumes localhost:5988 if host not specified";
                trailer.append("\nHost may be of the form name or name:port");
                trailer.append("\nPort 5988 assumed if port number missing.");
                om.printOptionsHelpTxt(header, trailer);

		 exit(0);
    }

    String localNameSpace;
    om.lookupValue("namespace", localNameSpace);
    globalNamespace = localNameSpace;
    cout << "Namespace = " << localNameSpace << endl;

    String userName;
    om.lookupValue("user", userName);
    if (userName != String::EMPTY)
    {
       cout << "Username = " << userName << endl;
    }
    Boolean verboseTest = om.isTrue("verbose");

    String password;
    om.lookupValue("password", password);
    if (password != String::EMPTY)
    {
       cout << "password = " << password << endl;
    }

	// Set up number of test repetitions.  Will repeat entire test this number of times
	// Default is zero
	String repeats;
	Uint32 repeatTestCount = 0;
	/* ATTN: KS P0 Test and fix function added to Option Manager
	*/
	if (!om.lookupIntegerValue("repeat", repeatTestCount))
	    repeatTestCount = 1;
	/*
	if (om.lookupValue("repeat", repeats))
        {
		char* repeatsStr = repeats.allocateCString();
		repeatTestCount = atol(repeatsStr);
		delete [] repeatsStr;
        }
	else
		repeatTestCount = 1;
	*/
	if(verboseTest)
		cout << "Test repeat count " << repeatTestCount << endl;

	// Setup the active test flag.  Determines if we change repository.
    Boolean activeTest = false;
    if (om.valueEquals("active", "true"))
		 activeTest = true;
     
    // here we determine the list of systems to test.
    // All arguments remaining in argv go into list.
    // if SLP option set, SLP list goes into set.
    // if SLP false and no args, use default localhost:5988
    Boolean useSLP =  om.isTrue("slp");
    cout << "SLP " << (useSLP ? "true" : "false") << endl;

    Boolean localConnection = (om.valueEquals("local", "true"))? true: false;
    cout << "localConnection " << (localConnection ? "true" : "false") << endl;

    Array<String> connectionList;
    if (argc > 1 && !localConnection)
		 for (Sint32 i = 1; i < argc; i++)
		     connectionList.append(argv[i]);

    // substitute the default only if noslp and no params
    if(useSLP == false && argc < 2)
      connectionList.append("localhost:5988");

    // Expand host to add port if not defined

#if !defined(PEGASUS_OS_ZOS) && ! defined(PEGASUS_OS_HPUX)
    if( useSLP )
    {
      slp_client discovery = slp_client();
      discovery.discovery_cycle ( "service:cim.pegasus",
		 		  NULL,
		 		  "DEFAULT" ) ;
      
      struct rply_list *replies = discovery.get_response( );
      String host ;
      while( replies != NULL )
		 {
		   slp_get_addr_string_from_url(replies->url, host) ;
		   connectionList.append( host ) ;
		   delete replies;
		   replies = discovery.get_response( ) ;
		 }
    }
#endif
    Boolean useSSL =  om.isTrue("ssl");

	// Show the connectionlist
    cout << "Connection List size " << connectionList.size() << endl;
    for (Uint32 i = 0; i < connectionList.size(); i++)
	cout << "Connection " << i << " address " << connectionList[i] << endl; 
    
    for(Uint32 numTests = 1; numTests <= repeatTestCount; numTests++)
	{
		cout << "Test Repetition # " << numTests << endl;
		for (Uint32 i = 0; i < connectionList.size(); i++)
		{
			cout << "Start Try Block" << endl;
		  try
		  {
		     cout << "Set Stopwatch" << endl;
		     Stopwatch elapsedTime;
		     cout << "Create client" << endl;
		     CIMClient client(60 * 1000);
		     cout << "Client created" << endl;
                     if (useSSL)
		     {

                        //
                        // Get environment variables:
                        //
                        const char* pegasusHome = getenv("PEGASUS_HOME");

                        String certpath = String::EMPTY;
                        if (pegasusHome)
                        {
                               certpath.append(pegasusHome);
                               certpath.append("/");
                        }
                        certpath.append(CERTIFICATE);

#ifdef PEGASUS_SSL_RANDOMFILE
                        String randFile = String::EMPTY;
                        if (pegasusHome)
                        {
                              randFile.append(pegasusHome);
                              randFile.append("/");
                        }
                        randFile.append(RANDOMFILE);

                        SSLContext * sslcontext = new SSLContext(certpath,verifyServerCertificate, randFile, true);
#else
                        SSLContext * sslcontext = new SSLContext(certpath);
#endif
			if (om.isTrue("local"))
			{
			     cout << "Using local SSL connection mechanism " << endl;
     			     client.connectLocal();
			}
			else 
			{
                            cout << "connecting to " << connectionList[i] << " using SSL" << endl;
		            client.connect(connectionList[i], sslcontext, userName, password);
                        }
		      }
	              else
		      {
			if (om.isTrue("local"))
			{
			     cout << "Using local connection mechanism " << endl;
     			     client.connectLocal();
			}
			else 
			{

			  cout << "Connecting to " << connectionList[i] << endl;
						   client.connect(connectionList[i], userName, password);
			}
		      }
		      cout << "Client Connected" << endl;

		      testStart("Test NameSpace Operations - Relative Name");
                      elapsedTime.reset();		
    		      TestNamespaceHierarchy1(client, activeTest, verboseTest);
	              testEnd(elapsedTime.getElapsed());
		      testStart("Test NameSpace Operations - Absolute Name");
                      elapsedTime.reset();		
		      TestNamespaceHierarchy2(client, activeTest, verboseTest);
		      testEnd(elapsedTime.getElapsed());
		      client.disconnect();
		  }
		  catch(CIMClientException& e)
		  {
			   PEGASUS_STD(cerr) << "Error: " << e.getMessage() <<
			     PEGASUS_STD(endl);
			   exit(1);
		  }
		}
	}
    PEGASUS_STD(cout) << "+++++ "<< argv[0] << " Terminated Normally" << PEGASUS_STD(endl);
    return 0;
}

