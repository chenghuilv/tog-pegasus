//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
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
// Modified By: Mike Day (mdday@us.ibm.com) 
//
// Modified By:	Karl Schopmeyer (k.schopmeyer@opengroup.org)
//
// Modified By: Nag Boranna (nagaraja_boranna@hp.com)
//
// Modified By: Jenny Yu (jenny_yu@hp.com)
//
// Modified By: Sushma Fernandes (sushma_fernandes@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Notes on deamon operation (Unix) and service operation (Win 32):
//
// To run pegasus as a daemon on Unix platforms: 
//
// cimserver
//
// To NOT run pegasus as a daemon on Unix platforms, set the daemon config
// property to false:
//
// cimserver daemon=false
//
// The daemon config property has no effect on windows operation. 
//
// To shutdown pegasus, use the -s option:
// 
// cimserver -s 
//
// To run pegasus as an NT service, there are FOUR  different possibilities:
//
// To INSTALL the Pegasus service, 
//
// cimserver -install
//
// To REMOVE the Pegasus service, 
//
// cimserver -remove
//
// To START the Pegasus service, 
//
// net start cimserver
//
// To STOP the Pegasus service, 
//
// net stop cimserver
//
// Alternatively, you can use the windows service manager. Pegasus shows up 
// in the service database as "Pegasus CIM Object Manager"
//
// Mike Day, mdday@us.ibm.com
// 
//////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/suballoc/suballoc.h>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Server/CIMServer.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Server/ShutdownService.h>
#include <Pegasus/Common/Destroyer.h>
#if !defined(PEGASUS_OS_ZOS) && ! defined(PEGASUS_OS_HPUX)
#include <slp/slp.h>
#endif


#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include "cimserver_windows.cpp"
#elif defined(PEGASUS_OS_TYPE_UNIX)
# include "cimserver_unix.cpp"
#else
# error "Unsupported platform"
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//
//  The command name.
//
static const char COMMAND_NAME []    = "cimserver";

//
//  The constant defining usage string.
//
static const char USAGE []           = "Usage: ";

/**
Constants representing the command line options.
*/
static const char OPTION_VERSION     = 'v';

static const char OPTION_HELP        = 'h';

static const char OPTION_HOME        = 'D';

static const char OPTION_SHUTDOWN    = 's';

#if defined(PEGASUS_OS_HPUX)
static const char OPTION_BINDVERBOSE = 'X';
#endif

static const String PROPERTY_TIMEOUT = "shutdownTimeout";
static const String CIMSERVERSTART_FILE = "/etc/wbem/cimserver_start.conf";

ConfigManager*    configManager;

/** GetOptions function - This function defines the Options Table
    and sets up the options from that table using the config manager.
*/
void GetOptions(
    ConfigManager* cm,
    int& argc,
    char** argv,
    const String& pegasusHome)
{
    try
    {
        cm->mergeConfigFiles();

        cm->mergeCommandLine(argc, argv);
    }
    catch (NoSuchFile nsf)
    {
        throw nsf;
    }
    catch (FileNotReadable fnr)
    {
        throw fnr;
    }
    catch (CannotRenameFile ftrf)
    {
        throw ftrf;
    }
    catch (ConfigFileSyntaxError cfse)
    {
        throw cfse;
    }
    catch(UnrecognizedConfigProperty ucp)
    {
        throw ucp;
    }
    catch(InvalidPropertyValue ipv)
    {
        throw ipv;
    }
}

/* PrintHelp - This is temporary until we expand the options manager to allow
   options help to be defined with the OptionRow entries and presented from
   those entries.
*/
void PrintHelp(const char* arg0)
{
    /**
        Build the usage string for the config command.
    */
    String usage = String (USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ [ options ] | [ configProperty=value, ... ] ]\n");
    usage.append ("  options\n");
    usage.append ("    -v          - displays CIM Server version number\n");
    usage.append ("    -h          - prints this help message\n");
    usage.append ("    -s          - shuts down CIM Server\n");
#ifndef PEGASUS_OS_HPUX
    usage.append ("    -D [home]   - sets pegasus home directory\n");
#endif
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    usage.append ("    -install    - installs pegasus as a Windows NT Service\n");
    usage.append ("    -remove     - removes pegasus as a Windows NT Service\n");
#endif
    usage.append ("  configProperty=value\n");
    usage.append ("                - sets CIM Server configuration property\n");

    cout << endl;
    cout << PEGASUS_NAME << PEGASUS_VERSION << endl;
    cout << endl;
    cout << usage << endl;
}

