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


///////////////////////////////////////////////////////////////////////////////
//  Interop Provider - This provider services those classes from the
//  DMTF Interop schema in an implementation compliant with the SMI-S v1.1
//  Server Profile
//
//  Please see PG_ServerProfile20.mof in the directory
//  $(PEGASUS_ROOT)/Schemas/Pegasus/InterOp/VER20 for retails regarding the
//  classes supported by this control provider.
//
//  Interop forces all creates to the PEGASUS_NAMESPACENAME_INTEROP
//  namespace. There is a test on each operation that returns
//  the Invalid Class CIMDError
//  This is a control provider and as such uses the Tracer functions
//  for data and function traces.  Since we do not expect high volume
//  use we added a number of traces to help diagnostics.
///////////////////////////////////////////////////////////////////////////////

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/*****************************************************************************
 *
 * Implementation of AssociationProvider associators method
 *
 *****************************************************************************/
void InteropProvider::associators(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::associators()");
    initProvider();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s associators. objectName= %s, assocClass= %s resultClass= %s "
            "role= %s resultRole %s, includeQualifiers= %s, "
            "includeClassOrigin= %s, PropertyList= %s",
        thisProvider,
        (const char *)objectName.toString().getCString(),
        (const char *)associationClass.getString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
        (const char *)resultRole.getCString(),
                      boolToString(includeQualifiers),
                      boolToString(includeClassOrigin),
        (const char *)propertyListToString(propertyList).getCString()));

    handler.processing();
    String originRole = role;
    String targetRole = resultRole;
    //
    // The localReferences call retrieves instances of the desired association
    // class and sets the originRole and targetRole properties if currently
    // empty.
    //
    Array<CIMInstance> refs = localReferences(context, objectName,
        associationClass, originRole, targetRole, CIMPropertyList(),
        resultClass);
    for(Uint32 i = 0, n = refs.size(); i < n; ++i)
    {
        CIMInstance & currentRef = refs[i];
        // Retrieve the "other side" of the association
        CIMObjectPath currentTarget = getRequiredValue<CIMObjectPath>(
            currentRef, targetRole);
        CIMInstance tmpInstance = localGetInstance(context, currentTarget,
            propertyList);
        tmpInstance.setPath(currentTarget);
        handler.deliver(tmpInstance);
    }
    handler.complete();

    PEG_METHOD_EXIT();
}

/*****************************************************************************
 *
 * Implementation of AssociationProvider associatorNames method
 *
 *****************************************************************************/
void InteropProvider::associatorNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::associatorNames()");
    initProvider();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s associatorNames.objectName= %s, assocClass= %s resultClass= %s "
            "role= %s resultRole = %s",
        thisProvider,
        (const char *)objectName.toString().getCString(),
        (const char *)associationClass.getString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
        (const char *)resultRole.getCString()));

    handler.processing();
        String originRole = role;
    String targetRole = resultRole;
    //
    // The localReferences call retrieves instances of the desired association
    // class and sets the originRole and targetRole properties if currently
    // empty.
    //
    Array<CIMInstance> refs = localReferences(context, objectName,
        associationClass, originRole, targetRole, CIMPropertyList(),
        resultClass);
    for(Uint32 i = 0, n = refs.size(); i < n; ++i)
    {
        CIMInstance & currentRef = refs[i];
        CIMObjectPath currentTarget = getRequiredValue<CIMObjectPath>(
            currentRef, targetRole);
        handler.deliver(currentTarget);
    }
    handler.complete();
    PEG_METHOD_EXIT();
}


/*****************************************************************************
 *
 * Implementation of AssociationProvider references method
 *
 *****************************************************************************/
void InteropProvider::references(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::references()");
    initProvider();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s references. objectName= %s, resultClass= %s role= %s "
            "includeQualifiers= %s, includeClassOrigin= %s, PropertyList= %s",
        thisProvider,
        (const char *)objectName.toString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
                      boolToString(includeQualifiers),
                      boolToString(includeClassOrigin),
        (const char *)propertyListToString(propertyList).getCString()));

    handler.processing();
    String tmpRole = role;
    String tmpTarget;
    //
    // Makes call to internal references method to get result, supplying the
    // role parameter, but obviously not setting a resultRole/target parameter.
    //
    Array<CIMInstance> refs =
        localReferences(context, objectName, resultClass, tmpRole, tmpTarget);
    for(Uint32 i = 0, n = refs.size(); i < n; ++i)
      handler.deliver((CIMObject)refs[i]);
    handler.complete();
    PEG_METHOD_EXIT();
}

/*****************************************************************************
 *
 * Implementation of AssociationProvider referenceNames method
 *
 *****************************************************************************/
void InteropProvider::referenceNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::referenceNames()");
    initProvider();
    handler.processing();

    String tmpRole = role;
    String tmpTarget;
    //
    // Makes call to internal references method to get result, supplying the
    // role parameter, but obviously not setting a resultRole/target parameter.
    //
    Array<CIMInstance> refs =
        localReferences(context, objectName, resultClass, tmpRole, tmpTarget);
    for(Uint32 i = 0, n = refs.size(); i < n; ++i)
    {
        handler.deliver(refs[i].getPath());
    }

    handler.complete();

    PEG_METHOD_EXIT();
}


PEGASUS_NAMESPACE_END
// END_OF_FILE
