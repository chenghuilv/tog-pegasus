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
// Author: Barbara Packard (barbara_packard@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

// WMIAssociatorProvider.cpp: implementation of the WMIAssociatorProvider class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "WMICollector.h"
#include "WMIBaseProvider.h"
#include "WMIClassProvider.h"
#include "WMIInstanceProvider.h"
#include "WMIAssociatorProvider.h"

#include "WMIProperty.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"
#include "WMIType.h"
#include "WMIException.h"


//////////////////////////////////////////////////////////////////////////////
// WMIAssociatorProvider::
//
// ///////////////////////////////////////////////////////////////////////////
PEGASUS_NAMESPACE_BEGIN

/////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMIAssociatorProvider::WMIAssociatorProvider()
{
	_collector = NULL;
	m_bInitialized = false;
}

WMIAssociatorProvider::~WMIAssociatorProvider()
{
	cleanup();
}

//////////////////////////////////////////////////////////////////////////////
// WMIAssociatorProvider::associators
//
//	NOTE:  This method sets up the query string and then calls
//		WMIBaseProvider::execCIMQuery
//////////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIAssociatorProvider::associators(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMObjectPath& objectName,
        const String& assocClass,
        const String& resultClass,
        const String& role,
        const String& resultRole,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList)
{
	String sQuery;
	String sQueryLanguage;

	Array<CIMObject> objects;

	PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIAssociatorProvider::associators()");

	sQueryLanguage = qString(Q_WQL);
	sQuery = getAssocQueryString(objectName, 
				assocClass, 
				resultClass, 
				role, 
				resultRole);

	objects = execCIMQuery(nameSpace,
				userName,
				password,
				sQueryLanguage, 
				sQuery, 
				propertyList,
				includeQualifiers,
				includeClassOrigin);

	PEG_METHOD_EXIT();

	return objects;
}

//////////////////////////////////////////////////////////////////////////////
// WMIAssociatorProvider::associatorNames
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMObjectPath> WMIAssociatorProvider::associatorNames(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMObjectPath& objectName,
        const String& assocClass,
        const String& resultClass,
        const String& role,
        const String& resultRole)
{
	Array<CIMObject> objects;
	Array<CIMObjectPath> objectNames;

	PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIAssociatorProvider::associatorNames()");

	// create an empty property list to save time...
	Array<CIMName> propNames;
	CIMPropertyList propertyList(propNames);

	// now get the objects
	objects = associators(	nameSpace,
							userName,
							password,
							objectName,
							assocClass,
							resultClass,
							role,
							resultRole,
							false,
							false,
							propertyList);

	// now get the names from the object
	Uint32 size = objects.size();
	Uint32 i;

	for (i=0; i<size; i++)
	{
		CIMObjectPath objPath;

		objPath = objects[i].getPath();
		objectNames.append(objPath);
	}

	PEG_METHOD_EXIT();

	return objectNames;
}

//////////////////////////////////////////////////////////////////////////////
// WMIAssociatorProvider
//		private methods
//
// ///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// WMIAssociatorProvider::getAssocQueryString - calls the BaseProvider method 
//		to build the query string from the input parameters
//
// ///////////////////////////////////////////////////////////////////////////
String WMIAssociatorProvider::getAssocQueryString(const CIMObjectPath &objectName, 
		const String &assocClass, 
		const String &resultClass, 
		const String &role,
		const String &resultRole)
{
	String sQuery;

	sQuery = qString(Q_ASSOCIATORS);

	return getQueryString(objectName, sQuery, assocClass, resultClass, role, resultRole);

}


PEGASUS_NAMESPACE_END
