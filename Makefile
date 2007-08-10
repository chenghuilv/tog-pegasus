#//%2006////////////////////////////////////////////////////////////////////////
#//
#// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
#// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
#// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
#// IBM Corp.; EMC Corporation, The Open Group.
#// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
#// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
#// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
#// EMC Corporation; VERITAS Software Corporation; The Open Group.
#// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
#// EMC Corporation; Symantec Corporation; The Open Group.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a copy
#// of this software and associated documentation files (the "Software"), to
#// deal in the Software without restriction, including without limitation the
#// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
#// sell copies of the Software, and to permit persons to whom the Software is
#// furnished to do so, subject to the following conditions:
#// 
#// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
#// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
#// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
#// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
#// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
#// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//==============================================================================
# Pegasus top level make file
# see the usage rule for options

ROOT = .

include $(ROOT)/env_var.status
include $(ROOT)/mak/config.mak

# This is a recurse make file
# Defines subdirectorys to go to recursively

# DIRS = src cgi
DIRS = src test Schemas

# Define the inclusion of the recurse.mak file to execute the next
# level of makefiles defined by the DIRS variable

defaultrule: all setupdevserver

include $(ROOT)/mak/recurse.mak

.PHONY: FORCE

FORCE:

usage: FORCE
	$(USAGE)
	$(USAGE)"Makefile targets:"
	$(USAGE)
	$(USAGE)"Recursive rules - These are the primatives that traverse the tree"
	$(USAGE)"invoking the specified command in each subdirectory directory."
	$(USAGE)"NOTE: all is special, it specifies no target and therefore invokes"
	$(USAGE)"the default rule for that directory."
	$(USAGE)"all                 - recursive DEFAULT rule"
	$(USAGE)"clean               - recursive clean"
	$(USAGE)"depend              - buildmu recursive depend"
	$(USAGE)"messages            - rootbundle recursive messages"
	$(USAGE)"tests               - recursive tests"
	$(USAGE)"poststarttests      - recursive poststarttests"
	$(USAGE) 
	$(USAGE)"Combinational rules - Combine other rules to achieve results"
	$(USAGE)"DEFAULT RULE        - all, setupdevserver"
	$(USAGE)"new                 - clean repositoryclean"
	$(USAGE)"build               - depend all, setupdevserver"
	$(USAGE)"world               - build unittests servertests"
	$(USAGE)
	$(USAGE)"Functional rules - Other rules to achieve specified results"
	$(USAGE)"clobber             -removes objects built during compile"
	$(USAGE)"                     specifically the following directories are removed:"
	$(USAGE)"                      $(PEGASUS_HOME)/bin"
	$(USAGE)"                      $(PEGASUS_HOME)/lib"
	$(USAGE)"                      $(PEGASUS_HOME)/obj"
	$(USAGE)"buildmu             - builds the mu utility"
	$(USAGE)"setupdevserver      - setup the development server env"
	$(USAGE)"cleandevserver      - cleans the development server env"
	$(USAGE)"repository          - builds the base repository. Does not remove other"
	$(USAGE)"                      namespaces than the base namespaces."  
	$(USAGE)"testrepository      - builds items for the test suites into the repository"
	$(USAGE)"removetestrepository- removes test items from the repository"
	$(USAGE)"repositoryclean     - removes the complete repository"
	$(USAGE)"listplatforms       - List all valid platforms"
	$(USAGE)
	$(USAGE)"Test rules (accessable here but implemented in TestMakefile)"
	$(USAGE)"alltests            - unittests and servertests"
	$(USAGE)"unittests           - runs the unit functional test"
	$(USAGE)"servertests         - runs basic server tests"
	$(USAGE)"perftests           - runs basic server performance tests"
	$(USAGE)"standardtests       - runs server extended tests"
	$(USAGE)"testusage           - TestMakefile usage"
	$(USAGE)"testusage2          - TestMakefile usage2"
	$(USAGE)"stresstests         - Runs the default stresstests"
	$(USAGE)
	$(USAGE)"--------------------"
	$(USAGE)"Quick start:"
	$(USAGE)"  After checkout of new tree:"
	$(USAGE)"  use \"make listplatforms\" to view a list of platforms"
	$(USAGE)"  set PEGASUS_PLATFORM=<your platofrm>"
	$(USAGE)"  set PEGASUS_ROOT=<location of your pegasus source>"
	$(USAGE)"  set PEGASUS_HOME=<build output location"
	$(USAGE)"  make world"
	$(USAGE)
	$(USAGE)"  This will build everthing with a default configuration"
	$(USAGE)"  and run the automated tests."
	$(USAGE)
	$(USAGE)"--------------------"
	$(USAGE)"Examples:"
	$(USAGE)"  After \"cvs checkout\" of new tree:    make world"
	$(USAGE)
	$(USAGE)"  After changes to include files:      make"
	$(USAGE)
	$(USAGE)"  After changes to the files included: make build"
	$(USAGE)
	$(USAGE)"  After \"cvs update\" or to start over: make new world" 
	$(USAGE)

listplatforms: FORCE
	$(USAGE)
	$(USAGE)"The $(words $(VALID_PLATFORMS)) valid platforms are:"
	$(USAGE)" $(foreach w, $(VALID_PLATFORMS), " $w ")"
	$(USAGE)
	$(USAGE)

