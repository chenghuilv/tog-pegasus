//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 The Open group, BMC Software, Tivoli Systems, IBM
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

#ifndef Pegasus_CIMQueryProvider_h
#define Pegasus_CIMQueryProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMBaseProvider.h>

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMInstance.h>

PEGASUS_NAMESPACE_BEGIN

/**
This class defines the set of methods implemented by an query provider. A providers that derives
from this class must implement all methods. The minimal method implementation simply throw the
NotSupported exception.
*/
class PEGASUS_PROVIDER_LINKAGE CIMQueryProvider : public virtual CIMBaseProvider
{
public:
    CIMQueryProvider(void);
    virtual ~CIMQueryProvider(void);

    /**
    Instructs the provider to execute the specified query.

    @param contex contains security and locale information relevant for the lifetime
    of this operation.

    @param queryLanguage
    @param query

    @param handler asynchronusly processes the results of this operation.

    @exception NotSupported
    @exception InvalidParameter
    */
    virtual void executeQuery(
	const OperationContext & context,
	const String & queryLanguage,
	const String & query,
	ResponseHandler<CIMObject> & handler) = 0;
};

PEGASUS_NAMESPACE_END

#endif
