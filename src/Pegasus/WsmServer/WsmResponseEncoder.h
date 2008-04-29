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

#ifndef Pegasus_WsmResponseEncoder_h
#define Pegasus_WsmResponseEncoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/WsmServer/WsmResponse.h>

PEGASUS_NAMESPACE_BEGIN

/** This class encodes WS-Man operation requests and passes them up-stream.
 */
class WsmResponseEncoder
{
public:

    WsmResponseEncoder();
    ~WsmResponseEncoder();

    void sendResponse(
        WsmResponse* response,
        const String& action = String::EMPTY,
        Buffer* bodygiven = 0,
        Buffer* extraHeaders = 0);

    void enqueue(WsmResponse* response);

private:

    void _encodeGetResponse(WsmGetResponse* response);
    void _encodePutResponse(WsmPutResponse* response);
    void _encodeCreateResponse(WsmCreateResponse* response);
    void _encodeDeleteResponse(WsmDeleteResponse* response);
    void _encodeWsmFaultResponse(WsmFaultResponse* response);
    void _encodeSoapFaultResponse(SoapFaultResponse* response);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmResponseEncoder_h */
