//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMInstance.h>

#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/ProviderManager2/Default/SimpleResponseHandler.h>

PEGASUS_USING_PEGASUS;

#include <iostream>

PEGASUS_USING_STD;

int main(int argc, char** argv)
{
    const char * verbose = getenv("PEGASUS_TEST_VERBOSE");

    // instantiate the primary response handler types
    {
        SimpleResponseHandler concreteHandler;
        ResponseHandler& handler = concreteHandler;

        handler.processing();
        handler.complete();
    }

    {
        SimpleObjectResponseHandler handler = SimpleObjectResponseHandler();

        handler.processing();
        handler.deliver(CIMObject());
        handler.complete();
    }

    {
        SimpleClassResponseHandler handler = SimpleClassResponseHandler();

        handler.processing();
        handler.deliver(CIMClass());
        handler.complete();
    }

    {
        SimpleInstanceResponseHandler handler = SimpleInstanceResponseHandler();

        handler.processing();
        handler.deliver(CIMInstance());
        handler.complete();
    }

    {
        SimpleObjectPathResponseHandler handler = SimpleObjectPathResponseHandler();

        handler.processing();
        handler.deliver(CIMObjectPath());
        handler.complete();
    }

    {
        SimpleMethodResultResponseHandler handler = SimpleMethodResultResponseHandler();

        handler.processing();
        handler.deliverParamValue(CIMParamValue("param1", String("p1")));
        handler.deliverParamValue(CIMParamValue("param2", String("p2")));
        handler.deliver(CIMValue());
        handler.complete();
    }

    cout << "+++++ passed all tests." << endl;

    return(0);
}
