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
// Author: John Alex (IBM)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_StressTestControllerCommand_h
#define Pegasus_StressTestControllerCommand_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/HashTable.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>
//#include "StressTestControllerException.h"

struct tm getCurrentActualTime(void);

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// ConfigTable to contain the stressTest configurations
////////////////////////////////////////////////////////////////////////////////
typedef HashTable<String, String, EqualFunc<String>, HashFunc<String> > Table;

/**
    This constant signifies that an operation option has not been recorded
*/
static const Uint32 OPERATION_TYPE_UNINITIALIZED             = 0;

/**
    This constant represents a help operation
*/
static const Uint32 OPERATION_TYPE_HELP                      = 1;

/**
    This constant represents a version display operation
*/
static const Uint32 OPERATION_TYPE_VERSION                   = 2;


static Boolean  verboseEnabled                               = false;


/**
   The StressTestController command requires CIM Server to be
   installed and running on the target host.

 */
class StressTestControllerCommand : public Command
{
public:

    enum { DEFAULT_TIMEOUT_MILLISECONDS = 200000 };

    /**

        Constructs an StressTestControllerCommand and initializes 
        instance variables.

     */

    StressTestControllerCommand ();
    /**

        Parses the command line, validates the options, and sets instance
        variables based on the option arguments.

        @param   argc  the number of command line arguments
        @param   argv  the string vector of command line arguments

        @exception  CommandFormatException  if an error is encountered in
                                            parsing the command line

     */

    void setCommand (Uint32 argc, char* argv []);

    /**

        Executes the command and writes the results to the PrintWriters.

        @param   outPrintWriter     the ostream to which output should be
                                    written
        @param   errPrintWriter     the ostream to which error output should be
                                    written

        @return  0                  if the command is successful
                 1                  if an error occurs in executing the command

     */
    Uint32 execute (ostream& outPrintWriter, ostream& errPrintWriter);

    /**

        Generates the actual commands for each of the test clients.

        @return  true               if the commands are generated successfully
                 false              if unsuccessful in generating commands.

     */
    Boolean generateClientCommands(ostream& log_file);

    /**
        Retrieves the config information from the stressController configuration
        file.

        @param   filePath     the filename itself may or may not include an 
                              absolute path.
        @return  
     */
    Boolean getFileContent (String filePath,ostream& log_file);

    /**
        Retrieves the config file path name

        @return _configFilePath  The pathname is returned.
 
     */
    String getConfigFilePath(){ return _configFilePath;}

    /**
        Checks to verify if new pathname was opted.

        @return true   A path name was set.
                false  Use the default pathname.
 
     */
    Boolean IsConfigFilePathSpecified(){return _configFilePathSpecified;}

    /**
        Retrieves the command operation type 

        @return _operationType  The operation type is returned.
 
     */
    Uint32 getOperationType(){return _operationType;}

    /**
        Populate test with default configuration values if default configuration file
        is not found.

        @return 
               
     */
    void getDefaultClients(ostream& log_file);

    /**
        The command name.
     */
    static const char   COMMAND_NAME [];


    /**
       Will generate or create all the required files for the tests.
       Required log files, pid files etc are created here.
       
       @parm    strTime         The time stamp for the tests.

       @return  

    */
    Boolean generateRequiredFileNames(char *strTime);

    /**
       Will remove all the unused files for the tests.
       - Unused log files & pid files are removed here.

    */
    void removeUnusedFiles(void);

    /**
        Retrieves the stress Test log file path name

        @return StressTestLogFile  The log filename is returned.
 
     */
    String getStressTestLogFile(){ return StressTestLogFile;}

    static char   FILENAME [];

    static char TESTDIR[];
    static char STRESSTESTDIR[];
    static char LOGDIR[];
    static char BINDIR[];
    static char DEFAULT_DIR[];
    static char DEFAULT_CFGDIR[];
    static char DEFAULT_LOGDIR[];
    static char DEFAULT_TMPDIR[];

    // Environment variables:
    //
    char* pegasusRoot;

    char* pegasusHome;

private:

    /**
        Parses each line in the stressController config file

        @return true   Parse was successful.
                false  Parsing resulted in a failure or invalid config.
     */
    Boolean parseLine(const String & string, int line,String & name, String & value,ostream& log_file);

    /**
        Stores client details from config file

        @return true   Successfully retrieved and stored the details.
                false  Un-successfull.
     */
    Boolean storeClientDetails(String name, String value);

    /**
        Validate StressTest configuration properties & clients

        @return true   Successfully validated.
                false  Un-successfull.
     */
    Boolean validateConfiguration(String & var, const String & value,ostream& log_file);
    
    /**
        parse and save all the options for each client 

        @return 
 
     */
    void getClientOptions(const Char16* p,ostream& log_file);

    /**
        Checks if the actual tolerance level of the current tests
        are within expected tolerances.

        @return true   Within tolerance level
                false  Failed tolerance expectations.
 
     */
    Boolean checkToleranceLevel(int act_clients,Uint64 nowMilliseconds,ostream& log_file);

