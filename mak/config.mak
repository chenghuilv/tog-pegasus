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
################################################################################
##
## Get external environment variables. Note that all external environment
## variables begin with "PEGASUS_".
##
################################################################################

ifndef ROOT
    ROOT =  $(subst \,/,$(PEGASUS_ROOT))
endif

ifdef PEGASUS_ENVVAR_FILE
    include $(PEGASUS_ENVVAR_FILE)
else
    include $(ROOT)/env_var.status
endif

ifdef PEGASUS_HOME
    HOME_DIR = $(subst \,/,$(PEGASUS_HOME))
else
    $(error PEGASUS_HOME environment variable undefined)
endif

ifdef PEGASUS_ROOT
    ROOT =  $(subst \,/,$(PEGASUS_ROOT))
else
    $(error PEGASUS_ROOT environment variable undefined)
endif

# l10n
ifdef ICU_ROOT
    ICUROOT =  $(subst \,/,$(ICU_ROOT))
endif

ifdef PEGASUS_TMP
    TMP_DIR = $(subst \,/,$(PEGASUS_TMP))
else
    TMP_DIR = .
endif

ifdef PEGASUS_DISPLAYCONSUMER_DIR
    DISPLAYCONSUMER_DIR = $(subst \,/,$(PEGASUS_DISPLAYCONSUMER_DIR))
else
    DISPLAYCONSUMER_DIR = $(subst \,/,$(PEGASUS_HOME))
endif

ifdef PEGASUS_DEBUG
     PEGASUS_USE_DEBUG_BUILD_OPTIONS = 1
endif

PLATFORM_FILES=$(wildcard $(ROOT)/mak/platform*.mak)
PLATFORM_TEMP=$(subst $(ROOT)/mak/platform_,, $(PLATFORM_FILES))
VALID_PLATFORMS=$(subst .mak,  , $(PLATFORM_TEMP))

ifndef PEGASUS_PLATFORM
    $(error PEGASUS_PLATFORM environment variable undefined. Please set to\
        one of the following:$(VALID_PLATFORMS))
endif

################################################################################
ifeq ($(findstring _GNU, $(PEGASUS_PLATFORM)), _GNU)
    ifdef CXX
      GCC_VERSION = $(shell $(CXX) -dumpversion)
    else
      GCC_VERSION = $(shell g++ -dumpversion)
    endif
else
    GCC_VERSION =
endif

#############################################################################
## As a general rule, the directory structure for the object files mirrors
## the directory structure of the source files.  E.g.,
## $PEGASUS_HOME/obj/Pegasus/Common contains the object files for the
## source files in $PEGASUS_ROOT/src/Pegasus/Common.  Each source-level
## Makefile includes a DIR value that defines this common path (e.g.,
## Pegasus/Common). In a small number of cases, source files are built
## multiple times with difference compile options.  
## To handle this situation, the ALT_OBJ_DIR variable can be used to
## specify an alternative object directory for use in building the
## objects defined in the Makefile.
##

ifndef ALT_OBJ_DIR
    OBJ_DIR = $(HOME_DIR)/obj/$(DIR)
else
    OBJ_DIR = $(HOME_DIR)/obj/$(ALT_OBJ_DIR)
endif

#############################################################################

BIN_DIR = $(HOME_DIR)/bin
LIB_DIR = $(HOME_DIR)/lib

# l10n
# define the location for the compiled messages
MSG_ROOT = $(HOME_DIR)/msg

#############################################################################
##  The following REPOSITORY_XXX variables are only used within the
## makefiles for building the repository (cimmofl) and running the tests.
## They have no effect on CIMconfig initial startup configuration.

#
# define the location for the repository
#
# NOTE: There is another variable efined in many of the test makefiles
# called REPOSITORYDIR. It is a local variable in each of those Makefiles
# to localally control where the temporay small repository they
# build, use and then delete is located. Most of the time it is set to TMP_DIR.
#

REPOSITORY_DIR = $(HOME_DIR)

#
# WARNING: The REPOSITORY_NAME varible is not used by all the test,
# many of them are still hardcoded to "repository".  What this means
# is that you can change the repository name and build it. But you
# cannot run the test without many of them failing
#

REPOSITORY_NAME = repository


REPOSITORY_ROOT = $(REPOSITORY_DIR)/$(REPOSITORY_NAME)

# define the repository mode
#       XML = XML format
#       BIN = Binary format
#
ifndef PEGASUS_REPOSITORY_MODE
   ## set to default value
   REPOSITORY_MODE = XML
