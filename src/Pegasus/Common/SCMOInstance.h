//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _SCMOINSTANCE_H_
#define _SCMOINSTANCE_H_


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SCMO.h>
#include <Pegasus/Common/SCMOClass.h>

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_SCMB_INSTANCE_MAGIC 0xD00D1234

class SCMOClass;

class PEGASUS_COMMON_LINKAGE SCMOInstance
{
public:

    /**
     * Creating a SCMOInstance using a SCMOClass.
     * @param baseClass A SCMOClass.
     */
    SCMOInstance(SCMOClass baseClass);

    /**
     * Copy constructor for the SCMO instance, used to implement refcounting.
     * @param theSCMOClass The instance for which to create a copy
     * @return
     */
    SCMOInstance(const SCMOInstance& theSCMOInstance )
    {
        inst.hdr = theSCMOInstance.inst.hdr;
        Ref();
    }

    /**
     * Destructor is decrementing the refcount. If refcount is zero, the
     * singele chunk memory object is deallocated.
     */
    ~SCMOInstance()
    {
        Unref();
    }

    /**
     * Builds a SCMOInstance based on this SCMOClass.
     * The method arguments determine whether qualifiers are included,
     * the class origin attributes are included,
     * and which properties are included in the new instance.
     * @param baseClass The SCMOClass of this instance.
     * @param includeQualifiers A Boolean indicating whether qualifiers in
     * the class definition (and its properties) are to be added to the
     * instance.  The TOINSTANCE flavor is ignored.
     * @param includeClassOrigin A Boolean indicating whether ClassOrigin
     * attributes are to be added to the instance.
     * @param propertyList Is an NULL terminated array of char* to property
     * names defining the properties that are included in the created instance.
     * If the propertyList is NULL, all properties are included to the instance.
     * If the propertyList is empty, no properties are added.
     *
     * Note that this function does NOT generate an error if a property name
     * is supplied that is NOT in the class;
     * it simply does not add that property to the instance.
     *
     */
    SCMOInstance(
        SCMOClass baseClass,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const char** propertyList);

    /**
     * Builds a SCMOInstance from the given SCMOClass and copies all
     * CIMInstance data into the new SCMOInstance.
     * @param baseClass The SCMOClass of this instance.
     * @param cimInstance A CIMInstace of the same class.
     */
    SCMOInstance(SCMOClass baseClass, const CIMInstance& cimInstance);

    /**
     * Converts the SCMOInstance into a CIMInstance.
     * It is a deep copy of the SCMOInstance into the CIMInstance.
     * @param cimInstance An empty CIMInstance.
     */
    void getCIMInstance(CIMInstance& cimInstance) const;

    /**
     * Makes a deep copy of the instance.
     * This creates a new copy of the instance.
     * @return A new copy of the SCMOInstance object.
     */
    SCMOInstance clone() const;

    /**
     * Returns the number of properties of the instance.
     * @param Number of properties
     */
    Uint32 getPropertyCount() const;

    /**
     * Gets the property name, type, and value addressed by a positional index.
     * The property name and value has to be copied by the caller !
     * @param pos The positional index of the property
     * @param pname Returns the property name as '\0' terminated string.
     *              Has to be copied by caller.
     *              It is set to NULL if rc != SCMO_OK.
     * @param pvalue Returns a pointer to the value of property.
     *               The value has to be copied by the caller !
     *               It returns NULL if rc != SCMO_OK.
     *               If the value is an array, the
     *               value array is stored in continuous memory.
     *               e.g. If the CIMType is CIMTYPE_UINT32:
     *               value = (void*)Uint32[0 to size-1]
     *               If it is an array of CIMTYPE_STRING, an array
     *               of char* to the string values is returned.
     *               This array has to be freed by the caller !
     * @param type Returns the CIMType of the property
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param isArray Returns if the value is an array.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param size Returns the size of the array.
     *             If it is not an array, 0 is returned.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     *
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The value is a null value.
     *             SCMO_INDEX_OUT_OF_BOUND : Given index not found
     *
     */
    SCMO_RC getPropertyAt(
        Uint32 pos,
        const char** pname,
        CIMType& type,
        const void** pvalue,
        Boolean& isArray,
        Uint32& size ) const;

