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
// Author: John Alex
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "StressTestController.h"
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/FileSystem.h>
//#define DEBUG
#include <time.h>
//Windows
#ifdef PEGASUS_PLATFORM_WIN32_IX86_MSVC
#include <windows.h>     /* for DWORD etc. */
typedef DWORD pid_t;     /* getpid() and others */
#include <process.h>
#elif !defined(PEGASUS_OS_OS400)
#include <unistd.h>
#endif


/**
   Signal handler set up SIGALARM.
*/
static Boolean useDefaults                                   = false;


/**
 * Message resource name
 */
static const char MSG_PATH []                                = "pegasus/pegasusCLI";

static const char REQUIRED_ARGS_MISSING []                   =
                     "Required arguments missing.";
static const char REQUIRED_ARGS_MISSING_KEY []               =
                     "Clients.cimuser.CIMUserCommand.REQUIRED_ARGS_MISSING";

static const char ERR_OPTION_NOT_SUPPORTED []                =
                     "Invalid option. Use '--help' to obtain command syntax.";

static const char ERR_OPTION_NOT_SUPPORTED_KEY[]             =
                     "Clients.cimuser.CIMUserCommand..ERR_OPTION_NOT_SUPPORTED";

static const char ERR_USAGE []                               =
                     "Incorrect usage. Use '--help' to obtain command syntax.";

static const char ERR_USAGE_KEY []                           =
                     "Clients.cimuser.CIMUserCommand..ERR_USAGE";


// exclude main from the Pegasus Namespace
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


