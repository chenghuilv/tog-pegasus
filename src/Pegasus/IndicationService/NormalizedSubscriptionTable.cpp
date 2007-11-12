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

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMInstance.h>

#include "NormalizedSubscriptionTable.h"
#include "IndicationConstants.h"

PEGASUS_NAMESPACE_BEGIN

NormalizedSubscriptionTable::NormalizedSubscriptionTable(
    const Array<CIMInstance> &subscriptions)
{
    for (Uint32 i = 0; i < subscriptions.size() ; ++i)
    {
        if (!add(subscriptions[i].getPath()))
        {
            PEG_TRACE_STRING (TRC_INDICATION_SERVICE_INTERNAL, Tracer::LEVEL2,
                "Subscription already exists : " +
                subscriptions[i].getPath().toString());
        }
    }
}

NormalizedSubscriptionTable::~NormalizedSubscriptionTable()
{
}

Boolean NormalizedSubscriptionTable:: remove(const CIMObjectPath &subPath)
{
    return _subscriptionTable.remove(normalize(subPath));
}

Boolean NormalizedSubscriptionTable::add(const CIMObjectPath &subPath,
    const Boolean &value)
{
    return _subscriptionTable.insert(normalize(subPath), value);
}

Boolean NormalizedSubscriptionTable::exists(const CIMObjectPath &subPath,
    Boolean &value)
{
    return _subscriptionTable.lookup(normalize(subPath), value);
}

CIMObjectPath NormalizedSubscriptionTable::normalize(
    const CIMObjectPath &subPath)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionRepository::normalizeSubscriptionPath");

   // Normalize the subscription object path.
    Array<CIMKeyBinding> subscriptionKB = subPath.getKeyBindings();
    CIMObjectPath filterPath;
    CIMObjectPath handlerPath;

    for (Uint32 i = 0; i < subscriptionKB.size(); i++)
    {
        if ((subscriptionKB [i].getName() == PEGASUS_PROPERTYNAME_FILTER) &&
            (subscriptionKB [i].getType() == CIMKeyBinding::REFERENCE))
        {
            filterPath = subscriptionKB[i].getValue();
        }
        if ((subscriptionKB [i].getName() == PEGASUS_PROPERTYNAME_HANDLER) &&
            (subscriptionKB [i].getType() == CIMKeyBinding::REFERENCE))
        {
            handlerPath = subscriptionKB[i].getValue();
        }
    }
    // Remove Host Tag
    filterPath.setHost(String::EMPTY);
    handlerPath.setHost(String::EMPTY);

    // Remove Namespaces if Subscription namespace, Filter and Handler
    // namespaces are same.
    CIMNamespaceName subNamespace = subPath.getNameSpace();
    if (filterPath.getNameSpace() == subNamespace)
    {
        filterPath.setNameSpace (CIMNamespaceName());
    }
    if (handlerPath.getNameSpace() == subNamespace)
    {
        handlerPath.setNameSpace (CIMNamespaceName());
    }

    Array<CIMKeyBinding> kb;
    kb.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_FILTER, CIMValue(filterPath)));
    kb.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_HANDLER, CIMValue(handlerPath)));

    PEG_METHOD_EXIT();
    return CIMObjectPath("", subNamespace, subPath.getClassName(), kb);
}

PEGASUS_NAMESPACE_END
