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
//%/////////////////////////////////////////////////////////////////////////////

#if defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM)
# define _OPEN_SYS_EXT
# include <sys/ps.h>
#elif defined(PEGASUS_PLATFORM_OS400_ISERIES_IBM)
# include <fcntl.h>
# include <qycmutilu2.H>
# include <unistd.cleinc>
# include "qycmmsgclsMessage.H" // ycmMessage class
# include "OS400SystemState.h"  // OS400LoadDynamicLibrary, etc
# include "EBCDIC_OS400.h"
#elif defined(PEGASUS_OS_VMS)
# include <descrip.h>           //  $DESCRIPTOR
# include <iodef.h>             // IO$_SENSEMODE
# include <ttdef.h>             // TT$M_NOBRDCST
# include <tt2def.h>            // TT2$M_PASTHRU
# include <starlet.h>
#endif

#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#if defined(PEGASUS_OS_SOLARIS)
# include <string.h>
#endif

#if !defined(PEGASUS_OS_VMS) && \
    !defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM) && \
    !defined(PEGASUS_PLATFORM_OS400_ISERIES_IBM) && \
    !defined(PEGASUS_PLATFORM_DARWIN_PPC_GNU)
# include <crypt.h>
#endif

#include "Network.h"

#if defined(PEGASUS_USE_SYSLOGS)
# include <syslog.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include "System.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Mutex.h>

#if defined(PEGASUS_OS_LSB)
# include <termios.h>
# include <stdio.h>
# include <stdlib.h>
#endif

#include "Once.h"

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// QlgPath (PEGASUS_OS_OS400 only)
//
//==============================================================================

#ifdef PEGASUS_OS_OS400

struct QlgPath
{
    QlgPath(const char* path);
    operator Qlg_Path_Name_T*() { return &qlg_struct; }
    Qlg_Path_Name_T qlg_struct;
    const char* pn;
};

QlgPath::QlgPath(const char* path) : pn(path)
{
    memset((void*)&qlg_struct, 0, sizeof(Qlg_Path_Name_T));
    qlg_struct.CCSID = 1208;
#pragma convert(37)
    memcpy(qlg_struct.Country_ID,"US",2);
    memcpy(qlg_struct.Language_ID,"ENU",3);
#pragma convert(0)
    qlg_struct.Path_Type = QLG_PTR_SINGLE;
    qlg_struct.Path_Length = strlen(path);
    qlg_struct.Path_Name_Delimiter[0] = '/';
}

#endif /* PEGASUS_OS_OS400 */

//==============================================================================
//
// System
//
//==============================================================================

// System ID constants for Logger::put and Logger::trace
#if defined(PEGASUS_PLATFORM_OS400_ISERIES_IBM)
const String System::CIMSERVER = "qycmcimom";  // Server system ID
#else
const String System::CIMSERVER = "cimserver";  // Server system ID
#endif

void System::getCurrentTime(Uint32& seconds, Uint32& milliseconds)
{
    timeval tv;
    gettimeofday(&tv, 0);
    seconds = Uint32(tv.tv_sec);
    milliseconds = Uint32(tv.tv_usec) / 1000;
}

void System::getCurrentTimeUsec(Uint32& seconds, Uint32& microseconds)
{
    timeval tv;
    gettimeofday(&tv, 0);
    seconds = Uint32(tv.tv_sec);
    microseconds = Uint32(tv.tv_usec);
}

String System::getCurrentASCIITime()
{
    char    str[50];
    time_t  rawTime;
    struct tm tmBuffer;

    time(&rawTime);
    strftime(str, 40,"%m/%d/%Y-%T", localtime_r(&rawTime, &tmBuffer));
    return String(str);
}

static inline void _sleep_wrapper(Uint32 seconds)
{
    sleep(seconds);
}

void System::sleep(Uint32 seconds)
{
    _sleep_wrapper(seconds);
}