#########################################################################
# This section defines any prerequisites that are required by the 
# recursive rules.
#
# NOTE: You can add prerequisties for the recursive rules but you cannot
#       add any commands to run as part of the rule. You can define them 
#       and make will quietly ignore them and they will not be run either
#       before or after the recursive rule. 
#
#
messages: rootbundle

depend: buildmu

#########################################################################
# This section defines combinational rules
#
#-----------------------
# build target: builds all source
#
build: depend all setupdevserver

#------------------------
# rebuild target is being deprecated instead use "make new build"
#
rebuild_msg: FORCE
	@$(ECHO) "==============================================================================="
	@$(ECHO) "Makefile: The rebuild target is being deprecated." 
	@$(ECHO) "          Use \"make usage\" for a description of the usage model."
	@$(ECHO) "          Consider using \"make new world\" ."
	@$(ECHO) "          Invoking the old rebuild rule now."
	@$(ECHO) "==============================================================================="

rebuild: rebuild_msg shortsleep new build s_unittests repository

#-----------------------
# new target: cleans everthing
#
# This can be combined on the command line with other rules like:
#
# make new build
# make new world 

new: clean repositoryclean

#-----------------------
# world targets: builds everything and dependent on which target may do testing
#
#       Typically used after a fresh checkout from CVS 

world: build s_unittests servertests


############################
#
# rules defined in TestMakefile that are repeated here for convenience
#
shortsleep: FORCE
	@$(MAKE)  -f TestMakefile shortsleep

servertests: FORCE
	@ $(MAKE) -f TestMakefile servertests

perftests: FORCE
	@ $(MAKE) -f TestMakefile perftests

s_unittests: FORCE
	@ $(MAKE) -f TestMakefile -s unittests

unittests: FORCE
	@ $(MAKE) -f TestMakefile unittests

standardtests: FORCE
	@ $(MAKE) -f TestMakefile standardtests

alltests: FORCE
	@ $(MAKE) -f TestMakefile alltests

testusage: FORCE
	@ $(MAKE) -f TestMakefile usage

testusage2: FORCE
	@ $(MAKE) -f TestMakefile usage2

stresstests:
	@$(ECHO) "Running OpenPegasus StressTests"
	@$(MAKE)  -f TestMakefile stresstests
	@$(ECHO) "+++++ OpenPegasus StressTests complete"

##########################################################################
#
# This section defines functional rules
#
#---------------------
# buildmu target: build mu the make utility that among other things
#                 includes depend
buildmu: FORCE
	$(MKDIRHIER) $(BIN_DIR)
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/utils/mu -f Makefile

#----------------------
# setupdevserver and cleandevserver are used to setup and clear the 
# server configuration files needed to run the server in a development
# environment. 
#
setupdevserver: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/Server -f Makefile install_run
	@$(ECHO) "PEGASUS Development Server Runtime Environment configured "

cleandevserver: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/Server -f Makefile install_run_clean

clobber: FORCE
	- $(RMDIRHIER) $(PEGASUS_HOME)/bin
	- $(RMDIRHIER) $(PEGASUS_HOME)/lib
	- $(RMDIRHIER) $(PEGASUS_HOME)/obj


#---------------------
# The repository Target removes and rebuilds the base repository. It
# does not remove all possible namespaces.  See
# Schemas/Pegasus/Makefile for details. The repository clean has the
# same limitation

# Note: Arguments must be quoted to preserve upper case characters in VMS.
repository: FORCE
	@ $(MAKE) "-SC" Schemas/Pegasus repository

repositoryclean: FORCE
	@ $(RMREPOSITORY) $(REPOSITORY_ROOT)

repositoryServer: FORCE
	@ $(MAKE) "-SC" Schemas/Pegasus repositoryServer

testrepository: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load repository
	@ $(MAKE) "-SC" test/wetest repository
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load repository
	@ $(MAKE) "-SC" src/Pegasus/CQL/CQLCLI repository
	@ $(MAKE) "-SC" src/Pegasus/Query/QueryExpression/tests repository
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load repository
ifndef PEGASUS_DISABLE_CQL
	@ $(MAKE) "-SC" src/Pegasus/ControlProviders/QueryCapabilitiesProvider/tests repository
endif
ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
	@ $(MAKE) "-SC" src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/tests repository
endif
	@ $(MAKE) --directory=$(PEGASUS_ROOT)/src/Clients/cimsub/tests/testscript \
            -f Makefile repository

testrepositoryServer: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load repositoryServer
	@ $(MAKE) "-SC" test/wetest repositoryServer
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load repositoryServer
	@ $(MAKE) "-SC" src/Pegasus/CQL/CQLCLI repositoryServer
	@ $(MAKE) "-SC" src/Pegasus/Query/QueryExpression/tests repositoryServer
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load repositoryServer
ifndef PEGASUS_DISABLE_CQL
	@ $(MAKE) "-SC" src/Pegasus/ControlProviders/QueryCapabilitiesProvider/tests repositoryServer
endif
ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
	@ $(MAKE) "-SC" src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/tests repositoryServer
endif
	@ $(MAKE) --directory=$(PEGASUS_ROOT)/src/Clients/cimsub/tests/testscript \
            -f Makefile repositoryServer

removetestrepository: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load removerepository
	@ $(MAKE) "-SC" test/wetest removerepository
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load removerepository
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load removerepository
	@ $(MAKE) "-SC" src/Clients/cimsub/tests/testscript removerepository

config:
	@ $(ROOT)/SetConfig_EnvVar

rootbundle: 
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/utils/cnv2rootbundle -f Makefile