void shutdownCIMOM(Uint32 timeoutValue)
{
    //
    // Create CIMClient object
    //
    CIMClient client;

    //
    // Get local host name
    //
    String hostStr = System::getHostName();

    //
    // open connection to CIMOM 
    //
    try
    {
        client.connectLocal();

        //
        // set client timeout to 2 seconds more than the shutdown timeout
        // so that the command client does not timeout before the cimserver 
        // terminates
        //
        client.setTimeOut( (timeoutValue+2)*1000 );
    }
    catch(CIMClientException& e)
    {
        PEGASUS_STD(cerr) << "Unable to connect to CIM Server." << PEGASUS_STD(endl);
        PEGASUS_STD(cerr) << "CIM Server may not be running." << PEGASUS_STD(endl);
        exit(0);
    }

    try
    {
        //
        // construct CIMObjectPath
        //
        String referenceStr = "//";
        referenceStr.append(hostStr);
        referenceStr.append(PEGASUS_NAMESPACENAME_SHUTDOWN);
        referenceStr.append(":");
        referenceStr.append(PEGASUS_CLASSNAME_SHUTDOWN);
        CIMObjectPath reference(referenceStr);

        //
        // issue the invokeMethod request on the shutdown method
        //
        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        // set force option to true for now
        inParams.append(CIMParamValue("force",
            CIMValue(Boolean(true))));

        inParams.append(CIMParamValue("timeout",
            CIMValue(Uint32(timeoutValue))));

        CIMValue retValue = client.invokeMethod(
            PEGASUS_NAMESPACENAME_SHUTDOWN,
            reference,
            "shutdown",
            inParams,
            outParams);
    }
    catch(CIMClientCIMException& e)
    {
        PEGASUS_STD(cerr) << "Failed to shutdown server: ";
        PEGASUS_STD(cerr) << e.getMessage() << PEGASUS_STD(endl);
        exit(1);
    }
    catch(CIMClientException& e)
    {
        //
        // This may mean the CIM Server has been terminated and returns a 
        // "Empty HTTP response message" HTTP error response.  To be sure
        // CIM Server gets shutdown, if CIM Server is still running at 
        // this time, we will kill the cimserver process.
        //
        // give CIM Server some time to finish up
        //
        System::sleep(1);
        cimserver_kill();
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error occurred while stopping the CIM Server: ";
        PEGASUS_STD(cerr) << e.getMessage() << PEGASUS_STD(endl);
        exit(1);
    }

    return;
}


