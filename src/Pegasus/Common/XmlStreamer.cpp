//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Author: Adrian Schuur schuur@de-ibm.com
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "XmlWriter.h"
#include "XmlReader.h"
#include "XmlParser.h"

#include "XmlStreamer.h"

PEGASUS_NAMESPACE_BEGIN

void XmlStreamer::encode(Array<Sint8>& out, const CIMClass& cls) {
   XmlWriter::appendClassElement(out, cls);
}

void XmlStreamer::encode(Array<Sint8>& out, const CIMInstance& inst) {
   XmlWriter::appendInstanceElement(out, inst);
}

void XmlStreamer::encode(Array<Sint8>& out, const CIMQualifierDecl& qual) {
   XmlWriter::appendQualifierDeclElement(out, qual);
}

void XmlStreamer::decode(const Array<Sint8>& in, unsigned int pos, CIMClass& cls) {
    XmlParser parser(((char*)in.getData())+pos);
    XmlReader::getObject(parser, cls);
}

void XmlStreamer::decode(const Array<Sint8>& in, unsigned int pos, CIMInstance& inst) {
   XmlParser parser(((char*)in.getData())+pos);
   XmlReader::getObject(parser, inst);
}

void XmlStreamer::decode(const Array<Sint8>& in, unsigned int pos, CIMQualifierDecl& qual) {
   XmlParser parser(((char*)in.getData())+pos);
   XmlReader::getObject(parser, qual);
}

void indentedOut(PEGASUS_STD(ostream)& os, Array<Sint8>& in,
                            Uint32 indentChars) {
   in.append('\0');
   XmlWriter::indentedPrint(os, in.getData(), indentChars);
}

PEGASUS_NAMESPACE_END