else
   ## validate assigned value
   ifeq ($(PEGASUS_REPOSITORY_MODE),XML)
       REPOSITORY_MODE = XML
   else
     ifeq ($(PEGASUS_REPOSITORY_MODE),BIN)
      REPOSITORY_MODE = BIN
     else
      $(error PEGASUS_REPOSITORY_MODE ($(PEGASUS_REPOSITORY_MODE)) \
		 is invalid. It must be set to either XML or BIN)
     endif
   endif
endif


###########################################################################

# The two variables, CIM_SCHEMA_DIR and CIM_SCHEMA_VER,
# are used to control the version of the CIM Schema
# loaded into the Pegasus Internal, InterOp,
# root/cimv2 and various test namespaces.
#
# Update the following two environment variables to
# change the version.

# The environment variable PEGASUS_CIM_SCHEMA can be used
# to change the values of CIM_SCHEMA_DIR, CIM_SCHEMA_VER
# and ALLOW_EXPERIMENTAL.
#
# To use the PEGASUS_CIM_SCHEMA variable the Schema mof
# files must be placed in the directory
# $(PEGASUS_ROOT)/Schemas/$(PEGASUS_CIM_SCHEMA)
#
# The value of PEGASUS_CIM_SCHEMA must conform to the
# following syntax:
#
#        CIM[Prelim]<CIM_SCHEMA_VER>
#
# The string "Prelim" should be included if the
# Schema contains "Experimental" class definitions.
#
# The value of <CIM_SCHEMA_VER> must be the value
# of the version string included by the DMTF as
# part of the mof file names (e.g, CIM_Core27.mof).
# Therefore, for example, the value of <CIM_SCHEMA_VER>
# for CIM27 Schema directories MUST be 27.
#
# Examples of valid values of PEGASUS_CIM_SCHEMA
# include CIMPrelim27, CIM27, CIMPrelim28, and CIM28.
#
# Note the CIMPrelim271 would NOT be a valid value
# for PEGASUS_CIM_SCHEMA because the version string
# portion of the mof files (e.g., CIM_Core27.mof) in
# the CIMPrelimin271 directory is 27 not 271.

# ***** CIM_SCHEMA_DIR INFO ****
# If CIM_SCHEMA_DIR changes to use a preliminary schema which
# has experimentals make sure and change the path below to appopriate
# directory path.  Example:  CIMPrelim271 is preliminary and has
# experimental classes.  Since experimental classes exist the -aE
# option of the mof compiler needs to be set.
# *****

ifdef PEGASUS_CIM_SCHEMA
    CIM_SCHEMA_DIR=$(PEGASUS_ROOT)/Schemas/$(PEGASUS_CIM_SCHEMA)
    ifeq ($(findstring $(patsubst CIM%,%,$(patsubst CIMPrelim%,%,$(PEGASUS_CIM_SCHEMA))),1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 271 28),)
       CIM_SCHEMA_VER=
    else
       CIM_SCHEMA_VER=$(patsubst CIM%,%,$(patsubst CIMPrelim%,%,$(PEGASUS_CIM_SCHEMA)))
    endif
else
    CIM_SCHEMA_DIR=$(PEGASUS_ROOT)/Schemas/CIM2131
    CIM_SCHEMA_VER=
endif

ifneq (, $(findstring Prelim, $(CIM_SCHEMA_DIR)))
    ALLOW_EXPERIMENTAL = -aE
else
    ALLOW_EXPERIMENTAL =
endif

LEX = flex

## ======================================================================
##
## PEGASUS_ENABLE_SORTED_DIFF
## This controls if the DIFFSORT function is used rather than a simple DIFF of
##  of the test results files to the static results file.
##
##   Set to "true" enables the sorted diffs of results to static results files.
##   otherwise results in regular diffs of results to static results files.
##   see bug 2283 for background information concerning this config variable.
##
##  Defaults to true.
##
##
ifndef PEGASUS_ENABLE_SORTED_DIFF
PEGASUS_ENABLE_SORTED_DIFF=true
endif

## ========================================================================
## DIFFSORT function definition
## Here is an example using the DIFFSORT function:
##
## difftest: FORCE
##      @ test > result
##      @ $(call DIFFSORT,result,standard_result)
##      @ $(ECHO) +++++ all test passed
##

define NL


endef

ifndef FORCE_NOCASE