Boolean System::exists(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgAccess(QlgPath(path), F_OK) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

Boolean System::canRead(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgAccess(QlgPath(path), R_OK) == 0;
#else
    return access(path, R_OK) == 0;
#endif
}

Boolean System::canWrite(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgAccess(QlgPath(path), W_OK) == 0;
#else
    return access(path, W_OK) == 0;
#endif
}

Boolean System::getCurrentDirectory(char* path, Uint32 size)
{
#if defined(PEGASUS_OS_OS400)
    return QlgGetcwd(QlgPath(path), size) == 0;
#else
    return getcwd(path, size) != NULL;
#endif
}

Boolean System::isDirectory(const char* path)
{
    struct stat st;

#if defined(PEGASUS_OS_OS400)
    if (QlgStat(QlgPath(path), &st) != 0)
        return false;
#else
    if (stat(path, &st) != 0)
        return false;
#endif
    return S_ISDIR(st.st_mode);
}

Boolean System::changeDirectory(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgChdir(QlgPath(path)) == 0;
#else
    return chdir(path) == 0;
#endif
}

Boolean System::makeDirectory(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgMkdir(QlgPath(path), 0777) == 0;
#else
    return mkdir(path, 0777) == 0;
#endif

}

Boolean System::getFileSize(const char* path, Uint32& size)
{
    struct stat st;

#if defined(PEGASUS_OS_OS400)
    if (QlgStat(QlgPath(path), &st) != 0)
        return false;
#else
    if (stat(path, &st) != 0)
        return false;
#endif

    size = st.st_size;
    return true;
}

Boolean System::removeDirectory(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgRmdir(QlgPath(path)) == 0;
#else
    return rmdir(path) == 0;
#endif
}

Boolean System::removeFile(const char* path)
{
#if defined(PEGASUS_OS_OS400)
    return QlgUnlink(QlgPath(path)) == 0;
#else
    return unlink(path) == 0;
#endif
}

Boolean System::renameFile(const char* oldPath, const char* newPath)
{
#if defined(PEGASUS_OS_OS400)
    if (QlgLink(QlgPath(oldPath), QlgPath(newPath)) != 0)
        return false;

    return QlgUnlink(QlgPath(oldPath)) == 0;
#elif defined(PEGASUS_OS_VMS)
//   Note: link() on OpenVMS has a different meaning so rename is used.
//         unlink() is a synonym for remove() so it can be used.
    if (rename(oldPath, newPath) != 0)
        return false;

    return true;
#else
    if (link(oldPath, newPath) != 0)
        return false;

    return unlink(oldPath) == 0;
#endif
}

String System::getHostName()
{
    static char _hostname[PEGASUS_MAXHOSTNAMELEN + 1];
    static MutexType _mutex = PEGASUS_MUTEX_INITIALIZER;

    // Use double-checked locking pattern to avoid overhead of
    // mutex on subsequenct calls.

    if (_hostname[0] == '\0')
    {
        mutex_lock(&_mutex);

        if (_hostname[0] == '\0')
        {
            gethostname(_hostname, sizeof(_hostname));
            _hostname[sizeof(_hostname)-1] = 0;
#if defined(PEGASUS_OS_OS400)
            EtoA(_hostname);
#endif
        }

        mutex_unlock(&_mutex);
    }

    return _hostname;
}

static int _getHostByName(
    const char* hostName, 
    char* hostNameOut, 
    size_t hostNameOutSize)
{
    struct hostent *hostEntry;

#if defined(PEGASUS_OS_LINUX)

    char hostEntryBuffer[8192];
    struct hostent hostEntryStruct;
    int hostEntryErrno;

    gethostbyname_r(
        hostName,
        &hostEntryStruct,
        hostEntryBuffer,
        sizeof(hostEntryBuffer),
        &hostEntry,
        &hostEntryErrno);

#elif defined(PEGASUS_OS_SOLARIS)

    char hostEntryBuffer[8192];
    struct hostent hostEntryStruct;
    int hostEntryErrno;

    hostEntry = gethostbyname_r(
        hostName,
        &hostEntryStruct,
        hostEntryBuffer,
        sizeof(hostEntryBuffer),
        &hostEntryErrno);

#else /* default */

    hostEntry = gethostbyname(hostName);

#endif

    if (hostEntry)
    {
        strncpy(hostNameOut, hostEntry->h_name, hostNameOutSize - 1);
        return 0;
    }

    return -1;
}

String System::getFullyQualifiedHostName ()
{
#if defined(PEGASUS_OS_ZOS)

    char hostName[PEGASUS_MAXHOSTNAMELEN + 1];
    String fqName;
    struct addrinfo *resolv;
    struct addrinfo hint;
    struct hostent *he;
    // receive short name of the local host
    if (gethostname(hostName, PEGASUS_MAXHOSTNAMELEN) != 0)
    {
        return String::EMPTY;
    }
    resolv = new struct addrinfo;
    hint.ai_flags = AI_CANONNAME;
    hint.ai_family = AF_UNSPEC; // any family
    hint.ai_socktype = 0;       // any socket type
    hint.ai_protocol = 0;       // any protocol
    int success = getaddrinfo(hostName,
                NULL,
                &hint,
                &resolv);
    if (success==0)
    {
        // assign fully qualified hostname
        fqName.assign(resolv->ai_canonname);
    } else
    {
        if ((he = gethostbyname(hostName)))
        {
            strcpy (hostName, he->h_name);
        }
        // assign hostName
        // if gethostbyname was successful assign that result
        // else assign unqualified hostname
    fqName.assign(hostName);
    }
    freeaddrinfo(resolv);
    delete resolv;

    return fqName;

#else /* !PEGASUS_OS_ZOS */

    char hostName[PEGASUS_MAXHOSTNAMELEN + 1];

    if (gethostname(hostName, sizeof(hostName)) != 0)
        return String::EMPTY;

    hostName[sizeof(hostName)-1] = 0;

    _getHostByName(hostName, hostName, sizeof(hostName));

# if defined(PEGASUS_OS_OS400)
    EtoA(hostName);
# endif

    return String(hostName);

#endif /* !PEGASUS_OS_ZOS */
}

String System::getSystemCreationClassName ()
{
    //
    //  The value returned should match the value of the CreationClassName key
    //  property used in the instrumentation of the CIM_ComputerSystem class
    //  as determined by the provider for the CIM_ComputerSystem class
    //
    return "CIM_ComputerSystem";
}

Uint32 System::lookupPort(
    const char * serviceName,
    Uint32 defaultPort)
{
    Uint32 localPort;

    struct servent *serv;

    //
    // Get wbem-local port from /etc/services
    //

#if defined(PEGASUS_OS_SOLARIS)
# define SERV_BUFF_SIZE 1024
    struct servent serv_result;
    char buf[SERV_BUFF_SIZE];

    if ( (serv = getservbyname_r(serviceName, TCP, &serv_result,
                                 buf, SERV_BUFF_SIZE)) != NULL )
#elif defined(PEGASUS_OS_OS400)
    struct servent serv_result;
    serv = &serv_result;
    struct servent_data buf;
    memset(&buf, 0x00, sizeof(struct servent_data));

    char srvnameEbcdic[256];
    strcpy(srvnameEbcdic, serviceName);
    AtoE(srvnameEbcdic);

    char tcpEbcdic[64];
    strcpy(tcpEbcdic, TCP);
    AtoE(tcpEbcdic);

    if ( (getservbyname_r(srvnameEbcdic, tcpEbcdic, &serv_result,
                          &buf)) == 0 )
#else // PEGASUS_OS_SOLARIS
    if ( (serv = getservbyname(serviceName, TCP)) != NULL )
#endif // PEGASUS_OS_SOLARIS
    {
        localPort = htons((uint16_t)serv->s_port);
    }
    else
    {
        localPort = defaultPort;
    }

    return localPort;
}

#if defined(PEGASUS_OS_LSB)

/*
   getpass equivalent.
   Adapted from example implementation described in GLIBC documentation
   (http://www.dusek.ch/manual/glibc/libc_32.html) and
   "Advanced Programming in the UNIX Environment" by Richard Stevens,
   pg. 350.

*/
char *getpassword(const char *prompt)
{
  const size_t MAX_PASS_LEN = 1024;
  static char buf[MAX_PASS_LEN];
  struct termios old, new_val;
  char *ptr;
  int c;

  buf[0] = 0;

  /* Turn echoing off and fail if we can't. */
  if (tcgetattr (fileno (stdin), &old) != 0)
    return buf;
  new_val = old;
  new_val.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
  if (tcsetattr (fileno (stdin), TCSAFLUSH, &new_val) != 0)
    return buf;

  /* Read the password. */
  fputs (prompt, stdin);
  ptr = buf;
  while ( (c = getc(stdin)) != EOF && c != '\n') {
    if (ptr < &buf[MAX_PASS_LEN])
      *ptr++ = c;
  }
  *ptr = 0;
  putc('\n', stdin);

  /* Restore terminal. */
  (void) tcsetattr (fileno (stdin), TCSAFLUSH, &old);
  fclose(stdin);
  return buf;
}

#endif /* PEGASUS_OS_LSB */

String System::getPassword(const char* prompt)
{
#if defined(PEGASUS_OS_VMS)

    struct
    {
        short int numbuf;
        char frst_char;
        char rsv1;
        long rsv2;
    }
    tahead;

    typedef struct
    {                           // I/O status block 
        short i_cond;           // Condition value 
        short i_xfer;           // Transfer count 
        long i_info;            // Device information 
    }
    iosb;

    typedef struct
    {                           // Terminal characteristics 
        char t_class;           // Terminal class 
        char t_type;            // Terminal type 
        short t_width;          // Terminal width in characters 
        long t_mandl;           // Terminal's mode and length 
        long t_extend;          // Extended terminal characteristics 
    }
    termb;

    termb otermb;
    termb ntermb;

    static long ichan;          // Gets channel number for TT: 

    register int errorcode;
    int kbdflgs;                // saved keyboard fd flags 
    int kbdpoll;                // in O_NDELAY mode 
    int kbdqp = false;          // there is a char in kbdq 
    int psize;                  // size of the prompt 

    const size_t MAX_PASS_LEN = 32;
    static char buf[MAX_PASS_LEN];
    char kbdq;                  // char we've already read 

    iosb iostatus;

    static long termset[2] = { 0, 0 };  // No terminator 

    $DESCRIPTOR(inpdev, "TT");  // Terminal to use for input 

    // Get a channel for the terminal

    buf[0] = 0;

    errorcode = sys$assign(&inpdev,     // Device name 
                           &ichan,      // Channel assigned 
                           0,   // request KERNEL mode access 
                           0);  // No mailbox assigned 

    if (errorcode != SS$_NORMAL)
    {
        return buf;
    }

    // Read current terminal settings

    errorcode = sys$qiow(0,     // Wait on event flag zero 
                         ichan, // Channel to input terminal 
                         IO$_SENSEMODE, // Function - Sense Mode 
                         &iostatus,     // Status after operation 
                         0, 0,  // No AST service 
                         &otermb,       // [P1] Address of Char Buffer 
                         sizeof (otermb),       // [P2] Size of Char Buffer 
                         0, 0, 0, 0);   // [P3] - [P6] 

    if (errorcode != SS$_NORMAL)
    {
        return buf;
    }

    // setup new settings 

    ntermb = otermb;

    // turn on passthru and nobroadcast 

    ntermb.t_extend |= TT2$M_PASTHRU;
    ntermb.t_mandl |= TT$M_NOBRDCST;

    // Write out new terminal settings 

    errorcode = sys$qiow(0,     // Wait on event flag zero 
                         ichan, // Channel to input terminal 
                         IO$_SETMODE,   // Function - Set Mode 
                         &iostatus,     // Status after operation 
                         0, 0,  // No AST service 
                         &ntermb,       // [P1] Address of Char Buffer 
                         sizeof (ntermb),       // [P2] Size of Char Buffer 
                         0, 0, 0, 0);   // [P3] - [P6] 

    if (errorcode != SS$_NORMAL)
    {
        return buf;
    }

    // Write a prompt, read characters from the terminal, performing no
    // editing
    // and doing no echo at all.

    psize = strlen(prompt);

    errorcode = sys$qiow(0,     // Event flag 
                         ichan, // Input channel 
                         IO$_READPROMPT | IO$M_NOECHO | IO$M_NOFILTR |
                         IO$M_TRMNOECHO,
                         // Read with prompt, no echo, no translate, no
                         // termination character echo
                         &iostatus,     // I/O status block 
                         NULL,  // AST block (none) 
                         0,     // AST parameter 
                         &buf,  // P1 - input buffer 
                         MAX_PASS_LEN,  // P2 - buffer length 
                         0,     // P3 - ignored (timeout) 
                         0,     // P4 - ignored (terminator char set) 
                         prompt,        // P5 - prompt buffer 
                         psize);        // P6 - prompt size 

    if (errorcode != SS$_NORMAL)
    {
        return buf;
    }

    // Write out old terminal settings 
    errorcode = sys$qiow(0,     // Wait on event flag zero 
                         ichan, // Channel to input terminal 
                         IO$_SETMODE,   // Function - Set Mode 
                         &iostatus,     // Status after operation 
                         0, 0,  // No AST service 
                         &otermb,       // [P1] Address of Char Buffer 
                         sizeof (otermb),       // [P2] Size of Char Buffer 
                         0, 0, 0, 0);   // [P3] - [P6] 

    if (errorcode != SS$_NORMAL)
    {
        return buf;
    }

    // Start new line
    
    const int CR = 0x0d;
    const int LF = 0x0a;
    fputc(CR, stdout);
    fputc(LF, stdout);

    // Remove the termination character
    psize = strlen(buf);
    buf[psize - 1] = 0;

    return buf;

#elif defined(PEGASUS_OS_OS400)

    // Not supported on OS/400, and we don't need it.
    // 'getpass' is DEPRECATED
    return String();

#elif defined(PEGASUS_OS_LSB)

    return String(getpassword(prompt));

#else /* default */

    return String(getpass(prompt));

#endif /* default */
}

String System::getEffectiveUserName()
{
    String userName = String::EMPTY;
    struct passwd* pwd = NULL;

#if defined(PEGASUS_OS_SOLARIS) || \
    defined(PEGASUS_OS_HPUX) || \
    defined(PEGASUS_OS_LINUX) || \
    defined(PEGASUS_OS_OS400)

    const unsigned int PWD_BUFF_SIZE = 1024;
    struct passwd       local_pwd;
    char                buf[PWD_BUFF_SIZE];

    if(getpwuid_r(geteuid(), &local_pwd, buf, PWD_BUFF_SIZE, &pwd) != 0)
    {
        String errorMsg = String("getpwuid_r failure : ") +
                            String(strerror(errno));
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
        // L10N TODO - This message needs to be added.
        //Logger::put(Logger::STANDARD_LOG, "CIMServer", Logger::WARNING,
        //                          errorMsg);
    }
#elif defined(PEGASUS_OS_ZOS)
    char effective_username[9];
    __getuserid(effective_username, 9);
    __etoa_l(effective_username,9);
    userName.assign(effective_username);
    return userName;
#else
    //
    //  get the currently logged in user's UID.
    //
    pwd = getpwuid(geteuid());
#endif
    if (pwd == NULL)
    {
         // L10N TODO - This message needs to be added.
         //Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
         //  "getpwuid_r failure, user may have been removed just after login");
         Tracer::trace (TRC_OS_ABSTRACTION, Tracer::LEVEL4,
             "getpwuid_r failure, user may have been removed just after login");
    }
    else
    {
#if defined(PEGASUS_OS_OS400)
        EtoA(pwd->pw_name);
#endif
        //
        //  get the user name
        //
        userName.assign(pwd->pw_name);
    }

    return(userName);
}

String System::encryptPassword(const char* password, const char* salt)
{
#if defined(PEGASUS_OS_VMS)

    const size_t MAX_PASS_LEN = 1024;
    char pbBuffer[MAX_PASS_LEN] = {0};
    int dwByteCount;
    char pcSalt[3] = {0};

    strncpy(pcSalt, salt, 2);
    dwByteCount = strlen(password);
    memcpy(pbBuffer, password, dwByteCount);

    for (int i=0; (i<dwByteCount) || (i>=MAX_PASS_LEN); i++)
    {
        (i%2 == 0) ? pbBuffer[i] ^= pcSalt[1] : pbBuffer[i] ^= pcSalt[0];
    }

    return String(pcSalt) + String((char *)pbBuffer);

#elif !defined(PEGASUS_OS_OS400) 

    return ( String(crypt( password,salt)) );

#else

    // Not supported on OS400, and we don't need it.
    return ( String(password) );

#endif
}

Boolean System::isSystemUser(const char* userName)
{
#if defined(PEGASUS_OS_SOLARIS) || \
    defined(PEGASUS_OS_HPUX) || \
    defined(PEGASUS_OS_LINUX) || \
    defined(PEGASUS_OS_OS400)

    const unsigned int PWD_BUFF_SIZE = 1024;
    struct passwd pwd;
    struct passwd *result;
    char pwdBuffer[PWD_BUFF_SIZE];

    if (getpwnam_r(userName, &pwd, pwdBuffer, PWD_BUFF_SIZE, &result) != 0)
    {
        String errorMsg = String("getpwnam_r failure : ") +
                            String(strerror(errno));
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
        // L10N TODO - This message needs to be added.
        //Logger::put(Logger::STANDARD_LOG, "CIMServer", Logger::WARNING,
        //                          errorMsg);
    }

    if (result == NULL)
        return false;

    return true;

#elif defined(PEGASUS_OS_OS400)

    AtoE((char*)userName);

    if (getpwnam(userName) == NULL)
    {
        EtoA((char*)userName);
        return false;
    }

    EtoA((char*)userName);
    return true;

#else /* default */

    return getpwnam(userName) != NULL;

#endif /* default */
}

Boolean System::isPrivilegedUser(const String& userName)
{
#if !defined(PEGASUS_OS_OS400)
    struct passwd   pwd;
    struct passwd   *result;
    const unsigned int PWD_BUFF_SIZE = 1024;
    char            pwdBuffer[PWD_BUFF_SIZE];

    if (getpwnam_r(
          userName.getCString(), &pwd, pwdBuffer, PWD_BUFF_SIZE, &result) != 0)
    {
        String errorMsg = String("getpwnam_r failure : ") +
                            String(strerror(errno));
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
        // L10N TODO - This message needs to be added.
        //Logger::put(Logger::STANDARD_LOG, "CIMServer", Logger::WARNING,
        //                          errorMsg);
    }

    // Check if the requested entry was found. If not return false.
    if ( result != NULL )
    {
        // Check if the uid is 0.
        if ( pwd.pw_uid == 0 )
        {
            return true;
        }
    }
    return false;

#elif defined(PEGASUS_OS_VMS)

    int retStat;

    unsigned long int prvPrv = 0;
    retStat = sys$setprv(0, 0, 0, &prvPrv);

    if (!$VMS_STATUS_SUCCESS(retStat))
        return false;

    // ATTN-VMS: should this be a bitwise and?
    return ((PRV$M_SETPRV && prvPrv) == 1) ? true : false;

#else /* default */

    CString user = userName.getCString();
    const char * tmp = (const char *)user;
    AtoE((char *)tmp);
    return ycmCheckUserCmdAuthorities(tmp);

#endif /* default */
}

static String _priviledgedUserName;
static Once _priviledgedUserNameOnce = PEGASUS_ONCE_INITIALIZER;

static void _initPrivilegedUserName()
{
    struct passwd* pwd = NULL;

#if defined(PEGASUS_OS_SOLARIS) || \
    defined(PEGASUS_OS_HPUX) || \
    defined(PEGASUS_OS_LINUX) || \
    defined(PEGASUS_OS_OS400)

    const unsigned int PWD_BUFF_SIZE = 1024;
    struct passwd local_pwd;
    char buf[PWD_BUFF_SIZE];

    if (getpwuid_r(0, &local_pwd, buf, PWD_BUFF_SIZE, &pwd) != 0)
    {
        String errorMsg = String("getpwuid_r failure : ") +
                String(strerror(errno));
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
        // L10N TODO - This message needs to be added.
        // Logger::put(Logger::STANDARD_LOG, "CIMServer", Logger::WARNING,
        //     errorMsg);
    }

#else /* default */

    pwd = getpwuid(0);

#endif /* default */

    if ( pwd != NULL )
    {
#if defined(PEGASUS_OS_OS400)
        EtoA(pwd->pw_name);
#endif
        _priviledgedUserName.assign(pwd->pw_name);
    }
    else
    {
        Tracer::trace (
            TRC_OS_ABSTRACTION, Tracer::LEVEL4, "Could not find entry.");
        PEGASUS_ASSERT(0);
    }
}

String System::getPrivilegedUserName()
{
    once(&_priviledgedUserNameOnce, _initPrivilegedUserName);
    return _priviledgedUserName;
}

#if !defined(PEGASUS_OS_VMS) || defined(PEGASUS_ENABLE_USERGROUP_AUTHORIZATION)

Boolean System::isGroupMember(const char* userName, const char* groupName)
{
    struct group grp;
    char* member;
    Boolean retVal = false;
    const unsigned int PWD_BUFF_SIZE = 1024;
    const unsigned int GRP_BUFF_SIZE = 1024;
    struct passwd pwd;
    struct passwd* result;
    struct group* grpresult;
    char pwdBuffer[PWD_BUFF_SIZE];
    char grpBuffer[GRP_BUFF_SIZE];

    // Search Primary group information.

    // Find the entry that matches "userName"

    if (getpwnam_r(userName, &pwd, pwdBuffer, PWD_BUFF_SIZE, &result) != 0)
    {
        String errorMsg = String("getpwnam_r failure : ") +
                            String(strerror(errno));
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                                  errorMsg);
        throw InternalSystemError();
    }

    if ( result != NULL )
    {
        // User found, check for group information.
        gid_t           group_id;
        group_id = pwd.pw_gid;

        // Get the group name using group_id and compare with group passed.
        if ( getgrgid_r(group_id, &grp,
                 grpBuffer, GRP_BUFF_SIZE, &grpresult) != 0)
        {
            String errorMsg = String("getgrgid_r failure : ") +
                                 String(strerror(errno));
            PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
            Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                                  errorMsg);
            throw InternalSystemError();
        }

        // Compare the user's group name to groupName.
        if ( strcmp (grp.gr_name, groupName) == 0 )
        {
             // User is a member of the group.
             return true;
        }
    }

    //
    // Search supplemental groups.
    // Get a user group entry
    //
    if (getgrnam_r((char *)groupName, &grp,
        grpBuffer, GRP_BUFF_SIZE, &grpresult) != 0)
    {
        String errorMsg = String("getgrnam_r failure : ") +
            String(strerror(errno));
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2, errorMsg);
        Logger::put(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING, errorMsg);
        throw InternalSystemError();
    }

    // Check if the requested group was found.
    if (grpresult == NULL)
    {
        return false;
    }

    Uint32 j = 0;

    //
    // Get all the members of the group
    //
    member = grp.gr_mem[j++];

    while (member)
    {
        //
        // Check if the user is a member of the group
        //
        if ( strcmp(userName, member) == 0 )
        {
            retVal = true;
            break;
        }
        member = grp.gr_mem[j++];
    }

    return retVal;
}