    /**
        Retrieves and stores client details from client pid file

        @return true   Successfully retrieved and stored the details.
                false  Un-successfull.
     */
    Boolean getClientPIDs(int clients,ostream& log_file);


    /**
        The host on which the command is to be executed.  A CIM Server must be
        running on this host when the command is executed.  The default host is
        the local host.
     */
    String _hostName;

    /**
        A Boolean indicating whether a hostname was specified on the command
        line.  The default host is the local host.
     */
    Boolean _hostNameSpecified;

    /**
        The timeout value to be used, in milliseconds.
        The default timeout value is DEFAULT_TIMEOUT_MILLISECONDS.
     */
    Uint32 _timeout;


    /**

        The port to be used when the command is executed. The
        port number must be the port number on which the
        target CIM Server is running.

        If no port is specified, StressTestController will attempt to locate 
        the port associated with the service "wbem-http" for non-SSL connections
        and "wbem-https" for SSL connections. If not found, the DMTF
        recommended default port number will be used.

        _portNumberStr and _portNumber are not used with
        connectLocal().

        The default port for non-SSL 5988 and 5989 for SSL.

     */
    String _portNumberStr;
    Uint32 _portNumber;
    Uint32   _port;  // JA not sure..if needed

    /**
        A Boolean indicating whether a port number was specified on the
	command line.
     */
    Boolean _portNumberSpecified;

    /**
        The username to be used for authentication and
        authorization of the operation.
     */
    String _userName;

    /**
        Indicates that the user name is set.
     */
    Boolean _userNameSpecified;

    /**
        The password to be used for authentication.
     */
    String _password;

    /**
        Indicates that the password is set.
     */
    Boolean _passwordSpecified;

    /**
        A Boolean indicating whether an SSL connection was specified on the
        command line.
     */
    Boolean _useSSL;

    /**
        The duration of the stress tests
     */
    static double _duration;

    /**
        Indicates that the duration is set.
     */
    Boolean _durationSpecified;

    /**
        The ToleranceLevel used for the stress tests
     */
    Uint32 _toleranceLevel;

    /**
        Indicates that the toleranceLevel is set.
     */
    Boolean _toleranceLevelSpecified;

    /**
        The NameSpace that may be used by the clients
     */
    String _nameSpace;

    /**
        Indicates that the Name Space is set.
     */
    Boolean _nameSpaceSpecified;

    /**
        The Class Name that may be used by the clients
     */
    String _className;

    /**
        Indicates that the Class Name is set.
     */
    Boolean _ClassNameSpecified;


    /**
        The option character used to specify the hostname.
     */
    static const char   _OPTION_HOSTNAME;

    /**
        The option character used to specify the port number.
     */
    static const char   _OPTION_PORTNUMBER;

    /**
        The option character used to specify the timeout value.
     */
    static const char   _OPTION_TIMEOUT;

    /**
        The option character used to specify the username.
     */
    static const char   _OPTION_USERNAME;

    /**
        The option character used to specify the password.
     */
    static const char   _OPTION_PASSWORD;

    /**
        The option character used to specify whether SSL should be used.
     */
    static const char   _OPTION_SSL;

    /**
        Label for the usage string for this command.
     */
    static const char   _USAGE [];

    /**
        The minimum valid portnumber.
     */
    static const Uint32 _MIN_PORTNUMBER;

    /**
        The maximum valid portnumber.
     */
    static const Uint32 _MAX_PORTNUMBER;

    /**
        The minimum valid duration level.
     */
    static const Uint32 _MIN_DURATION;

    /**
        The minimum valid tolerance level.
     */
    static const Uint32 _MIN_TOLERANCE;

    /**
        The maximum valid tolerance level.
     */
    static const Uint32 _MAX_TOLERANCE;

    /**
        The path of the config file containing the configuration info.
     */
    String _configFilePath;

    /**
        A Boolean indicating whether a config file path was specified on the
        command line.
     */
    Boolean _configFilePathSpecified;

    /**
        A String containing the usage information for the command.
     */
    String usage;

    /**
        The type of operation specified on the command line.
     */
    Uint32        _operationType;

    Table* _propertyTable;
    Table* _clientTable;
    String* _clientCommands;
    Uint64* _clientDurations;
    Uint64* _clientDelays;

    /**
        The total number of clients for this test
     */
    Uint32 _clientCount;
    String _currClient;
    Uint32 _currClientCount;
    /**
      Stress Controller Log file.
    */
    String StressTestLogFile;

    /**
      StressTest Client PID file
    */
    String StressTestClientPIDFile;

    /**
      StressTest Client Log file
    */
    String StressTestClientLogFile;

    /**
      StressTest temporary Client PID file
    */
    String tmpStressTestClientPIDFile;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_StressTestControllerCommand_h */