DIFFSORT = $(SORT) $(1) > $(1).tmp $(NL) \
$(SORT) $(2) > $(2).tmp $(NL) \
$(DIFF) $(1).tmp $(2).tmp $(NL) \
$(RM) -f $(1).tmp $(NL) \
$(RM) -f $(2).tmp $(NL)

else

DIFFSORT = $(SORT) -f $(1) > $(1).tmp $(NL) \
$(SORT) -f $(2) > $(2).tmp $(NL) \
$(DIFF) -i $(1).tmp $(2).tmp $(NL) \
$(RM) -f $(1).tmp $(NL) \
$(RM) -f $(2).tmp $(NL)

endif

DIFFSORTNOCASE = $(SORT) $(1) > $(1).tmp $(NL) \
$(SORT) $(2) > $(2).tmp $(NL) \
$(DIFF) -i $(1).tmp $(2).tmp $(NL) \
$(RM) -f $(1).tmp $(NL) \
$(RM) -f $(2).tmp $(NL)

#
# The following is used to define the usage message for MakeFile
#
# See the pegasus/Makfile for an exampleof its usage.
#
USAGE = @$(ECHO) " $(1)"

################################################################################
##
## Attempt to include a platform configuration file:
##
################################################################################

PLATFORM_FILE = $(ROOT)/mak/platform_$(PEGASUS_PLATFORM).mak
ifneq ($(wildcard $(PLATFORM_FILE)), )
    include $(PLATFORM_FILE)
else
  $(error  PEGASUS_PLATFORM environment variable must be set to one of\
        the following:$(VALID_PLATFORMS))
endif

################################################################################
##
##  Set up any platform independent compile conditionals by adding them to
##  precreated FLAGS parameter.
##  Assumes that the basic flags have been setup in FLAGS.
##  Assumes that compile time flags are controlled with -D CLI option.
##
################################################################################


################################################################################
##
## PEGASUS_MAX_THREADS_PER_SVC_QUEUE
##
## Controls the maximum number of threads allowed per message service queue.
##     It is allowed to range between 1 and MAX_THREADS_PER_SVC_QUEUE_LIMIT
##     as set in pegasus/src/Pegasus/Common/MessageQueueService.cpp.  If the
##     specified value is out of range, MAX_THREADS_PER_SVC_QUEUE_LIMIT is
##     used.  The default value is MAX_THREADS_PER_SVC_QUEUE_DEFAULT, as
##     defined in pegasus/src/Pegasus/Common/MessageQueueService.cpp.
##
##	Label					Current value
##	--------------------------------------  -------------
##      MAX_THREADS_PER_SVC_QUEUE_LIMIT	        5000
##      MAX_THREADS_PER_SVC_QUEUE_DEFAULT       5
##
##

ifdef PEGASUS_MAX_THREADS_PER_SVC_QUEUE
  DEFINES += -DMAX_THREADS_PER_SVC_QUEUE=$(PEGASUS_MAX_THREADS_PER_SVC_QUEUE)
endif

##############################################################################
##
## PEGASUS_INDICATIONS_Q_THRESHOLD
##
## Controls if indications providers are stalled if the indications
## service queue is too large.
##
##      defaults to not set.
##
## 	It can be set to any positive value.
##
## If not set providers are never stalled. This implies that the
## indications service queue may become as large as neccesary to hold all
## the indicaitons generated.
##
## If set to any value then providers are stalled by forcing them to sleep
## when they try to deliver an indication and the indications service queue
## exceeds this value. They are resumed when the queue count falls 10 percent
## below this value.
##
## Stall and resume log entries are made to inform the administrator
## the condition has occured.
##
## WARNING: This also affects the Out of Process Providers (OOP Providers)
##    The OOP Providers use two one way pipes for communication.
##    By stalling the Provider this prevents the pipe from being read
##    which will cause the pipe to fill up and the remote side will block.
##    OOP Prividers mix indications and operations on these two pipes.
##    This means the operations will also be blocked as a side effect of
##    the indications being stalled.
##
##

ifdef PEGASUS_INDICATIONS_Q_THRESHOLD
  DEFINES += -DPEGASUS_INDICATIONS_Q_THRESHOLD=$(PEGASUS_INDICATIONS_Q_THRESHOLD)
endif


# Allow PEGASUS_ASSERT statements to be disabled.
ifdef PEGASUS_NOASSERTS
    DEFINES += -DNDEBUG
endif

# do not compile trace code. sometimes it causes problems debugging
ifdef PEGASUS_REMOVE_TRACE
    DEFINES += -DPEGASUS_REMOVE_TRACE