#endif /* !PEGASUS_OS_VMS || PEGASUS_ENABLE_USERGROUP_AUTHORIZATION */

#ifndef PEGASUS_OS_OS400

Boolean System::lookupUserId(
    const char* userName,
    PEGASUS_UID_T& uid,
    PEGASUS_GID_T& gid)
{
    const unsigned int PWD_BUFF_SIZE = 1024;
    struct passwd pwd;
    struct passwd *result;
    char pwdBuffer[PWD_BUFF_SIZE];

# if defined(PEGASUS_OS_OS400)
    AtoE((char *)userName);
# endif

    int rc = getpwnam_r(userName, &pwd, pwdBuffer, PWD_BUFF_SIZE, &result);

# if defined(PEGASUS_OS_OS400)
    EtoA((char *)userName);
# endif

    if (rc != 0)
    {
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            String("getpwnam_r failed: ") + String(strerror(errno)));
        return false;
    }

    if (result == 0)
    {
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "getpwnam_r failed.");
        return false;
    }

    uid = pwd.pw_uid;
    gid = pwd.pw_gid;

    return true;
}

Boolean System::changeUserContext(
    const PEGASUS_UID_T& uid,
    const PEGASUS_GID_T& gid)
{
    Tracer::trace(TRC_OS_ABSTRACTION, Tracer::LEVEL4,
        "Changing user context to: uid = %d, gid = %d",
        (int)uid, (int)gid);

    if (setgid(gid) != 0)
    {
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            String("setgid failed: ") + String(strerror(errno)));
        return false;
    }

    if (setuid(uid) != 0)
    {
        PEG_TRACE_STRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            String("setuid failed: ") + String(strerror(errno)));
        return false;
    }

    return true;
}

