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
//
//%/////////////////////////////////////////////////////////////////////////////


//
//
// This header describes the cimmofParser class.
// It is a singleton, and can only be accessed via the pointer
// returned by its static Intance() method.
// //
// The instance of this
// class hold enough state information that there should be no need for
// the underlying YACC parser to be written reentrant.
//
// The YACCer (and LExer) communicate with the instance of this class
// via the ointer returned by the Instance() method.
//
// This specialization contains a reference to the containing program's
// mofComplerCmdLine object, which holds the command line arguments
// including the list of directories to search to find included mof files
//

#ifndef _CIMMOFPARSER_H_
#define _CIMMOFPARSER_H_


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Compiler/compilerCommonDefs.h>
#include <Pegasus/Compiler/Linkage.h>
#include "parser.h"
#include "mofCompilerOptions.h"
#include "cimmofConsumer.h"
#include "cimmofMessages.h"
#include "memobjs.h"

// Diagnostics that can be used to display flow.
// This is manually turned on be selecting the proper define
// below.
#define YACCTRACE(X)
//#define YACCTRACE(X) {if (yydebug) cerr << X << endl;}
// The following is used for the moment to set the trace because
// there is a bug when YYDEBUG set and yydebug needs to be connected
// to the commof compile flags.
//#define YACCTRACE(X) {cerr << X << endl;}


extern int cimmof_parse(); // the yacc parser entry point

//class cimmofRepositoryConsumer;

// This class extends class parser (see parser.h)
class PEGASUS_COMPILER_LINKAGE cimmofParser : public parser
{
    private:
        // This is meant to be a singleton, so we hide the constructor
        // and the destructor
        static cimmofParser *_instance;

        cimmofParser();

        ~cimmofParser();

        void trace(const String &head, const String &tail) const;

        //either throw us out or retry depending on user preference
        void maybeThrowParseError(const String &msg) const;
        void maybeThrowLexerError(const String &msg) const;

        //Processing to determine if class should be updated
        Boolean updateClass(
                const CIMClass& classdecl,
                cimmofMessages::MsgCode& updateMessage,
                Boolean& classExist);

        Boolean parseVersion(
                const String& version,
                int& iM,
                int& iN,
                int& iU);

        // Here are the members added by this specialization
        const mofCompilerOptions *_cmdline;

        String _includefile;  // temp storage for included file to be entered

        String _defaultNamespacePath;  // The path we'll use if none is given

        String _currentNamespacePath;  // a namespace set from a #pragma

        compilerCommonDefs::operationType _ot;

        cimmofConsumer* _consumer; // repository interface object

    public:
        // Provide a way for the singleton to be constructed, or a
        // pointer to be returned:
        static cimmofParser *Instance();

        void elog(const String &msg) const; // handle logging of errors

        void wlog(const String &msg) const; // handle logging of warnings

        //------------------------------------------------------------------
        // Methods for manipulating the members added in this specialization
        //------------------------------------------------------------------
        // compiler options.  This may be set from command line data,
        // or by an embedding application
        void setCompilerOptions(const mofCompilerOptions *co);
        const mofCompilerOptions *getCompilerOptions() const;
        // for all, or nearly all, operations, a repository object is needed
        Boolean setRepository(void);
        const cimmofConsumer *getRepository() const;
        // Whether you need a repository or not depends on the operationsType
        void setOperationType(compilerCommonDefs::operationType);
        compilerCommonDefs::operationType getOperationType() const;
        // Set a default root namespace path to pass to  the repository
        void setDefaultNamespacePath(const String &path); // default value
        void setCurrentNamespacePath(const String &path); // current override
        const String &getDefaultNamespacePath() const;
        const String &getCurrentNamespacePath() const;
        // Get the effective namespace path -- the override, if there is one.
        const String &getNamespacePath() const;