endif

# PEP 161
# Control whether utf-8 filenames are supported by the repository
ifdef PEGASUS_SUPPORT_UTF8_FILENAME
    DEFINES += -DPEGASUS_SUPPORT_UTF8_FILENAME

    # Control whether utf-8 filenames in the repository are escaped
    ifdef PEGASUS_REPOSITORY_ESCAPE_UTF8
        DEFINES += -DPEGASUS_REPOSITORY_ESCAPE_UTF8
    endif
endif

#
# PEP 142
# The following flag need to be set to enable
# user group authorization functionality.
#
ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    DEFINES += -DPEGASUS_ENABLE_USERGROUP_AUTHORIZATION
endif

#
# PEP 193
# The following flag need to be set to disable
# CQL in indication subscriptions
#
ifdef PEGASUS_DISABLE_CQL
    DEFINES += -DPEGASUS_DISABLE_CQL
endif

#
# PEP 186
# Allow override of product name/version/status.  A file
# pegasus/src/Pegasus/Common/ProductVersion.h must exist when this
# flag is defined.
#
ifdef PEGASUS_OVERRIDE_PRODUCT_ID
    DEFINES += -DPEGASUS_OVERRIDE_PRODUCT_ID
endif

#
# PEP 72
# Allow Out-of-Process Providers to be disabled by default
#
ifdef PEGASUS_DEFAULT_ENABLE_OOP
  ifeq ($(PEGASUS_DEFAULT_ENABLE_OOP),true)
    DEFINES += -DPEGASUS_DEFAULT_ENABLE_OOP
  else
    ifneq ($(PEGASUS_DEFAULT_ENABLE_OOP),false)
      $(error PEGASUS_DEFAULT_ENABLE_OOP ($(PEGASUS_DEFAULT_ENABLE_OOP)) invalid, must be true or false)
    endif
  endif
endif

#
# Allow to define the default value for the Provider User Context
# property as REQUESTOR.
# If is set and true use REQUESTOR
# If is not set or false use PRIVILEGED
#
ifdef PEGASUS_DEFAULT_USERCTXT_REQUESTOR
  ifeq ($(PEGASUS_DEFAULT_USERCTXT_REQUESTOR),true)
    DEFINES += -DPEGASUS_DEFAULT_USERCTXT_REQUESTOR
  else
    ifneq ($(PEGASUS_DEFAULT_USERCTXT_REQUESTOR),false)
      $(error PEGASUS_DEFAULT_USERCTXT_REQUESTOR ($(PEGASUS_DEFAULT_USERCTXT_REQUESTOR)) invalid, must be true or false)
    endif
  endif
endif

#
# PEP 197
# Allow the Provider User Context feature to be disabled.
#
ifdef PEGASUS_DISABLE_PROV_USERCTXT
    DEFINES += -DPEGASUS_DISABLE_PROV_USERCTXT
endif

# Bug 2147
# Allow local domain socket usage to be disabled.
ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    DEFINES += -DPEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
endif

# PEP 211
# Controls object normalization support.
ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
    DEFINES += -DPEGASUS_ENABLE_OBJECT_NORMALIZATION
endif

# PEP 233
# Controls support for EmbeddedInstance properties
# and parameters
ifndef PEGASUS_EMBEDDED_INSTANCE_SUPPORT
    PEGASUS_EMBEDDED_INSTANCE_SUPPORT = true
endif

ifeq ($(PEGASUS_EMBEDDED_INSTANCE_SUPPORT), true)
    DEFINES += -DPEGASUS_EMBEDDED_INSTANCE_SUPPORT
else
    ifneq ($(PEGASUS_EMBEDDED_INSTANCE_SUPPORT), false)
        $(error PEGASUS_EMBEDDED_INSTANCE_SUPPORT ($(PEGASUS_EMBEDDED_INSTANCE_SUPPORT)) invalid, must be true or false)
    endif
endif


# Allow ExecQuery functionality to be enabled
ifndef PEGASUS_ENABLE_EXECQUERY
    DEFINES += -DPEGASUS_DISABLE_EXECQUERY
endif

# Allow System Log Handler to be enabled
ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
  DEFINES += -DPEGASUS_ENABLE_SYSTEM_LOG_HANDLER
endif

# Allow Email Handler to be enabled
ifdef PEGASUS_ENABLE_EMAIL_HANDLER
  DEFINES += -DPEGASUS_ENABLE_EMAIL_HANDLER
endif