#endif /* PEGASUS_OS_OS400 */

Uint32 System::getPID()
{
    return getpid();
}

Boolean System::truncateFile(
    const char* path,
    size_t newSize)
{
#if defined(PEGASUS_OS_OS400)
    int fd = QlgOpen(QlgPath(path), O_WRONLY);

    if (fd != -1)
    {
       int rc = ftruncate(fd, newSize);
       close(fd);
       return (rc == 0);
    }

    return false;
#else
    return (truncate(path, newSize) == 0);
#endif
}

Boolean System::is_absolute_path(const char *path)
{
    if (path == NULL)
        return false;

    if (path[0] == '/')
        return true;

    return false;
}

Boolean System::changeFilePermissions(const char* path, mode_t mode)
{
    Sint32 ret = 0;
    const char * tmp = path;

#if defined(PEGASUS_OS_OS400)
    // ATTN: Update this code to handle UTF8 when path contains UTF8
    AtoE((char *)tmp);
#endif

    return chmod(tmp, mode) == 0 ? true : false;
}

Boolean System::verifyFileOwnership(const char* path)
{
    struct stat st;

#if defined(PEGASUS_OS_OS400)
    if (QlgStat(QlgPath(path), &st) != 0)
        return false;
#else
    if (lstat(path, &st) != 0)
        return false;
#endif

    return ((st.st_uid == geteuid()) &&    // Verify the file owner
            S_ISREG(st.st_mode) &&         // Verify it is a regular file
            (st.st_nlink == 1));           // Verify it is not a hard link
}

