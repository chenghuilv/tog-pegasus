# Pegasus top level make file
# options are
# Make rebuild
# Make world
# Make tests - Executes the complete test suite
# Make repository - Rebuilds the Pegasus repository
#
ROOT = .

include $(ROOT)/env_var.status
include $(ROOT)/mak/config.mak

# This is a recurse make file
# Defines subdirectorys to go to recursively

# DIRS = src cgi
DIRS = src test rpm Schemas

# Define the inclusion of the recurse.mak file to execute the next
# level of makefiles defined by the DIRS variable

include $(ROOT)/mak/recurse.mak

# rebuild target cleans, setup dependencies, compiles all and builds 
# repository

FORCE:

rebuild: clean depend all repository
	@ $(MAKE) -s tests

world: depend all repository
	@ $(MAKE) -s tests

# The repository Target removes and rebuilds the CIM repository

# Note: Arguments must be quoted to preserve upper case characters in VMS.
repository: FORCE
	@ $(MAKE) "-SC" Schemas/Pegasus repository

repositoryServer: FORCE
	@ $(MAKE) "-SC" Schemas/Pegasus repositoryServer

testrepository: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load repository
	@ $(MAKE) "-SC" test/wetest repository
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load repository
	@ $(MAKE) "-SC" src/Pegasus/CQL/CQLCLI repository
	@ $(MAKE) "-SC" src/Pegasus/Query/QueryExpression/tests repository
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load repository

testrepositoryServer: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load repositoryServer
	@ $(MAKE) "-SC" test/wetest repositoryServer
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load repositoryServer
	@ $(MAKE) "-SC" src/Pegasus/CQL/CQLCLI repositoryServer
	@ $(MAKE) "-SC" src/Pegasus/Query/QueryExpression/tests repositoryServer
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load repositoryServer

removetestrepository: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load removerepository
	@ $(MAKE) "-SC" test/wetest removerepository
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load removerepository
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load removerepository

config:
	@ $(ROOT)/SetConfig_EnvVar

all: messages 

messages: rootbundle

rootbundle: 
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/utils/cnv2rootbundle -f Makefile

# the collections of tests that we run with the server active.
# For now, these are centralized and do not include startup
# and shutdown of the server.

activetests: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/test -f Makefile clean
	$(PEGASUS_ROOT)/bin/TestClient
	$(PEGASUS_ROOT)/bin/Client
	$(MAKE) --directory=$(PEGASUS_ROOT)/test -f Makefile tests