# Allow qualifiers to be disabled for instance operations
ifdef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
  DEFINES += -DPEGASUS_DISABLE_INSTANCE_QUALIFIERS
endif

# Controls snmp indication handler to use NET-SNMP to deliver trap
ifdef PEGASUS_USE_NET_SNMP
  DEFINES += -DPEGASUS_USE_NET_SNMP
endif

ifdef PEGASUS_HAS_SSL
    DEFINES += -DPEGASUS_HAS_SSL 

    # Enable SSL Random file by default.
    ifndef PEGASUS_USE_SSL_RANDOMFILE
        PEGASUS_USE_SSL_RANDOMFILE = true
    endif

    # Allow SSL Random file functionality to be optionally disabled.
    ifdef PEGASUS_USE_SSL_RANDOMFILE
        ifeq ($(PEGASUS_USE_SSL_RANDOMFILE), true)
            DEFINES += -DPEGASUS_SSL_RANDOMFILE
        else
            ifneq ($(PEGASUS_USE_SSL_RANDOMFILE), false)
                $(error PEGASUS_USE_SSL_RANDOMFILE\
                     ($(PEGASUS_USE_SSL_RANDOMFILE)) invalid, \
                      must be true or false)
            endif
        endif
    endif

    ifndef OPENSSL_COMMAND
        ifdef OPENSSL_BIN
            OPENSSL_COMMAND = $(OPENSSL_BIN)/openssl
        else
            OPENSSL_COMMAND = openssl
        endif
    endif
    ifndef OPENSSL_SET_SERIAL_SUPPORTED
        ifneq (, $(findstring 0.9.6, $(shell $(OPENSSL_COMMAND) version)))
            OPENSSL_SET_SERIAL_SUPPORTED = false
        else
            OPENSSL_SET_SERIAL_SUPPORTED = true
        endif
    endif

    # Enable CRL verification
    ifndef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
        PEGASUS_ENABLE_SSL_CRL_VERIFICATION = true
    endif

    # Check for Enable SSL CRL verification
    ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
        ifeq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), true)
            DEFINES += -DPEGASUS_ENABLE_SSL_CRL_VERIFICATION
        else
            ifneq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), false)
                $(error PEGASUS_ENABLE_SSL_CRL_VERIFICATION\
                     ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION)) invalid, \
                      must be true or false)
            endif
        endif
    endif
endif

#
# PEP 258
# Allow Audit Logger to be disabled.  It is enabled by default.
#

ifndef PEGASUS_ENABLE_AUDIT_LOGGER
    PEGASUS_ENABLE_AUDIT_LOGGER = true
endif

ifdef PEGASUS_ENABLE_AUDIT_LOGGER
    ifeq ($(PEGASUS_ENABLE_AUDIT_LOGGER),true)
        DEFINES += -DPEGASUS_ENABLE_AUDIT_LOGGER
    else
        ifneq ($(PEGASUS_ENABLE_AUDIT_LOGGER),false)
            $(error PEGASUS_ENABLE_AUDIT_LOGGER \
              ($(PEGASUS_ENABLE_AUDIT_LOGGER)) invalid, must be true or false)
        endif
    endif
endif

# Check for use of deprecated variable
ifdef PEGASUS_DISABLE_AUDIT_LOGGER
    $(error The PEGASUS_DISABLE_AUDIT_LOGGER variable is deprecated. \
        Use PEGASUS_ENABLE_AUDIT_LOGGER=false instead)
endif


#
# PEP 291
# Enable IPv6 support
#

ifndef PEGASUS_ENABLE_IPV6
    PEGASUS_ENABLE_IPV6 = true
endif

# Check for Enable IPv6 support
ifdef PEGASUS_ENABLE_IPV6
  ifeq ($(PEGASUS_ENABLE_IPV6),true)
    DEFINES += -DPEGASUS_ENABLE_IPV6
  else
    ifneq ($(PEGASUS_ENABLE_IPV6),false)
      $(error PEGASUS_ENABLE_IPV6 ($(PEGASUS_ENABLE_IPV6)) \
       invalid, must be true or false)
    endif
  endif
endif