void System::syslog(const String& ident, Uint32 severity, const char* message)
{
#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX)

    // Since the openlog(), syslog(), and closelog() function calls must be
    // coordinated (see below), we need a thread control.

    static Mutex logMutex;

    AutoMutex loglock(logMutex);

    // Get a const char* representation of the identifier string.  Note: The
    // character string passed to the openlog() function must persist until
    // closelog() is called.  The syslog() method uses this pointer directly
    // rather than a copy of the string it refers to.

    CString identCString = ident.getCString();
    openlog(identCString, LOG_PID, LOG_DAEMON);

    // Map from the Logger log level to the system log level.

    Uint32 syslogLevel;
    if (severity & Logger::FATAL)
    {
        syslogLevel = LOG_CRIT;
    }
    else if (severity & Logger::SEVERE)
    {
        syslogLevel = LOG_ERR;
    }
    else if (severity & Logger::WARNING)
    {
        syslogLevel = LOG_WARNING;
    }
    else if (severity & Logger::INFORMATION)
    {
        syslogLevel = LOG_INFO;
    }
    else // if (severity & Logger::TRACE)
    {
        syslogLevel = LOG_DEBUG;
    }

    // Write the message to the system log.

    ::syslog(syslogLevel, "%s", message);

    closelog();

#elif defined(PEGASUS_OS_OS400)

    std::string replacementData = message;
    // All messages will go to the joblog. In the future
    // some messages may go to other message queues yet
    // to be determined.
    if ((severity & Logger::TRACE) ||
        (severity & Logger::INFORMATION))
    {

        // turn into ycmMessage so we can put it in the job log
# pragma convert(37)
        ycmMessage theMessage("CPIDF80",
                              message,
                              strlen(message),
                              "Logger",
                              ycmCTLCIMID,
                              TRUE);
# pragma convert(0)

        // put the message in the joblog
        theMessage.joblogIt(UnitOfWorkError,
                            ycmMessage::Informational);
    }

    if ((severity & Logger::WARNING) ||
        (severity & Logger::SEVERE)  ||
        (severity & Logger::FATAL))
    {
        // turn into ycmMessage so we can put it in the job log
# pragma convert(37)
        ycmMessage theMessage("CPDDF82",
                              message,
                              strlen(message),
                              "Logger",
                              ycmCTLCIMID,
                              TRUE);
# pragma convert(0)
        // put the message in the joblog
        theMessage.joblogIt(UnitOfWorkError,
                            ycmMessage::Diagnostic);
    }

#else /* default */

    // Not implemented!

#endif /* default */
}


