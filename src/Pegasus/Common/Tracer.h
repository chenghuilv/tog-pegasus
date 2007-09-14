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

#ifndef Pegasus_Tracer_h
#define Pegasus_Tracer_h

#include <cstdarg>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/TraceComponents.h>
#include <Pegasus/Common/TraceFileHandler.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SharedPtr.h>

PEGASUS_NAMESPACE_BEGIN

/** Token used for tracing functions.
*/
struct TracerToken
{
    Uint32 component;
    const char* method;
};

/** Tracer implements tracing of messages to a defined file
 */
class PEGASUS_COMMON_LINKAGE Tracer
{
public:

    /** Levels of trace
        Trace messages are written to the trace file only if they are at or
        above a given trace level
        LEVEL1 - Function Entry/Exit
        LEVEL2 - Basic flow trace messages, low data detail
        LEVEL3 - Inter-function logic flow, medium data detail
        LEVEL4 - High data detail
     */
    static const Uint32 LEVEL1;
    static const Uint32 LEVEL2;
    static const Uint32 LEVEL3;
    static const Uint32 LEVEL4;

    /** Traces the given character string.
        Overloaded to include the filename
        and line number of trace origin.
        @param fileName        filename of the trace originator
        @param lineNum         line number of the trace originator
        @param traceComponent  component being traced
        @param cstring         the character string to be traced
     */
    static void traceCString(
        const char* fileName,
        const Uint32 lineNum,
        const Uint32 traceComponent,
        const char* cstring);

    /** Traces the message in the given CIMException object.  The message
        written to the trace file will include the source filename and
        line number of the CIMException originator.
        @param traceComponent  component being traced
        @param level           trace level of the trace message
        @param CIMException    the CIMException to be traced.
     */
    static void traceCIMException(
        const Uint32 traceComponent,
        const Uint32 level,
        const CIMException& cimException);

    /** Gets an HTTP request message.
        
        Given an HTTP request message, this method checks if the
        message contains a "Basic" authorization header. 
   
        If true, the username/passwd is suppressed and returned.
        Otherwise the request message is returned without any changes.

        @param requestMessage  requestMessage to be checked
 
        @return request message
 
    */
    static SharedArrayPtr<char> getHTTPRequestMessage(
        const Buffer& requestMessage);

    /** Set the trace file to the given file
        @param  traceFile  full path of the trace file
        @return 0          if the filepath is valid
                1          if the traceFile is an empty string or
                           if an error occurs while opening the file
                           in append mode
    */
    static Uint32 setTraceFile(const char* traceFile);

    /** Set the trace level to the given level
        @param  level  trace level to be set
        @return 0      if trace level is valid
                1      if trace level is invalid
    */
    static Uint32 setTraceLevel(const Uint32 level);

    /** Set components to be traced
        @param traceComponents list of components to be traced,
               components should be separated by ','
    */
    static void setTraceComponents(
       const String& traceComponents);

    /** Traces method entry.
        @param token           TracerToken
        @param fileName        filename of the trace originator
        @param lineNum         line number of the trace originator
        @param traceComponent  component being traced
        @param methodName      method being traced
     */
    static void traceEnter(
        TracerToken& token,
        const char* file,
        size_t line,
        Uint32 component,
        const char* method);

    /** Traces method exit.
        @param token    TracerToken containing component and method
        @param fileName filename of the trace originator
        @param lineNum  line number of the trace originator
     */
    static void traceExit(
        TracerToken& token,
        const char* file,
        size_t line);

    /** Validates the File Path for the trace File
        @param  filePath full path of the file
        @return 1        if the file path is valid
                0        if the file path is invalid
     */
    static Boolean isValidFileName(const char* filePath);

    /** Validates the trace components
        @param  traceComponents   comma separated list of trace components
        @return 1        if the components are valid
                0        if one or more components are invalid
     */
    static Boolean isValidComponents(const String& traceComponents);

    /** Validates the trace components
        @param  traceComponents   comma separated list of trace components
        @param  invalidComponents comma separated list of invalid components
        @return 1        if the components are valid
                0        if one or more components are invalid
     */
    static Boolean isValidComponents(
        const String& traceComponents,
        String& invalidComponents);

    /** Specify the name of the module being traced.  If non-empty, this
        value is used as an extension to the name of the trace file.
        @param  moduleName Name of the module being traced.
     */
    static void setModuleName(const String& moduleName);

    /**
    */
    static Boolean isTraceOn() { return _traceOn; }