# Verify Test IPv6 support
# If PEGASUS_ENABLE_IPV6 is defined and PEGASUS_TEST_IPV6 is not defined, we set
# PEGASUS_TEST_IPV6 to the same value as PEGASUS_ENABLE_IPV6.
# You can explicitly set PEGASUS_TEST_IPV6 to false if you don't want to run the
# IPv6 tests (for example, on an IPv4 system that is running an IPv6-enabled
# version of Pegasus).
#
ifdef PEGASUS_TEST_IPV6
  ifneq ($(PEGASUS_TEST_IPV6),true)
    ifneq ($(PEGASUS_TEST_IPV6),false)
      $(error PEGASUS_TEST_IPV6 ($(PEGASUS_TEST_IPV6)) \
       invalid, must be true or false)
    endif
  endif
else 
  PEGASUS_TEST_IPV6 = $(PEGASUS_ENABLE_IPV6)
endif

############################################################################
#
# PEGASUS_ENABLE_SLP and PEGASUS_DISABLE_SLP
#
# PEGASUS_DISABLE_SLP has been depracated. New use model is:
#
# Use PEGASUS_ENABLE_SLP=true  to enable  compilation of SLP functions.
#
# Use PEGASUS_ENABLE_SLP=false to disable compilation of SLP functions.
#
# Currently (Aug. 12, 2005) Windows is the only platform that enables SLP
# by default.
#
# NOTE. Effective with Bug # 2633 some platforms enable SLP.
# To see which platforms look for platform make files that set
# the variable PEGASUS_ENABLE_SLP.
#
#

ifdef PEGASUS_ENABLE_SLP
  ifdef PEGASUS_DISABLE_SLP
    $(error Conflicting defines PEGASUS_ENABLE_SLP and PEGASUS_DISABLE_SLP both set)
  endif
endif

ifdef PEGASUS_DISABLE_SLP
    $(error PEGASUS_DISABLE_SLP has been deprecated. Please use PEGASUS_ENABLE_SLP=[true/false] )

PEGASUS_ENABLE_SLP=false

endif

ifdef PEGASUS_ENABLE_SLP
  ifeq ($(PEGASUS_ENABLE_SLP),true)
    DEFINES += -DPEGASUS_ENABLE_SLP
  else
    ifneq ($(PEGASUS_ENABLE_SLP),false)
      $(error PEGASUS_ENABLE_SLP ($(PEGASUS_ENABLE_SLP)) invalid, must be true or false)
    endif
  endif
endif


############################################################################
#
# PEGASUS_USE_OPENSLP
#
# Environment variable to set openslp as SLP environment to use
# for SLP Directory and User Agents.
#
# Allows enabling use of openslp interfaces for slp instead of the
# internal pegasus slp agent.  Note that this does not disable the
# compilation of the internal agent code, etc.  However, it assumes
# openslp is installed on the platform and changes the interfaces
# to match this.  At this moment, this is a change specifically for
# adaptec but we expect to generalize it to provide openslp as a
# generalized alternative to ldapslp.
# to use this. To set this function up,
#
# Use this variable in conjunction with PEGASUS_OPENSLP_HOME
# to enable OpenSlp as the slp implementation.
#
# NOTE that it has no affect if the PEGASUS_ENABLE_SLP etc. flags are not set.
#

ifdef PEGASUS_USE_OPENSLP
   ifeq ($(PEGASUS_ENABLE_SLP),true)
      DEFINES += -DPEGASUS_USE_OPENSLP
    else
      $(error PEGASUS_USE_OPENSLP defined but PEGASUS_ENABLE_SLP is not true. Please correct this inconsistency)
    endif
endif

# PEP 267
# SLP reregistration support.
# PEGASUS_SLP_REG_TIMEOUT is defined as the SLP registration timeout
# interval, in minutes.
ifdef PEGASUS_SLP_REG_TIMEOUT
    ifeq ($(PEGASUS_ENABLE_SLP),true)
       DEFINES += -DPEGASUS_SLP_REG_TIMEOUT=$(PEGASUS_SLP_REG_TIMEOUT)
     else
       $(error PEGASUS_SLP_REG_TIMEOUT defined but PEGASUS_ENABLE_SLP is not true. Please correct this inconsistency)
     endif
 endif

############################################################################
#
# PEGASUS_OPENSLP_HOME
#
# Environment variable to set home location for OpenSLP include and library
# files if they are located somewhere other than /usr/include and /usr/lib.
#
# PEGASUS_USE_OPENSLP must also be defined for this environment variable
# to have any effect.
#
# This is the directory level within which both the include and lib
# directories holding the OpenSLP files will be found.
#
# EG: If the are located in /opt/OpenSLP/include and /opt/OpenSLP/lib
#     then this environment variable should be set to /opt/OpenSLP.
#


#
# Enable this flag to allow the handshake to continue regardless of verification result
#
ifdef PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
  DEFINES += -DPEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
