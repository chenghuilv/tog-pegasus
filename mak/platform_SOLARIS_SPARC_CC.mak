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
include $(ROOT)/mak/config-unix.mak

OS = solaris

ARCHITECTURE = sparc

COMPILER = CC

CC = cc

#
# This is a hack because the Pegasus build system doesn't have a way to specify
# flags just for the C compiler or just for the C++ compiler.
#
CXX = CC -pto

SH = sh

YACC = yacc

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .so

PEGASUS_SUPPORTS_DYNLIB = yes

SYS_INCLUDES = 

DEFINES = -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM) -D_POSIX_PTHREAD_SEMANTICS

DEFINES += -DPEGASUS_OS_SOLARIS

# PEGASUS_SNIA_INTEROP_TEST is currently (1/27/06) tested  and set in these 
# platform files:
#
#    platform_SOLARIS_SPARC_CC.mak
#    platform_WIN32_IX86_MSVC.mak
#
ifdef PEGASUS_SNIA_INTEROP_TEST
DEFINES += -DPEGASUS_SNIA_INTEROP_TEST
endif

#
# This is needed for SPARC.  It shouldn't be needed for x86
# or x86-64 if a port is ever done for Solaris on those
# platforms.
#
DEFINES += -DTYPE_CONV

# "READBUG" forces fstream.read to read 1 char at a time to
# overcome a bug in Wshop 6.2
# There are patches for this now.
#
# DEFINES += -DPEGASUS_OS_SOLARIS_READBUG

SUNOS_VERSION = $(shell uname -r)


# Pegasus requires the kernel LWP thread model.
# It doesn't exist on SunOS 5.6 or 5.7 so thery are no longer supported.
#
ifeq ($(SUNOS_VERSION), 5.6)
DEFINES += -DSUNOS_5_6
    $(error SunOS version 5.6 is not supportted)
endif

# Pegasus requires the kernel LWP thread model.
# It doesn't exist on SunOS 5.6 or 5.7 so thery are no longer supported.
#
ifeq ($(SUNOS_VERSION), 5.7)
DEFINES += -DSUNOS_5_7
    $(error SunOS version 5.7 is not supportted)
endif

ifeq ($(SUNOS_VERSION), 5.8)
DEFINES += -DSUNOS_5_8
endif

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS 
FLAGS = -g -KPIC -mt -xs -xildoff
else
FLAGS = -O4 -KPIC -mt -xildoff -s -xipo=1
endif

# Need warnings:
FLAGS += +w

##==============================================================================
##
## COMMON_SYS_LIBS
##
##     Build the common list of libraries used in linking both libraries and
##     programs.
##
##==============================================================================

COMMON_SYS_LIBS = -lpthread -ldl -lsocket -lnsl -lxnet -lCstd

ifeq ($(SUNOS_VERSION), 5.6)
COMMON_SYS_LIBS += -lposix4
else
COMMON_SYS_LIBS += -lrt
endif

# on SunOS 5.8 use the alternate (kernel LWP) thread model that is standard on 
# SunOS 5.9 and 5.10
# 
ifeq ($(SUNOS_VERSION), 5.8)
COMMON_SYS_LIBS += -R /usr/lib/lwp
endif

##==============================================================================
##
## SYS_LIBS (system libraries needed to build programs)
##
##==============================================================================
SYS_LIBS = $(COMMON_SYS_LIBS) $(EXTRA_LIBRARIES)

##==============================================================================
##
## LIBRARY_SYS_LIBS (system libraries needed to build other libraries)
##
##==============================================================================
LIBRARY_SYS_LIBS = $(COMMON_SYS_LIBS)