        //------------------------------------------------------------------
        // Methods that implement or override base class methods
        //------------------------------------------------------------------
        // establish an input buffer given an input file stream
        int setInputBuffer(const FILE *f, Boolean closeCurrent);
        // establish an input buffer given an existing context (YY_BUFFERSTATE)
        int setInputBuffer(void *buffstate, Boolean closeCurrent);
        // Dig into an include file given its name
        int enterInlineInclude(const String &filename);
        // Dig into an include file given an input file stream
        int enterInlineInclude(const FILE *f);
        // Handle end-of-file
        int wrapCurrentBuffer();
        // Parse an input file
        int parse();
        // Log a parser error
        void log_parse_error(char *token, const char *errmsg) const;

        //------------------------------------------------------------------
        // Handle the processing of CIM-specific constructs
        //------------------------------------------------------------------
        // This is called after a completed #pragma production is formed
        void processPragma(const String &pragmaName,
            const String &pragmaString);

        // addClass called when completed class declaration production is formed
        int addClass(CIMClass *classdecl);

        // This is called when a new class declaration heading is discovered
        CIMClass *newClassDecl(const CIMName &name, const CIMName &superclass);

        // Called when a completed instanace declaration production is formed
        int addInstance(CIMInstance *instance);

        // Called when a new qualifier declaration heading is discovered
        CIMQualifierDecl *newQualifierDecl(const String &name,
            const CIMValue *value,
            const CIMScope & scope, const CIMFlavor & flavor);

        // Called when a completed qualifier declaration production is formed
        int addQualifier(CIMQualifierDecl *qualifier);

        // Called when a new qualifier declaration heading is discovered
        CIMQualifier *newQualifier(const String &name, const CIMValue &val,
                const CIMFlavor & flav);

        // Called when a new instance declaration heading is discovered
        CIMInstance *newInstance(const CIMName &name);

        // Called when a new property is discovered
        CIMProperty *newProperty(const CIMName &name, const CIMValue &val,
                const Boolean isArray,
                const Uint32 arraySize,
                const CIMName &referencedObj = CIMName()) const;

        // Called when a property production inside a class is complete
        int applyProperty(CIMClass &c, CIMProperty &p);

        // Called when a property production inside an instance is complete
        int applyProperty(CIMInstance &instance, CIMProperty &p);

        // Called when a new method is discovered
        CIMMethod   *newMethod(const CIMName &name, const CIMType type);

        // Called when a method production inside a class is complete
        int applyMethod(CIMClass &c, CIMMethod &m);

        // Called when a method parameter is discovered
        CIMParameter *newParameter(const CIMName &name, const CIMType type,
                Boolean isArray=false, Uint32 array=0,
                const CIMName &objName=CIMName());

        // Called when a method parameter production is complete
        int applyParameter(CIMMethod &method, CIMParameter &parm);

        // Called when a qualifier value production is complete
        CIMValue *QualifierValue(const CIMName &qualifierName,
                Boolean isNull, const String &valstr);

        // Called to retrieve the value object for an existing parameter
        CIMProperty *PropertyFromInstance(CIMInstance &instance,
                const CIMName &propertyName) const;

        CIMValue *ValueFromProperty(const CIMProperty &prop) const;

        CIMValue *PropertyValueFromInstance(CIMInstance &instance,
                const CIMName &propertyName) const;

        // Called when a class alias is found
        void addClassAlias(const String &alias, const CIMClass *cd,
                Boolean isInstance);

        // Called when an instance alias is found
        Uint32 addInstanceAlias(const String &alias, const CIMInstance *cd,
                Boolean isInstance);

        // Called when an instance alias reference is found
        Uint32 getInstanceAlias(const String &alias, CIMObjectPath &ObjPath);

        // Make a clone of a property object, inserting a new value object
        CIMProperty *copyPropertyWithNewValue(const CIMProperty &p,
                const CIMValue &v) const;
};

// Exceptions

class PEGASUS_COMPILER_LINKAGE LexerError : public Exception {
 public:
  static const char MSG[];
  LexerError(const String &lexerr) : Exception(MSG + lexerr) {}
};

#endif