///////////////////////////////////////////////////////////////////////////////
// AutoFileLock class
///////////////////////////////////////////////////////////////////////////////

AutoFileLock::AutoFileLock(const char* fileName)
{
#ifdef PEGASUS_OS_TYPE_UNIX
    _fl.l_type = F_WRLCK;
    _fl.l_whence = SEEK_SET;
    _fl.l_start = 0;
    _fl.l_len = 0;
    _fl.l_pid = getpid();

    do
    {
        _fd = open(fileName, O_WRONLY);
    } while ((_fd == -1) && (errno == EINTR));

    if (_fd != -1)
    {
        int rc;

        do
        {
            rc = fcntl(_fd, F_SETLKW, &_fl);
        } while ((rc == -1) && (errno == EINTR));

        if (rc == -1)
        {
            Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                "AutoFileLock: Failed to lock file '%s', error code %d.",
                fileName, errno);
            _fd = -1;
        }
    }
    else
    {
        Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "AutoFileLock: Failed to open lock file '%s', error code %d.",
            fileName, errno);
    }
#endif
}

AutoFileLock::~AutoFileLock()
{
#ifdef PEGASUS_OS_TYPE_UNIX
    if (_fd != -1)
    {
        _fl.l_type = F_UNLCK;
        int rc = fcntl(_fd, F_SETLK, &_fl);
        if (rc == -1)
        {
            Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                "AutoFileLock: Failed to unlock file, error code %d.",
                errno);
        }
        close(_fd);
    }
#endif
}


//==============================================================================
//
// PEGASUS_OS_AIX
//
//==============================================================================

// System Initializater for AIX
#ifdef PEGASUS_OS_AIX
# include <cstdlib>

class SystemInitializer
{

public:
    /**
     *
     * Default constructor.
     *
     */
    SystemInitializer();
};

SystemInitializer::SystemInitializer()
{
    putenv("XPG_SUS_ENV=ON");
}

static SystemInitializer initializer;

#endif /* PEGASUS_OS_AIX */

PEGASUS_NAMESPACE_END