    /**
     * Gets the type and value of the named property.
     * The value has to be copied by the caller !
     * @param name The property name
     * @param pvalue Returns a pointer to the value of property.
     *               The value has to be copied by the caller !
     *               It returns NULL if rc != SCMO_OK.
     *               If the value is an array, the
     *               value array is stored in continuous memory.
     *               e.g. If the CIMType is CIMTYPE_UINT32:
     *               value = (void*)Uint32[0 to size-1]
     *               If it is an array of CIMTYPE_STRING, an array
     *               of char* to the string values is returned.
     *               This array has to be freed by the caller !
     * @param type Returns the CIMType of the property
     *             It is invalid if rc == SCMO_NOT_FOUND.
     * @param isArray Returns if the value is an array.
     *             It is invalid if rc == SCMO_NOT_FOUND.
     * @param size Returns the size of the array.
     *             If it is not an array, 0 is returned.
     *             It is invalid if rc == SCMO_NOT_FOUND.
     *
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The value is a null value.
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC getProperty(
        const char* name,
        CIMType& type,
        const void** pvalue,
        Boolean& isArray,
        Uint32& size ) const;

    /**
     * Set/replace a property in the instance.
     * If the class origin is specified, it is honored at identifying
     * the property within the instance.
     * Note: Only properties which are already part of the instance/class can
     * be set/replaced.
     * @param name The name of the property to be set.
     * @param type The CIMType of the property
     * @param value A pointer to property  value.
     *         The value is copied into the instance
     *         If the value == NULL, a null value is assumed.
     *         If the value is an array, the
     *         value array must be stored in continuous memory.
     *         e.g. If the CIMType is CIMTYPE_UINT32:
     *         value = (void*)Uint32[0 to size-1]
     * @param isArray Indicate that the value is an array. Default false.
     * @param size Returns the size of the array. If not an array this
     *         this parameter is ignorer. Default 0.
     * @param origin The class originality of the property.
     *               If NULL, then it is ignorred. Default NULL.
     * @return     SCMO_OK
     *             SCMO_NOT_SAME_ORIGIN : The property name was found, but
     *                                    the origin was not the same.
     *             SCMO_NOT_FOUND : Given property name not found.
     *             SCMO_WRONG_TYPE : Named property has the wrong type.
     *             SCMO_NOT_AN_ARRAY : Named property is not an array.
     *             SCMO_IS_AN_ARRAY  : Named property is an array.
     */
    SCMO_RC setPropertyWithOrigin(
        const char* name,
        CIMType type,
        void* value,
        Boolean isArray=false,
        Uint32 size = 0,
        const char* origin = NULL);

    /**
     * Rebuild of the key bindings from the property values.
     * if no or incomplete key properties are set on the instance.
     */
    void buildKeyBindingsFromProperties();

    /**
     * Set/replace a property filter on an instance.
     * The filter is a white list of property names.
     * A property part of the list can be accessed by name or index and
     * is eligible to be returned to requester.
     * Key properties can not be filtered. They are always a part of the
     * instance. If a key property is not part of the property list,
     * it will not be filtered out.
     * @param propertyList Is an NULL terminated array of char* to
     * property names
     */
    void setPropertyFilter(const char **propertyList);

    /**
     * Gets the hash index for the named property. Filtering is ignored.
     * @param theName The property name
     * @param pos Returns the hash index.
     * @return     SCMO_OK
     *             SCMO_INVALID_PARAMETER: name was a NULL pointer.
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC getPropertyNodeIndex(const char* name, Uint32& pos) const;

    /**
     * Set/replace a property in the instance at node index.
     * Note: If node is filtered, the property is not set but the return value 
     * is still SCMO_OK.
     * @param index The node index.
     * @param type The CIMType of the property
     * @param value A pointer to property  value.
     *         The value is copied into the instance
     *         If the value is an array, the
     *         value array must be stored in continuous memory.
     *         e.g. If the CIMType is CIMTYPE_UINT32:
     *         value = (void*)Uint32[0 to size-1]
     * @param isArray Indicate that the value is an array. Default false.
     * @param size The size of the array. If not an array this
     *         this parameter is ignorer. Default 0.
     * @return     SCMO_OK
     *             SCMO_INDEX_OUT_OF_BOUND : Given index not found
     *             SCMO_WRONG_TYPE : The property at given node index
     *                               has the wrong type.
     *             SCMO_NOT_AN_ARRAY : The property at given node index
     *                                 is not an array.
     *             SCMO_IS_AN_ARRAY  : The property at given node index
     *                                 is an array.
     */
    SCMO_RC setPropertyWithNodeIndex(
        Uint32 node,
        CIMType type,
        void* value,
        Boolean isArray=false,
        Uint32 size = 0);

    /**
     * Set/replace the named key binding
     * @param name The key binding name.
     * @param type The type as CIMKeyBinding::Type.
     * @parma value The value as string.
     * @return     SCMO_OK
     *             SCMO_TYPE_MISSMATCH : Given type does not
     *                                   match to key binding type
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC setKeyBinding(
        const char* name,
        CIMKeyBinding::Type type,
        const char* pvalue);

    /**
     * Gets the key binding count.
     * @return the number of key bindings set.
     */
    Uint32 getKeyBindingCount();