int main (int argc, char* argv [])
{
    char     strTime[256];
    struct tm tmTime;
    int      rc;
    String   fileName = String::EMPTY;
    ofstream log_file;


    tmTime = getCurrentActualTime();
    strftime(strTime,256,"%d%m%Y%H%M%S.",&tmTime);
   
    StressTestControllerCommand    command=StressTestControllerCommand();

    // generate log files and PID files
    if(!command.generateRequiredFileNames(strTime))
    {
       cout<<StressTestControllerCommand::COMMAND_NAME<<"::Failed to generate required files for tests. "<<endl;
       command.removeUnusedFiles();
       exit (Command::RC_ERROR);
    }

    // open the log file
    OpenAppend(log_file,command.getStressTestLogFile());

    if (!log_file)
    {
       log_file.close();
       cout<<"Cannot get file "<<command.getStressTestLogFile()<<endl;
       command.removeUnusedFiles();
       exit (Command::RC_ERROR);
       
    }
    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);
    log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Initiated on "<<strTime<<endl;
    log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Process ID: "<<getpid()<<endl;
    

    MessageLoader::setPegasusMsgHomeRelative(argv[0]);
    try
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME;
        log_file<<"::Geting Command Options."<<endl;
        if(verboseEnabled)
        {
           cout<<StressTestControllerCommand::COMMAND_NAME;
           cout<<"::Getting Command options."<<endl;
        }
        //
        // validate and set command arguments
        //
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        String msg(cfe.getMessage());

        log_file << StressTestControllerCommand::COMMAND_NAME << "::" << msg <<  endl;
        cerr << StressTestControllerCommand::COMMAND_NAME << "::" << msg <<  endl;

        if (msg.find(String("Unknown flag")) != PEG_NOT_FOUND)
        {
           MessageLoaderParms parms(ERR_OPTION_NOT_SUPPORTED_KEY,ERR_OPTION_NOT_SUPPORTED);
              parms.msg_src_path = MSG_PATH;
              cerr << StressTestControllerCommand::COMMAND_NAME <<
                   ": " << MessageLoader::getMessage(parms) << endl;
              log_file<< StressTestControllerCommand::COMMAND_NAME <<
                   ": " << MessageLoader::getMessage(parms) << endl;
        }
        else
        {
           MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
              parms.msg_src_path = MSG_PATH;
           cerr << StressTestControllerCommand::COMMAND_NAME <<
             ": " << MessageLoader::getMessage(parms) << endl;
           log_file << StressTestControllerCommand::COMMAND_NAME <<
             ": " << MessageLoader::getMessage(parms) << endl;
        }

        log_file.close();
        command.removeUnusedFiles();
        exit (Command::RC_ERROR);
    }
    catch(...)
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Unknown exception caught when setting commands."<<endl;
        cerr<<StressTestControllerCommand::COMMAND_NAME<<"::Unknown exception caught when setting commands."<<endl;
        log_file.close();
        command.removeUnusedFiles();
        exit (Command::RC_ERROR);
    }


    // For help or version options execute usage/version and 
    // exit
    if((command.getOperationType() == OPERATION_TYPE_HELP)
       ||(command.getOperationType() == OPERATION_TYPE_VERSION))
    {
        rc = command.execute (cout, cerr);
        log_file.close();
        // 
        // Log file not required when help or verbose is opted.
        //
        FileSystem::removeFile(command.getStressTestLogFile());
        exit (rc);
    }

    String filename;

    //
    // If a configuration file is specified then:
    //    Check if it exists as indicated, if not 
    //    also look for it in the default config dir.
    //
    if(command.IsConfigFilePathSpecified())
    {
       filename = command.getConfigFilePath();
       FileSystem::translateSlashes(filename);
       //
       // Check whether the file exists or not
       //
       if (!FileSystem::exists(filename))
       {
           // Check for file in default directory as well
           fileName = String::EMPTY;
           fileName.append(command.pegasusRoot);
           fileName.append(StressTestControllerCommand::DEFAULT_CFGDIR);
           fileName.append(filename);

           if (!FileSystem::exists(fileName))
           {
              cerr << StressTestControllerCommand::COMMAND_NAME ;
              cerr << "::Specified Configuration file \""<<filename;
              cerr << "\" does not exist."<<endl;
              log_file.close();
              command.removeUnusedFiles();
              exit (Command::RC_ERROR);
           }
           log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Using config file: "<<fileName<<endl;
       } 
       else 
          fileName = filename;

       log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Using config file: "<<fileName<<endl;
       cout<<StressTestControllerCommand::COMMAND_NAME<<"::Using config file: "<<fileName<<endl;
    } 
    else
    {
       //
       // Use default file in default dir.
       //
       fileName = String::EMPTY;
       fileName.append(command.pegasusRoot);
       fileName.append(StressTestControllerCommand::DEFAULT_CFGDIR);
       fileName.append(StressTestControllerCommand::FILENAME);
       //
       // Use hard coded default configuration values if default conf. file 
       // was not found.
       if (!FileSystem::exists(fileName))
       {
            //
            // Use Hard-coded default values
            //
            useDefaults = true;
       }
       else
       {
           log_file << StressTestControllerCommand::COMMAND_NAME <<
                "::Using default file: " << fileName<<endl;
           cout << StressTestControllerCommand::COMMAND_NAME <<
                "::Using default file: " << fileName<<endl;
       }
    }
   
    //
    // Read the contents of the file
    //
    try
    {
       //
       // Use Hard-coded default values
       //
       if(useDefaults)
       {
           log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Using hard coded default config values."<<endl;
           cout<<StressTestControllerCommand::COMMAND_NAME<<"::Using hard coded default config values."<<endl;
           command.getDefaultClients(log_file);
       }
       else
       {
           log_file << StressTestControllerCommand::COMMAND_NAME <<
                "::Reading config file: " << fileName<<endl;
           if(verboseEnabled)
           {
               cout << StressTestControllerCommand::COMMAND_NAME <<
                        "::Reading config file: " << fileName<<endl;
           }
           command.getFileContent(fileName,log_file);
       }
    }
    catch(NoSuchFile& e)
    {
        String msg(e.getMessage());

        log_file << StressTestControllerCommand::COMMAND_NAME << ": " << msg <<  endl;
        cerr << StressTestControllerCommand::COMMAND_NAME << ": " << msg <<  endl;
        log_file.close();
        command.removeUnusedFiles();
        exit (Command::RC_ERROR);

    }
    catch(Exception& e )
    {
        String msg(e.getMessage());
        log_file << StressTestControllerCommand::COMMAND_NAME << "::" << msg <<  endl;
        cerr << StressTestControllerCommand::COMMAND_NAME << "::Invalid Configuration ";
        cerr << "in File: " << fileName <<  endl;
        cerr << msg <<  endl;
        log_file.close();
        command.removeUnusedFiles();
        exit (Command::RC_ERROR);
    }
    catch(...)
    {
        // throw what was caught
        log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Unknown exception caught when acquiring configuration."<<endl;
        cerr<<StressTestControllerCommand::COMMAND_NAME<<"::Unknown exception caught when acquiring configuration."<<endl;
        log_file.close();
        command.removeUnusedFiles();
        exit (Command::RC_ERROR);
    }

    log_file << StressTestControllerCommand::COMMAND_NAME << "::Generating Client Commands"<<  endl;
    if(verboseEnabled)
        cout << StressTestControllerCommand::COMMAND_NAME << "::Generating Client Commands"<<  endl;


    // 
    // TimeStamp 
    //
    log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Initiated on "<<strTime<<endl;
    log_file<<StressTestControllerCommand::COMMAND_NAME<<"::Process ID: "<<getpid()<<endl;
    cout<<StressTestControllerCommand::COMMAND_NAME<<"::Initiated on "<<strTime<<endl;
    cout<<StressTestControllerCommand::COMMAND_NAME<<"::Process ID: "<<getpid()<<endl;


    if(!command.generateClientCommands(log_file))
    {
       cerr << StressTestControllerCommand::COMMAND_NAME << "::Failed to Generate Client Commands."<<  endl;
       log_file << StressTestControllerCommand::COMMAND_NAME << "::Failed to Generate Client Commands."<<  endl;
       log_file.close();
       command.removeUnusedFiles();
       exit (Command::RC_ERROR);
    }
    //
    // Getting current time
    //
    tmTime = getCurrentActualTime();
    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);
    log_file << StressTestControllerCommand::COMMAND_NAME <<endl;
    log_file << "   Preparing to execute Clients on "<<strTime<<endl;
    // Begin to run stress Tests 
    rc = command.execute (cout, cerr);
    //
    // Getting current time after stress Tests are completed
    //
    tmTime = getCurrentActualTime();
    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);
    //
    // Check overall status of tests
    //
    if(rc)
    {
       log_file << StressTestControllerCommand::COMMAND_NAME;
       log_file << "::execution interrupted on "<<strTime<<endl;
       cout << StressTestControllerCommand::COMMAND_NAME;
       cout << "::execution interrupted on "<<strTime<<endl;
    } else {
       log_file << StressTestControllerCommand::COMMAND_NAME;
       log_file << "::successfully completed on "<<strTime<<endl;
       cout << StressTestControllerCommand::COMMAND_NAME;
       cout << "::successfully completed on "<<strTime<<endl;
    }
    log_file.close();
    exit (rc);
    return rc;
}
