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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:  Aruran, IBM (ashanmug@in.ibm.com) for Bug# 3614
//              Vijay Eli, IBM, (vijayeli@in.ibm.com) for Bug# 3613
//              Aruran, IBM (ashanmug@in.ibm.com) for Bug# 3613
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_NormalizationPropertyOwner_h
#define Pegasus_NormalizationPropertyOwner_h

#include <Pegasus/Config/ConfigPropertyOwner.h>

#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This is NormalizationPropertyOwner class that extends ConfigPropertyOwner
    class.
*/
class PEGASUS_CONFIG_LINKAGE NormalizationPropertyOwner : public ConfigPropertyOwner
{
public:
    NormalizationPropertyOwner(void);

    /**
    Initialize the config properties.

    This method is expected to be called only once at the start of the
    CIMOM. It initializes the properties with the default values.
    */
    void initialize(void);

    /**
    Get information about the specified property.

    @param propertyName   The name of the property.
    @param propertyInfo   List to store the property info.
    @exception UnrecognizedConfigProperty  if the property is not defined.
    */
    void getPropertyInfo(const String & name, Array<String> & propertyInfo)
    const;

    /**
    Get default value of the specified property.

    @param  name         The name of the property.
    @return string containing the default value of the property specified.
    @exception UnrecognizedConfigProperty  if the property is not defined.
    */
    String getDefaultValue(const String & name)const;

    /**
    Get current value of the specified property.

    @param  name         The name of the property.
    @return string containing the currnet value of the property specified.
    @exception UnrecognizedConfigProperty  if the property is not defined.
    */
    String getCurrentValue(const String & name)const;

    /**
    Get planned value of the specified property.

    @param  name         The name of the property.
    @return string containing the planned value of the property specified.
    @exception UnrecognizedConfigProperty  if the property is not defined.
    */
    String getPlannedValue(const String & name)const;

    /**
    Init current value of the specified property to the specified value.
    This method is expected to be called only once at the start of the
    CIMOM. The property value will be initialized irrespective of whether
    the property is dynamic or not.

    @param  name         The name of the property.
    @param  value        The current value of the property.
    @exception     UnrecognizedConfigProperty  if the property is not defined.
    @exception     InvalidPropertyValue  if the property value is not valid.
    */
    void initCurrentValue(const String & name, const String & value);

    /**
    Init planned value of the specified property to the specified value.
    This method is expected to be called only once at the start of the
    CIMOM. The property value will be initialized irrespective of whether
    the property is dynamic or not.

    @param  name         The name of the property.
    @param  value        The planned value of the property.
    @exception     UnrecognizedConfigProperty  if the property is not defined.
    @exception     InvalidPropertyValue  if the property value is not valid.
    */
    void initPlannedValue(const String & name, const String & value);

    /**
    Update current value of the specified property to the specified value.
    The property value will be updated only if the property is dynamically
    updatable.

    @param  name         The name of the property.
    @param  value        The current value of the property.
    @exception     NonDynamicConfigProperty  if the property is not dynamic.
    @exception     InvalidPropertyValue  if the property value is not valid.
    @exception     UnrecognizedConfigProperty  if the property is not defined.
    */
    void updateCurrentValue(const String & name, const String & value);

    /**
    Update planned value of the specified property to the specified value.

    @param  name         The name of the property.
    @param  value        The planned value of the property.
    @exception     InvalidPropertyValue  if the property value is not valid.
    @exception     UnrecognizedConfigProperty  if the property is not defined.
    */
    void updatePlannedValue(const String & name, const String & value);

    /**
    Checks to see if the given value is valid or not.

    @param  name         The name of the property.
    @param  value        The value of the property to be validated.
    @return Boolean      True if the specified value for the property is valid.
    @exception UnrecognizedConfigProperty  if the property is not defined.
    */
    Boolean isValid(const String & name, const String & value)const;

    /**
    Checks to see if the specified property is dynamic or not.

    @param  name         The name of the property.
    @return Boolean      True if the specified property is dynamic.
    @exception UnrecognizedConfigProperty  if the property is not defined.
    */
    Boolean isDynamic(const String & name)const;

private:
    struct ConfigProperty * _lookupConfigProperty(const String & name) const;

    AutoPtr<struct ConfigProperty> _providerObjectNormalizationEnabled;
    AutoPtr<struct ConfigProperty> _providerObjectNormalizationModuleExclusions;

};

PEGASUS_NAMESPACE_END

#endif