endif

############################################################################
#
# PEGASUS_ENABLE_INTEROP_PROVIDER
# Enables the interop provider AND the server profile.
# initially this was activated by setting either the perfinst or slp enable
# flags.  This allows activating this function without any either perfinst or
# slp enabled.  Note that if either of these are enabled, this funtion is also
# enabled

## if either slp or perfinst are enabled and this is false, flag error
## This gets messy because should account for both postive and negative on
## interop so we don't get multiples.

ifdef PEGASUS_ENABLE_SLP
    ifeq ($(PEGASUS_ENABLE_SLP),true)
        ifndef PEGASUS_ENABLE_INTEROP_PROVIDER
            PEGASUS_ENABLE_INTEROP_PROVIDER = true
        else
            ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
                $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true if SLP enabled)
            endif
        endif
    endif
endif

## if PERFINST enabled, set to force interop.
ifndef PEGASUS_DISABLE_PERFINST
    ifndef PEGASUS_ENABLE_INTEROP_PROVIDER
        PEGASUS_ENABLE_INTEROP_PROVIDER = true
    else
        ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
            $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true if PERFINST enabled)
        endif
    endif
endif

ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
    ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
        DEFINES += -DPEGASUS_ENABLE_INTEROP_PROVIDER
    else
        ifneq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
            $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true or false)
        endif
    endif
endif


############################################################################
# set PEGASUS_DEBUG into the DEFINES if it exists.
# Note that this flag is the general separator between
# debug compiles and non-debug compiles and controls both
# the use of any debug options on compilers and linkers
# and general debug support that we want to be turned on in
# debug mode.
ifdef PEGASUS_DEBUG
    DEFINES += -DPEGASUS_DEBUG

    # Indications debugging options
    ifdef PEGASUS_INDICATION_PERFINST
        DEFINES += -DPEGASUS_INDICATION_PERFINST
    endif

    ifdef PEGASUS_INDICATION_HASHTRACE
        DEFINES += -DPEGASUS_INDICATION_HASHTRACE
    endif

    # Setup the conditional compile for client displays.
    ifdef PEGASUS_CLIENT_TRACE_ENABLE
        DEFINES += -DPEGASUS_CLIENT_TRACE_ENABLE
    endif
endif

# compile in the experimental APIs
DEFINES += -DPEGASUS_USE_EXPERIMENTAL_INTERFACES

# Ensure that the deprecated interfaces are defined in the Pegasus libraries.
# One may wish to disable these interfaces if binary compatibility with
# previous Pegasus releases is not required.
ifndef PEGASUS_DISABLE_DEPRECATED_INTERFACES
    DEFINES += -DPEGASUS_USE_DEPRECATED_INTERFACES
endif

# Set compile flag to control compilation of CIMOM statistics
ifdef PEGASUS_DISABLE_PERFINST
    DEFINES += -DPEGASUS_DISABLE_PERFINST
endif

# Set compile flag to control compilation of SNIA Extensions
ifdef PEGASUS_SNIA_EXTENSIONS
    DEFINES += -DPEGASUS_SNIA_EXTENSIONS
endif

ifdef PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
    ifeq ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER), true)
        DEFINES += -DPEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
    else
        ifneq ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER), false)
            $(error PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER \
                 ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER)) invalid, \
                  must be true or false)
        endif
    endif
endif

ifdef PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER
    ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
        DEFINES += -DPEGASUS_ENABLE_JMPI_PROVIDER_MANAGER
    else
        ifneq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), false)
            $(error PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER \
                 ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER)) invalid, \
                  must be true or false)
        endif
    endif
endif

# Allow remote CMPI functionality to be enabled
ifdef PEGASUS_ENABLE_REMOTE_CMPI
    DEFINES += -DPEGASUS_ENABLE_REMOTE_CMPI
endif

############################################################
#
# Set any vendor-specific compile flags
#
############################################################

ifdef PEGASUS_VENDOR_HP
    DEFINES+= -DPEGASUS_VENDOR_HP
endif


############################################################
#
# Set up other Make Variables that depend on platform config files
#
############################################################

# This is temporary until we end up with a better place to
# put this variable
# Makefiles can do directory remove with
# $(RMREPOSITORY) repositoryname
#
RMREPOSITORY = $(RMDIRHIER)

ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    DEFINES += -DPEGASUS_USE_RELEASE_CONFIG_OPTIONS
endif

