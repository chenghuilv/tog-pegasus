# Configuration options for Pegasus on all architectures running Linux

include $(ROOT)/mak/config-unix.mak

PEGASUS_PLATFORM_LINUX_GENERIC_GNU = 1
DEFINES += -DPEGASUS_PLATFORM_LINUX_GENERIC_GNU
DEFINES += -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)

OS = linux

COMPILER = gnu

PLATFORM_VERSION_SUPPORTED = yes

CXX = g++

SH = sh

YACC = yacc

COPY = cp

MOVE = mv

PEGASUS_SUPPORTS_DYNLIB = yes

MAJOR_VERSION_NUMBER = 1

LIB_SUFFIX = .so.$(MAJOR_VERSION_NUMBER)

DEFINES += -DPEGASUS_USE_SYSLOGS

DEFINES += -DPEGASUS_HAS_SIGNALS

SYS_LIBS = -ldl -lpthread -lcrypt

ifdef PEGASUS_HAS_SSL
  SYS_LIBS += -lssl
endif

# Build using fixed release settings
#
ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
FLAGS += -DPEGASUS_USE_RELEASE_CONFIG_OPTIONS
endif
ifdef PEGASUS_USE_RELEASE_DIRS
FLAGS += -DPEGASUS_USE_RELEASE_DIRS
endif

# PAM support
ifdef PEGASUS_PAM_AUTHENTICATION
   ifeq ($(HPUX_IA64_VERSION), yes)
      SYS_LIBS += -L$(PAMLIB_HOME) -lpam
   else
      SYS_LIBS += -lpam
   endif

## The following flags need to be set or unset
## to compile-in the code required for PAM authentication
## and compile-out the code that uses the password file.

 DEFINES += -DPEGASUS_PAM_AUTHENTICATION -DPEGASUS_NO_PASSWORDFILE

endif


ifdef PEGASUS_DEBUG
FLAGS += -g -fPIC -W -Wall -Wno-unused  -D_GNU_SOURCE -DTHREAD_SAFE -D_REENTRANT -DPEGASUS_DEBUG
else
FLAGS += -O2 -fPIC -W -Wall -Wno-unused -D_GNU_SOURCE -DTHREAD_SAFE -D_REENTRANT
endif

ifdef PEGASUS_HAS_PERFINST
  FLAGS += -DPEGASUS_HAS_PERFINST
endif

ifndef PEGASUS_USE_MU_DEPEND
PEGASUS_HAS_MAKEDEPEND = yes
endif

ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
 FLAGS += -DPEGASUS_LOCAL_DOMAIN_SOCKET
endif

ifdef USE_CONNECTLOCAL
 FLAGS += -DUSE_CONNECTLOCAL
endif

# Warning: Setting the PEGASUS_USE_DEPRECATED_INTERFACES flag exposes
# deprecated interfaces in externally-facing header files.  The deprecated
# interfaces may not be compatible with subsequent Pegasus releases.
# This flag should be used only for transitionary compatibility with
# existing code and not for new development.

ifdef PEGASUS_USE_DEPRECATED_INTERFACES
  DEFINES += -DPEGASUS_USE_DEPRECATED_INTERFACES
endif

# l10n
ifdef PEGASUS_HAS_MESSAGES
  DEFINES += -DPEGASUS_HAS_MESSAGES
  ifdef ICU_ROOT
	MSG_COMPILE = genrb
	MSG_FLAGS =
	MSG_SOURCE_EXT = .txt
	MSG_COMPILE_EXT = .res
	CNV_ROOT_CMD = cnv2rootbundle

##################################
##
## ICU_NO_UPPERCASE_ROOT if set, specifies NOT to uppercase the root resource bundle,
## default is to uppercase the root resource bundle
##
##################################

ifdef ICU_NO_UPPERCASE_ROOT
  CNV_ROOT_FLAGS = 
else
  CNV_ROOT_FLAGS = -u
endif

####################################
##
##   ICU_ROOT_BUNDLE_LANG if set, specifies the language that the root resource bundle will be generated from
##   defaults to _en if not set.  if set, for any directory containing resource bundles,
##   there must exist a file name: package(the value of ICU_ROOT_BUNDLE_LANG).txt or the make messages target will fail
##
####################################

ifdef ICU_ROOT_BUNDLE_LANG
  MSG_ROOT_SOURCE = $(ICU_ROOT_BUNDLE_LANG)
else
  MSG_ROOT_SOURCE = _en
endif

    EXTRA_INCLUDES += -I${ICUROOT}/source/common
    DEFINES += -DPEGASUS_HAS_ICU
    SYS_LIBS += -licuuc
    SYS_LIBS += -licui18n
  endif
endif