/////////////////////////////////////////////////////////////////////////
//  MAIN
//////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    String pegasusHome  = String::EMPTY;
    Boolean pegasusIOLog = false;
    String httpPort = String::EMPTY;
    String httpsPort = String::EMPTY;
    String logsDirectory = String::EMPTY;
    Boolean useSLP = false;
    Boolean useSSL = false;
    Boolean daemonOption = false;
    Boolean shutdownOption = false;
    Uint32 timeoutValue  = 0;

    //
    // Get environment variables:
    //
    const char* tmp = getenv("PEGASUS_HOME");

    if (tmp)
    {
        pegasusHome = tmp;
    }

    FileSystem::translateSlashes(pegasusHome);

    // on Windows NT if there are no command-line options, run as a service

    if (argc == 1 )
    {
      cim_server_service(argc, argv);
    }
    else
    {
        // Get help, version, and shutdown options

        for (int i = 1; i < argc; )
        {
            const char* arg = argv[i];

            // Check for -option
            if (*arg == '-')
            {
                // Get the option
                const char* option = arg + 1;

                //
                // Check to see if user asked for the version (-v option):
                //
                if (*option == OPTION_VERSION)
                {
                    cout << PEGASUS_VERSION << endl;
                    exit(0);
                }
                //
                // Check to see if user asked for help (-h option):
                //
                else if (*option == OPTION_HELP)
                {
                    PrintHelp(argv[0]);
                    exit(0);
                }
                else if (*option == OPTION_HOME)
                {
                    if (i + 1 < argc)
                    {
                        pegasusHome.assign(argv[i + 1]);
                    }
                    else
                    {
                        cout << "Missing argument for option -" << option << endl;
                        exit(0);
                    }

                    memmove(&argv[i], &argv[i + 2], (argc-i-1) * sizeof(char*));
                    argc -= 2;
                }
#if defined(PEGASUS_OS_HPUX)
                //
                // Check to see if user asked for the version (-X option):
                //
                if (*option == OPTION_BINDVERBOSE)
                {
		    System::bindVerbose = true;
                    cout << "Unsupported debug option, BIND_VERBOSE, enabled." 
                         << endl;
                    // remove the option from the command line
                    memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
                    argc--;   
                }
#endif
                //
                // Check to see if user asked for shutdown (-s option):
                //
                else if (*option == OPTION_SHUTDOWN)
                {
                    //
                    // check to see if user is root
                    //
                    if (!System::isPrivilegedUser(System::getEffectiveUserName()))
                    {
                        cout << "You must have superuser privilege to run ";
                        cout << "cimserver." << endl;
                        exit(0);
                    }

                    //
                    // Check to see if shutdown has already been specified:
                    //
                    if (shutdownOption)
                    {
                        cout << "Duplicate shutdown option specified." << endl;
                        exit(0);
                    }

                    shutdownOption = true;
 
                    // remove the option from the command line
                    memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
                    argc--;   
                }
                else
                    i++;
            }
            else
                i++;
        }
    }

    //
    // Set the value for pegasusHome property
    //
    ConfigManager::setPegasusHome(pegasusHome);

    //
    // Get an instance of the Config Manager.
    //
    configManager = ConfigManager::getInstance();

    //
    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments from the command
    // line.
    //
    try
    {
        GetOptions(configManager, argc, argv, pegasusHome);
    }
    catch (Exception& e)
    {
        cerr << argv[0] << ": " << e.getMessage() << endl;
        exit(1);
    }

    try
    {
        //
        // Check to see if we should (can) install as a NT service
        //

        if (String::equal(configManager->getCurrentValue("install"), "true"))
        {
            if( 0 != cimserver_install_nt_service( pegasusHome ))
            {
                cout << "\nPegasus installed as NT Service";
                exit(0);
            }
        }

        //
        // Check to see if we should (can) remove Pegasus as an NT service
        //

        if (String::equal(configManager->getCurrentValue("remove"), "true"))
        {
            if( 0 != cimserver_remove_nt_service() )
            {
                cout << "\nPegasus removed as NT Service";
                exit(0);
            }
        }

        //
        // Check to see if we should Pegasus as a daemon
        //

        if (String::equal(configManager->getCurrentValue("daemon"), "true"))
        {
            daemonOption = true;
        }

        //
        // Check the log trace options and set global variable
        //

        if (String::equal(configManager->getCurrentValue("logtrace"), "true"))
        {
            pegasusIOLog = true;
        }

        // Get the log file directory definition.
        // We put String into Cstring because
        // Directory functions only handle Cstring.
        // ATTN-KS: create String based directory functions.

        logsDirectory = configManager->getCurrentValue("logdir");
        logsDirectory = 
	    ConfigManager::getHomedPath(configManager->getCurrentValue("logdir"));

        // Set up the Logger. This does not open the logs
        // Might be more logical to clean before set.
        // ATTN: Need tool to completely disable logging.

#ifndef PEGASUS_OS_HPUX
        Logger::setHomeDirectory(logsDirectory);
#endif

        //
        // Check to see if we need to shutdown CIMOM 
        //
        if (shutdownOption)
        {
            String configTimeout = 
                configManager->getCurrentValue("shutdownTimeout");
            ArrayDestroyer<char> timeoutCString(configTimeout.allocateCString());
            timeoutValue = strtol(timeoutCString.getPointer(), (char **)0, 10);
            
            shutdownCIMOM(timeoutValue);

            cout << "CIM Server stopped." << endl;
            exit(0);
        }

        //
        // Get the port numbers
        //

        httpPort = configManager->getCurrentValue("httpPort");

        httpsPort = configManager->getCurrentValue("httpsPort");

        // Leave this in until people get familiar with the logs.
#ifndef PEGASUS_OS_HPUX
        cout << "Logs Directory = " << logsDirectory << endl;
#endif

        if (String::equal(configManager->getCurrentValue("cleanlogs"), "true"))
        {
            Logger::clean(logsDirectory);;
        }

        if (String::equal(configManager->getCurrentValue("slp"), "true"))
        {
            useSLP =  true;
        }

        if (String::equal(configManager->getCurrentValue("SSL"), "true"))
        {
            useSSL =  true;
        }
    }
    catch (UnrecognizedConfigProperty e)
    {
        cout << "Error: " << e.getMessage() << endl;
    }

    Uint32 portNumber;

    char address[32];

    if (useSSL)
    {
        char* p = httpsPort.allocateCString();
	char* end = 0;
	Uint32 port = strtol(p, &end, 10);
	assert(end != 0 && *end == '\0');
	delete [] p;

        //
        // Look up the WBEM-HTTPS port number 
        //
        portNumber = System::lookupPort(WBEM_HTTPS_SERVICE_NAME, port);
        sprintf(address, "%u", portNumber);
    }
    else
    {
        char* p = httpPort.allocateCString();
	char* end = 0;
	Uint32 port = strtol(p, &end, 10);
	assert(end != 0 && *end == '\0');
	delete [] p;

        //
        // Look up the WBEM-HTTP port number 
        //
        portNumber = System::lookupPort(WBEM_HTTP_SERVICE_NAME, port);
        sprintf(address, "%u", portNumber);
    }

    // Put out startup up message.
    cout << PEGASUS_NAME << PEGASUS_VERSION <<
	 " on port " << address << endl;
    cout << "Built " << __DATE__ << " " << __TIME__ << endl;
    cout <<"Starting..."
         << (pegasusIOLog ? " Tracing to Log ": " ")
	 << (useSLP ? " SLP reg. " : " No SLP ")
         << (useSSL ? " Use SSL " : " No SSL ")
	<< endl;

    // do we need to run as a daemon ?
    if (daemonOption)
    {
        if(-1 == cimserver_fork())
          exit(-1);
    }

    // try loop to bind the address, and run the server
    try
    {
#if !defined(PEGASUS_OS_ZOS) && ! defined(PEGASUS_OS_HPUX)
      	slp_client *discovery = new slp_client() ;;
        String serviceURL;
	serviceURL.assign("service:cim.pegasus://");
	String host_name = slp_get_host_name();
	serviceURL += host_name;
	serviceURL += ":";
	serviceURL += address;
	char *url = serviceURL.allocateCString();
	//	free(host_name);
#endif

	Monitor monitor;
	CIMServer server(&monitor, useSSL);

	// bind throws an exception if the bind fails
#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
	cout << "Binding to domain socket" << endl;
#else
	cout << "Binding to " << address << endl;
#endif

	server.bind(portNumber);

	time_t last = 0;

#if defined(PEGASUS_OS_HPUX)
        //
        // create a file to indicate that the cimserver has started and
        // save the process id of the cimserver process in the file
        //

        // remove the old file if it exists
        System::removeFile(fname);

        // open the file
        FILE *pid_file = fopen(fname, "w");
        if (pid_file)
        {
            // save the pid in the file
            fprintf(pid_file, "%ld\n", (long)server_pid);
            fclose(pid_file);
        }
#endif
	cout << "Started. " << endl;

        // Put server started message to the logger
        Logger::put(Logger::STANDARD_LOG, "CIMServer", Logger::INFORMATION,
                    "Started $0 version $1 on port $2.",
                    PEGASUS_NAME, PEGASUS_VERSION, address);


        //
        // Loop to call CIMServer's runForever() method until CIMServer
        // has been shutdown
        //
	while( !server.terminated() )
	{
#if !defined(PEGASUS_OS_ZOS) && ! defined(PEGASUS_OS_HPUX)
	  if(useSLP  ) 
	  {
	    if(  (time(NULL) - last ) > 60 ) 
	    {
	      if( discovery != NULL && url != NULL )
		discovery->srv_reg_all(url,  
				       "(namespace=root/cimv2)",
				       "service:cim.pegasus", 
				       "DEFAULT", 
				       70) ;
	      time(&last);
	    }
	  
	    discovery->service_listener();
	  }
#endif
	  server.runForever();
	}

        //
        // normal termination
	//
        // Put server shutdown message to the logger
        Logger::put(Logger::STANDARD_LOG, "CIMServer", Logger::INFORMATION,
            "$0 stopped.", PEGASUS_NAME);

#if defined(PEGASUS_OS_HPUX)
        //
        // close the file created at startup time to indicate that the 
        // cimserver has terminated normally.
        //
        FileSystem::removeFile(CIMSERVERSTART_FILE);
#endif
    }
    catch(Exception& e)
    {
	PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        return 1;
    }

    return 0;
}
