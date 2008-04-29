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

#ifndef Pegasus_XmlParser_h
#define Pegasus_XmlParser_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Stack.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Buffer.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE XmlException : public Exception
{
public:

    enum Code
    {
        BAD_START_TAG = 1,
        BAD_END_TAG,
        BAD_ATTRIBUTE_NAME,
        EXPECTED_EQUAL_SIGN,
        BAD_ATTRIBUTE_VALUE,
        MINUS_MINUS_IN_COMMENT,
        UNTERMINATED_COMMENT,
        UNTERMINATED_CDATA,
        UNTERMINATED_DOCTYPE,
        MALFORMED_REFERENCE,
        EXPECTED_COMMENT_OR_CDATA,
        START_END_MISMATCH,
        UNCLOSED_TAGS,
        MULTIPLE_ROOTS,
        VALIDATION_ERROR,
        SEMANTIC_ERROR,
        UNDECLARED_NAMESPACE
    };


    XmlException(
        Code code,
        Uint32 lineNumber,
        const String& message = String());


    XmlException(
        Code code,
        Uint32 lineNumber,
        MessageLoaderParms& msgParms);


    XmlException::Code getCode() const { return _code; }

private:

    Code _code;
};

class PEGASUS_COMMON_LINKAGE XmlValidationError : public XmlException
{
public:

    XmlValidationError(Uint32 lineNumber, const String& message);
    XmlValidationError(Uint32 lineNumber, MessageLoaderParms& msgParms);
};

class PEGASUS_COMMON_LINKAGE XmlSemanticError : public XmlException
{
public:

    XmlSemanticError(Uint32 lineNumber, const String& message);
    XmlSemanticError(Uint32 lineNumber, MessageLoaderParms& msgParms);
};

struct XmlNamespace
{
    const char* localName;
    const char* extendedName;
    int type;
    Uint32 scopeLevel;
};

struct XmlAttribute
{
    int nsType;
    const char* name;
    const char* localName;
    const char* value;
};

struct PEGASUS_COMMON_LINKAGE XmlEntry
{
    enum XmlEntryType
    {
        XML_DECLARATION,
        START_TAG,
        EMPTY_TAG,
        END_TAG,
        COMMENT,
        CDATA,
        DOCTYPE,
        CONTENT
    };

    XmlEntryType type;
    const char* text;
    int nsType;            // Only applies to START_TAG, EMPTY_TAG, and END_TAG
    const char* localName; // Only applies to START_TAG, EMPTY_TAG, and END_TAG
    Array<XmlAttribute> attributes;

    void print() const;

    const XmlAttribute* findAttribute(const char* name) const;

    const XmlAttribute* findAttribute(int nsType, const char* name) const;

    Boolean getAttributeValue(const char* name, Uint32& value) const;

    Boolean getAttributeValue(const char* name, Real32& value) const;

    Boolean getAttributeValue(const char* name, const char*& value) const;

    Boolean getAttributeValue(const char* name, String& value) const;
};

inline int operator==(const XmlEntry&, const XmlEntry&)
{
    return 0;
}

class PEGASUS_COMMON_LINKAGE XmlParser
{
public:

    // Warning: this constructor modifies the text.

    XmlParser(char* text, XmlNamespace* ns = 0);

    /** Comments are returned with entry if includeComment is true else 
        XmlParser ignores comments. Default is false.
    */
    Boolean next(XmlEntry& entry, Boolean includeComment = false);

    void putBack(XmlEntry& entry);

    ~XmlParser();

    Uint32 getStackSize() const { return _stack.size(); }

    Uint32 getLine() const { return _line; }

    XmlNamespace* getNamespace(int nsType);

private:

    Boolean _getElementName(char*& p, const char*& localName);

    Boolean _getOpenElementName(
        char*& p,
        const char*& localName,
        Boolean& openCloseElement);

    void _getAttributeNameAndEqual(char*& p, const char*& localName);

    void _getComment(char*& p);

    void _getCData(char*& p);

    void _getDocType(char*& p);

    void _getElement(char*& p, XmlEntry& entry);

    int _getNamespaceType(const char* tag);

    int _getSupportedNamespaceType(const char* extendedName);

    Uint32 _line;
    char* _current;
    char _restoreChar;
    Stack<char*> _stack;
    Boolean _foundRoot;
    Stack<XmlEntry> _putBackStack;

    XmlNamespace* _supportedNamespaces;
    Stack<XmlNamespace> _nameSpaces;
    int _currentUnsupportedNSType;
};

PEGASUS_COMMON_LINKAGE void XmlAppendCString(
    Buffer& out,
    const char* str);

PEGASUS_NAMESPACE_END

#endif /* Pegasus_XmlParser_h */
