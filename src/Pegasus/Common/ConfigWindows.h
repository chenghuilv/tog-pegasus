//BEGIN_LICENSE
//
// Copyright (c) 2000 The Open Group, BMC Software, Tivoli Systems, IBM
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//END_LICENSE
//BEGIN_HISTORY
//
// Author:
//
// $Log: ConfigWindows.h,v $
// Revision 1.7  2001/04/11 00:39:18  mike
// More porting
//
// Revision 1.6  2001/03/04 21:59:39  bob
// Added PEGASUS_CMDLINE_LINKAGE macro
//
// Revision 1.5  2001/02/18 02:49:00  mike
// Removed ugly workarounds for MSVC++ 5.0 (using SP3 now)
//
// Revision 1.4  2001/02/17 00:44:13  bob
// Added linkage macros for new libraries:  getoopt and Compiler
//
// Revision 1.3  2001/01/29 02:23:44  mike
// Added support for GetInstance operation
//
// Revision 1.2  2001/01/20 22:44:44  karl
// retrofit for provider interfaces
//
// Revision 1.1.1.1  2001/01/14 19:50:40  mike
// Pegasus import
//
//
//END_HISTORY

////////////////////////////////////////////////////////////////////////////////
//
// Config_iX86_win98_msvc.h
//
//	This file contains definitions for the Intel X86 running Windows98
//	using the Microsoft Visual C++ compiler. This file must
//	be included in Config.h to be used.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Config_iX86_win98_msvc_h
#define Pegasus_Config_iX86_win98_msvc_h

#define PEGASUS_MACHINE_IX86
#define PEGASUS_OS_WIN98
#define PEGASUS_COMPILER_MSVC

// ATTN: use full qualification of cout!

// namespace std { };
// using namespace std;

#define PEGASUS_NAMESPACE_BEGIN namespace Pegasus {

#define PEGASUS_NAMESPACE_END }

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_EXPORT __declspec(dllexport)

#define PEGASUS_IMPORT __declspec(dllimport)

#ifdef PEGASUS_COMMON_INTERNAL
# define PEGASUS_COMMON_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_COMMON_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_REPOSITORY_INTERNAL
# define PEGASUS_REPOSITORY_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_REPOSITORY_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_PROTOCOL_INTERNAL
# define PEGASUS_PROTOCOL_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_PROTOCOL_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_CLIENT_INTERNAL
# define PEGASUS_CLIENT_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_CLIENT_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_SERVER_INTERNAL
# define PEGASUS_SERVER_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_SERVER_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_PROVIDER_INTERNAL
# define PEGASUS_PROVIDER_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_PROVIDER_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_COMPILER_INTERNAL
# define PEGASUS_COMPILER_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_COMPILER_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_CMDLINE_INTERNAL
# define PEGASUS_CMDLINE_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_CMDLINE_LINKAGE PEGASUS_IMPORT
#endif

#ifdef PEGASUS_GETOOPT_INTERNAL
# define PEGASUS_GETOOPT_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_GETOOPT_LINKAGE PEGASUS_IMPORT
#endif

// ATTN: take this out when no longer needed!

#ifdef PEGASUS_COMM_INTERNAL
# define PEGASUS_COMM_LINKAGE PEGASUS_EXPORT
#else
# define PEGASUS_COMM_LINKAGE PEGASUS_IMPORT
#endif

#define for if (0) ; else for

typedef unsigned char Uint8;
typedef char Sint8;
typedef unsigned short Uint16;
typedef short Sint16;
typedef unsigned int Uint32;
typedef int Sint32;
typedef float Real32;
typedef double Real64;
typedef bool Boolean;
typedef unsigned __int64 Uint64;
typedef __int64 Sint64;

#pragma warning ( disable : 4251 )

#define PEGASUS_IOS_BINARY , ios::binary

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_Config_iX86_win98_msvc_h */