    // Checks if trace is enabled for the given component and trace level
    // @param    traceComponent  component being traced
    // @param    level      level of the trace message
    // @return   0               if the component and level are not enabled
    //           1               if the component and level are enabled
    static Boolean isTraceEnabled(
        const Uint32 traceComponent,
        const Uint32 level);

private:

    /** A static single indicator if tracing is turned on allowing to
        determine the state of trace quickly without many instructions.
        Used to wrap the public static trace interface methods to avoid
        obsolete calls when tracing is turned off.
     */
    static Boolean _traceOn;

    static const char   _COMPONENT_SEPARATOR;
    static const Uint32 _NUM_COMPONENTS;
    static const Uint32 _STRLEN_MAX_UNSIGNED_INT;
    static const Uint32 _STRLEN_MAX_PID_TID;
    static const Boolean _SUCCESS;
    static const Boolean _FAILURE;
    AutoArrayPtr<Boolean> _traceComponentMask;
    Uint32              _traceLevelMask;
    AutoPtr<TraceFileHandler> _traceHandler;
    String              _moduleName;
    static Tracer*      _tracerInstance;

    // Message Strings for function Entry and Exit
    static const char _METHOD_ENTER_MSG[];
    static const char _METHOD_EXIT_MSG[];

    // Message Strings for Logger
    static const char _LOG_MSG[];

    // Traces the given message. Overloaded to include the file name and the
    // line number as one of the parameters.
    // @param    traceComponent  component being traced
    // @param    message         message header (file name:line number)
    // @param    *fmt            printf style format string
    // @param    argList         variable argument list
    static void _trace(
        const char* fileName,
        const Uint32 lineNum,
        const Uint32 traceComponent,
        const char* fmt,
        va_list argList);

    //  Traces the message in the given CIMException object.  The message
    //  to be written to the trace file will include the source filename and
    //  line number of the CIMException originator.
    //  @param    traceComponent  component being traced
    //  @param    CIMException    the CIMException to be traced.
    //
    static void _traceCIMException(
        const Uint32 traceComponent,
        const CIMException& cimException);

    // Called by all the trace interfaces to log message
    // consisting of a single character string to the trace file
    // @param    traceComponent  component being traced    
    // @param    cstring         the string to be traced
    static void _traceCString(
        const Uint32 traceComponent,
        const char* message,
        const char* cstring);
    
    // Called by all the trace interfaces to log message
    // with variable number of arguments to the trace file
    // @param    traceComponent  component being traced
    // @param    *fmt            printf style format string
    // @param    argList         variable argument list
    static void _trace(
        const Uint32 traceComponent,
        const char* message,
        const char* fmt,
        va_list argList);

    // Traces method enter/exit
    // @param    fileName        filename of the trace originator
    // @param    lineNum         line number of the trace originator
    // @param    traceComponent  component being traced
    // @param    method          name of the method
    static void _traceMethod(
        const char* fileName,
        const Uint32 lineNum,
        const Uint32 traceComponent,
        const char* methodEntryExit,
        const char* method);

    // Tracer constructor
    // Constructor is private to prevent construction of Tracer objects
    // Single Instance of Tracer is maintained for each process.
    Tracer();

    //   Tracer destructor
    ~Tracer();

    // Returns the Singleton instance of the Tracer
    // @return   Tracer*  Instance of Tracer
    static Tracer* _getInstance();

    friend class TraceCallFrame;
};

//==============================================================================
//
// PEGASUS_REMOVE_TRACE defines the compile time inclusion of the Trace
// interfaces. If defined the interfaces map to empty functions.
//
//==============================================================================

#ifdef PEGASUS_REMOVE_TRACE

inline void Tracer::traceCString(
    const char* fileName,
    const Uint32 lineNum,
    const Uint32 traceComponent,
    const char* cstring)
{
    // empty function
}

inline void Tracer::traceCIMException(
    const Uint32 traceComponent,
    const Uint32 level,
    const CIMException& cimException)
{
    // empty function
}

static SharedArrayPtr<char> getHTTPRequestMessage(
        const Buffer& requestMessage)
{
    //empty function
    return SharedArrayPtr<char>();
}

inline Uint32 Tracer::setTraceFile(const char* traceFile)
{
    // empty function
    return 0;
}

inline Uint32 Tracer::setTraceLevel(const Uint32 level)
{
    // empty function
    return 0;
}

inline void Tracer::setTraceComponents(const String& traceComponents)
{
    // empty function
}

#endif /* PEGASUS_REMOVE_TRACE */

//==============================================================================
//
// Tracing macros
//
//==============================================================================