    /**
     * Get the indexed key binding.
     * @parm idx The key bining index
     * @parm pname Returns the name.
     *             Has to be copied by caller.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param type Returns the type as CIMKeyBinding::Type.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @parma pvalue Returns the value as string.
     *             Has to be copied by caller.
     *             It is only valid if rc == SCMO_OK.
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The key binding is not set.
     *             SCMO_INDEX_OUT_OF_BOUND : Given index not found
     *
     */
    SCMO_RC getKeyBindingAt(
        Uint32 idx,
        const char** pname,
        CIMKeyBinding::Type& type,
        const char** pvalue) const;

    /**
     * Get the named key binding.
     * @parm name The name of the key binding.
     * @param type Returns the type as CIMKeyBinding::Type.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @parma value Returns the value as string.
     *             Has to be copied by caller.
     *             It is only valid if rc == SCMO_OK.
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The key binding is not set.
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC getKeyBinding(
        const char* name,
        CIMKeyBinding::Type& ptype,
        const char** pvalue) const;

    /**
     * Determines whether the object has been initialized.
     * @return True if the object has not been initialized, false otherwise.
     */
    Boolean isUninitialized( ) const {return (inst.base == NULL); };

    /**
     * Determies if two objects are referencing to the same instance
     * @return True if the objects are referencing to the some instance.
     */
    Boolean isSame(SCMOInstance& theInstance) const;

    /**
     * Get the host name of the instance. The caller has to make a copy !
     * @return The host name as UTF8.
     */
    const char* getHostName() const;

    /**
     * Sets the provided host name at the instance.
     * @param hostName The host name as UTF8.
     */
    void setHostName(const char* hostName);

    /**
     * Get the class name of the instance. The cabler has to make a copy !
     * @return The class name as UTF8.
     */
    const char* getClassName() const;

    /**
     * Get the name space of the instance. The caller has to make a copy !
     * @return The name space as UTF8.
     */
    const char* getNameSpace() const;

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to include qualifiers for this instance.
     */
    void includeQualifiers()
    {
        inst.hdr->flags.includeQualifiers = true;
    };

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to NOT to include (exclude) qualifiers for this instance.
     */
    void excludeQualifiers()
    {
        inst.hdr->flags.includeQualifiers = false;
    }

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to include class origins for this instance.
     */
    void includeClassOrigins()
    {
        inst.hdr->flags.includeClassOrigin = true;
    };

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to NOT to include (exclude) class origins for this instance.
     */
    void excludeClassOrigins()
    {
        inst.hdr->flags.includeClassOrigin = false;
    }

private:

    void Ref()
    {
        inst.hdr->refCount++;
        // printf("\ninst.hdr->refCount=%u\n",inst.hdr->refCount.get());
    };

    void Unref()
    {
        if (inst.hdr->refCount.decAndTestIfZero())
        {
            // printf("\ninst.hdr->refCount=%u\n",inst.hdr->refCount.get());
            // The class has also be dereferenced.
            delete inst.hdr->theClass;
            free(inst.base);
            inst.base=NULL;
        }
        else
        {
            // printf("\ninst.hdr->refCount=%u\n",inst.hdr->refCount.get());
        }

    };
    /**
     * A SCMOInstance can only be created by a SCMOClass
     */
    SCMOInstance();

    void _initSCMOInstance(
        SCMOClass* pClass,
        Boolean inclQual,
        Boolean inclOrigin);


    SCMO_RC _getPropertyAtNodeIndex(
            Uint32 pos,
            const char** pname,
            CIMType& type,
            const void** pvalue,
            Boolean& isArray,
            Uint32& size ) const;

    void _setPropertyAtNodeIndex(
        Uint32 pos,
        CIMType type,
        void* value,
        Boolean isArray,
        Uint32 size);

    void* _getSCMBUnion(
        CIMType type,
        Boolean isArray,
        Uint32 size,
        Uint64 start,
        char* base) const;

    void _setSCMBUnion(
        void* value,
        CIMType type,
        Boolean isArray,
        Uint32 size,
        Uint64 start);

    SCMO_RC _getKeyBindingAtNodeIndex(
        Uint32 pos,
        const char** pname,
        CIMKeyBinding::Type& ptype,
        const char** pvalue) const;

    Uint32 _initPropFilterWithKeys();

    void _setPropertyInPropertyFilter(Uint32 i);

    Boolean _isPropertyInFilter(Uint32 i) const;

    void _clearPropertyFilter();

    static String _printArrayValue(
        CIMType type,
        Uint32 size,
        SCMBUnion u,
        char* base);


    static String _printUnionValue(CIMType type,SCMBUnion u,char* base);


    union{
        // To access the instance main structure
        SCMBInstance_Main *hdr;
        // To access the memory management header
        SCMBMgmt_Header     *mem;
        // Generic access pointer
        char *base;
    }inst;

    friend class SCMOClass;
    friend class SCMODump;
};


PEGASUS_NAMESPACE_END


#endif