ifdef PEGASUS_USE_RELEASE_DIRS
    DEFINES += -DPEGASUS_USE_RELEASE_DIRS
endif

ifdef PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
    DEFINES += -DPEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
endif

# Unless otherwise specified, Pegasus libraries go in $(PEGASUS_HOME)/lib
ifndef PEGASUS_DEST_LIB_DIR
    PEGASUS_DEST_LIB_DIR = lib
endif

ifeq ($(OS),VMS)
 DEFINES += -DPEGASUS_DEST_LIB_DIR="""$(PEGASUS_DEST_LIB_DIR)"""
else
 DEFINES += -DPEGASUS_DEST_LIB_DIR=\"$(PEGASUS_DEST_LIB_DIR)\"
endif

################################################################################
##
## PEGASUS_CLASS_CACHE_SIZE
##
##     This environment variable gives the size of the class cache used by
##     the CIM repository. When it is undefined, the size defaults to something
##     relatively small (see src/Pegasus/Repository/CIMRepository.cpp). If
##     defined, it gives the size of the class cache. If it is 0 , the class
##     cache is not defined compiled in at all.
##
################################################################################

ifdef PEGASUS_CLASS_CACHE_SIZE
DEFINES += -DPEGASUS_CLASS_CACHE_SIZE=$(PEGASUS_CLASS_CACHE_SIZE)
endif

################################################################################
##
## Additional build flags passed in through environment variables.
## These flags are added to the compile/link commands.
##
################################################################################

ifdef PEGASUS_EXTRA_CXX_FLAGS
    EXTRA_CXX_FLAGS = $(PEGASUS_EXTRA_CXX_FLAGS)
endif

ifdef PEGASUS_EXTRA_C_FLAGS
    EXTRA_C_FLAGS = $(PEGASUS_EXTRA_C_FLAGS)
endif

ifdef PEGASUS_EXTRA_LINK_FLAGS
    EXTRA_LINK_FLAGS = $(PEGASUS_EXTRA_LINK_FLAGS)
endif

##==============================================================================
##
## By definining PEGASUS_USE_STATIC_LIBRARIES in the environment and STATIC
## in the Makefile, a static library is produced rather than a shared one.
## PEGASUS_USE_STATIC_LIBRARIES should be "true" or "false".
##
##==============================================================================

ifdef PEGASUS_USE_STATIC_LIBRARIES
  ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),true)
  else
    ifneq ($(PEGASUS_USE_STATIC_LIBRARIES),false)
      $(error PEGASUS_USE_STATIC_LIBRARIES ($(PEGASUS_USE_STATIC_LIBRARIES)) invalid, must be true or false)
    endif
  endif
endif

##==============================================================================
##
## PEGASUS_ENABLE_PRIVILEGE_SEPARATION
##
##     Enables privilege separation support (uses the executor process to
##     perform privileged operations).
##
##==============================================================================

ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
  ifeq ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION),true)
    DEFINES += -DPEGASUS_ENABLE_PRIVILEGE_SEPARATION
  else
    ifneq ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION),false)
      $(error PEGASUS_ENABLE_PRIVILEGE_SEPARATION \
        ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)) invalid, must be true or false)
    endif
  endif

  ## Defines the user context of the cimservermain process when privilege
  ## separation is enabled.
  PEGASUS_CIMSERVERMAIN_USER = cimsrvr
  DEFINES += -DPEGASUS_CIMSERVERMAIN_USER=\"$(PEGASUS_CIMSERVERMAIN_USER)\"
endif

##==============================================================================
##
## PEGASUS_USE_PAM_STANDALONE_PROC
##
##==============================================================================

ifdef PEGASUS_USE_PAM_STANDALONE_PROC
  DEFINES += -DPEGASUS_USE_PAM_STANDALONE_PROC
endif

##==============================================================================

ifndef PEGASUS_JAVA_CLASSPATH_DELIMITER
    PEGASUS_JAVA_CLASSPATH_DELIMITER = :
endif

ifndef PEGASUS_JVM
	PEGASUS_JVM = sun
endif
ifeq ($(PEGASUS_JVM),gcj)
	PEGASUS_JAVA_COMPILER		= gcj -C
	PEGASUS_JAVA_JAR		= fastjar
	PEGASUS_JAVA_INTERPRETER	= gij
else
	PEGASUS_JAVA_COMPILER		= javac -target 1.4 -source 1.4
	PEGASUS_JAVA_JAR		= jar
	PEGASUS_JAVA_INTERPRETER	= java
endif