// Defines a variable that bypasses inclusion of line and filename in output.
// #define PEGASUS_NO_FILE_LINE_TRACE=1 to exclude file names and line numbers
#ifdef PEGASUS_NO_FILE_LINE_TRACE
# define PEGASUS_COMMA_FILE_LINE /* empty */
# define PEGASUS_FILE_LINE_COMMA /* empty */
#else
# define PEGASUS_COMMA_FILE_LINE ,__FILE__,__LINE__
# define PEGASUS_FILE_LINE_COMMA __FILE__,__LINE__,
#endif

#ifdef PEGASUS_REMOVE_TRACE

# define PEG_METHOD_ENTER(comp,meth)
# define PEG_METHOD_EXIT()
# define PEG_TRACE_STRING(comp,level,string)
# define PEG_TRACE(VAR_ARGS)
# define PEG_TRACE_CSTRING(comp,level,chars)

#else /* PEGASUS_REMOVE_TRACE */

# define PEG_METHOD_ENTER(comp, meth) \
    TracerToken __tracerToken; \
    __tracerToken.method = 0; \
    do \
    { \
        if (Tracer::isTraceOn()) \
            Tracer::traceEnter( \
                __tracerToken PEGASUS_COMMA_FILE_LINE, comp, meth); \
    } \
    while (0)

#  define PEG_METHOD_EXIT() \
    do \
    { \
        if (Tracer::isTraceOn()) \
            Tracer::traceExit(__tracerToken PEGASUS_COMMA_FILE_LINE); \
    } \
    while (0)

// Macro for Trace String.  the do construct allows this to appear
// as a single statement.
# define PEG_TRACE_STRING(comp, level, string) \
    do \
    { \
        if (Tracer::isTraceOn()) \
        { \
            PEGASUS_ASSERT(level != Tracer::LEVEL1); \
            if (Tracer::isTraceEnabled(comp, level)) \
            { \
                Tracer::traceCString(PEGASUS_FILE_LINE_COMMA \
                                     comp, \
                                     (const char*) (string).getCString()); \
            } \
        } \
    } \
    while (0)

// Macro to trace character lists.  the do construct allows this to appear
// as a single statement.
# define PEG_TRACE_CSTRING(comp, level, chars) \
    do \
    { \
        if (Tracer::isTraceOn()) \
        { \
            PEGASUS_ASSERT(level != Tracer::LEVEL1); \
            if (Tracer::isTraceEnabled(comp, level)) \
            { \
                Tracer::traceCString(PEGASUS_FILE_LINE_COMMA comp, chars); \
            } \
        } \
    } \
    while (0)

//
// This class is constructed with the same arguments passed to PEG_TRACE().
// The constructor saves all the fixed arguments and calls va_start() on
// the varying arguments (wherein the va_list argument is the ap member of
// this class). The PEG_TRACE() macro eventually calls invoke() with the
// file and line macros in order to write the trace entry. For more details,
// see the comments below on the PEG_TRACE() macro.
//
class TraceCallFrame
{
public:

    const char* file;
    Uint32 line;
    
    TraceCallFrame(const char* file_, Uint32 line_) : file(file_), line(line_)
    {
    }
    
    PEGASUS_FORMAT(4, 5)
    inline void invoke(
        const Uint32 component,
        const Uint32 level,
        const char* format,
        ...) 
    {
        if (Tracer::isTraceEnabled(component, level))
        {
            va_list ap;
            va_start(ap, format);
            Tracer::_trace(file, line, component, format, ap);
            va_end(ap);
        }
    }

    ~TraceCallFrame()
    {
    }
};
//
// This macro is a wrapper for calling the printf-style form of the 
// Tracer::trace() function. Since macros cannot have a varying number of 
// arguments, PEG_TRACE() must be invoked with double parentheses. For
// example:
//
//     PEG_TRACE((TRC_HTTP, Tracer::LEVEL1, "Oops: %d", 999));
//
// This macro offers two advantages over the calling trace() directly.
//
//     1. It eliminates the call to trace() if isTraceOn() returns false.
//        This has proven to reduce the expense of servicing a request
//        (when tracing is off) by as much as 3%.
//
//     2. It implicitly injects the __FILE__ and __LINE__ macros, relieving
//        the caller of this burden.
//
# define PEG_TRACE(VAR_ARGS) \
    do \
    { \
        if (Tracer::isTraceOn()) \
        { \
                TraceCallFrame frame(__FILE__, __LINE__); \
                frame.invoke VAR_ARGS; \
        } \
    } \
    while (0)

#endif /* !PEGASUS_REMOVE_TRACE */

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Tracer_h */
