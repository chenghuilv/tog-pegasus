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

#include "CIMResponseData.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/SCMOXmlWriter.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/SCMOClassCache.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// C++ objects interface handling

// Instance Names handling
Array<CIMObjectPath>& CIMResponseData::getInstanceNames()
{
    PEGASUS_DEBUG_ASSERT(
    (_dataType==RESP_INSTNAMES || _dataType==RESP_OBJECTPATHS));
    _resolveToCIM();
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_CIM);
    return _instanceNames;
}

// Instance handling
CIMInstance& CIMResponseData::getInstance()
{
    PEGASUS_DEBUG_ASSERT(_dataType == RESP_INSTANCE);
    _resolveToCIM();
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_CIM);
    return _instances[0];
}

// Instances handling
Array<CIMInstance>& CIMResponseData::getInstances()
{
    PEGASUS_DEBUG_ASSERT(_dataType == RESP_INSTANCES);
    _resolveToCIM();
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_CIM);
    return _instances;
}

// Objects handling
Array<CIMObject>& CIMResponseData::getObjects()
{
    PEGASUS_DEBUG_ASSERT(_dataType == RESP_OBJECTS);
    _resolveToCIM();
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_CIM);
    return _objects;
}

// SCMO representation, single instance stored as one element array
// object paths are represented as SCMOInstance
Array<SCMOInstance>& CIMResponseData::getSCMO()
{
    _resolveToSCMO();
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_SCMO);
    return _scmoInstances;
}

void CIMResponseData::setSCMO(const Array<SCMOInstance>& x)
{
    // Just assignment bears danger of us being dependent on the original array
    // content staying valid
    // _scmoInstances=x;
    for (Uint32 loop=0, max=x.size(); loop<max; loop++)
    {
        _scmoInstances.append(x[loop]);
    }
    // _scmoInstances.appendArray(x);
    _encoding |= RESP_ENC_SCMO;
}


// Binary data is just a data stream
Array<Uint8>& CIMResponseData::getBinary()
{
    // TODO: Check if the following condition might come true
    // One actually should resolve everything down to binary in here ...
    return _binaryData;
}

bool CIMResponseData::setBinary(CIMBuffer& in, bool hasLen)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::setBinary");

    if (hasLen)
    {
        if (!in.getUint8A(_binaryData))
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Failed to get binary object path data!");
            PEG_METHOD_EXIT();
            return false;
        }
    }
    else
    {
        size_t remainingDataLength = in.capacity() - in.size();
        _binaryData.append((Uint8*)in.getPtr(), remainingDataLength);
    }
    _encoding |= RESP_ENC_BINARY;
    PEG_METHOD_EXIT();
    return true;
}

bool CIMResponseData::setXml(CIMBuffer& in)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::setXml");

    if (_dataType == RESP_INSTNAMES)
    {
        Uint32 count;

        if (!in.getUint32(count))
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Failed to get XML objectpath data (number of paths)!");
            PEG_METHOD_EXIT();
            return false;
        }

        for (Uint32 i = 0; i < count; i++)
        {
            Array<Sint8> ref;
            CIMNamespaceName ns;
            String host;

            if (!in.getSint8A(ref))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML objectpath data (references)!");
                PEG_METHOD_EXIT();
                return false;
            }

            if (!in.getString(host))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data (host)!");
                PEG_METHOD_EXIT();
                return false;
            }

            if (!in.getNamespaceName(ns))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data (namespace)!");
                PEG_METHOD_EXIT();
                return false;
            }

            _referencesData.append(ref);
            _hostsData.append(host);
            _nameSpacesData.append(ns);
        }
    }
    // TODO: Code the left out types

    _encoding |= RESP_ENC_XML;
    PEG_METHOD_EXIT();
    return true;
}

// function used by OperationAggregator to aggregate response data in a
// single ResponseData object
void CIMResponseData::appendResponseData(const CIMResponseData & x)
{
    // as the Messages set the data types, this should be impossible
    PEGASUS_DEBUG_ASSERT(_dataType == x._dataType);
    _encoding |= x._encoding;

    // add all binary data
    _binaryData.appendArray(x._binaryData);

    // add all the C++ stuff
    _instanceNames.appendArray(x._instanceNames);
    _instances.appendArray(x._instances);
    _objects.appendArray(x._objects);

    // add the SCMO instances
    _scmoInstances.appendArray(x._scmoInstances);

    // add Xml encodings too
    _referencesData.appendArray(x._referencesData);
    _instanceData.appendArray(x._instanceData);
    _hostsData.appendArray(x._hostsData);
    _nameSpacesData.appendArray(x._nameSpacesData);
}

// Encoding responses into output format
void CIMResponseData::encodeBinaryResponse(CIMBuffer& out)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::encodeBinaryResponse");

    // Need to do a complete job here by transferring all contained data
    // into binary format and handing it out in the CIMBuffer
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        // Binary does NOT need a marker as it consists of C++ and SCMO
        const Array<Uint8>& data = _binaryData;
        out.putBytes(data.getData(), data.size());
    }
    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        // TODO: Set Marker for C++ data
        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                out.putObjectPathA(_instanceNames, false);
                break;
            }
            case RESP_INSTANCE:
            {
                if (0 != _instances.size())
                {
                    out.putInstance(_instances[0], false, false);
                }
                break;
            }
            case RESP_INSTANCES:
            {
                out.putInstanceA(_instances, false);
                break;
            }
            case RESP_OBJECTS:
            {
                out.putObjectA(_objects);
                break;
            }
            case RESP_OBJECTPATHS:
            {
                // TODO: Determine what to do here
                break;
            }
            default:
            {
                // TODO:
                // Argl, not nice, but not a problem yet, ignore this
            }
        }
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        // TODO: Set Marker for SCMO data

        // Call magic here to transform a SCMO object into binary format
        fprintf(stderr, "Watch wat ya do'n! SCMO to binary ? NO OOP yet.\n");
        fflush(stderr);
    }
    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        // This actually should not happen following general code logic
        PEGASUS_DEBUG_ASSERT(true);
    }

    PEG_METHOD_EXIT();
}

void CIMResponseData::encodeXmlResponse(Buffer& out)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::encodeXmlResponse");

    fprintf(
        stderr,
        "encodeXmlResponse(encoding=%X,content=%X)\n",
        _encoding,
        _dataType);
    fflush(stderr);

    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                const Array<ArraySint8>& a = _referencesData;
                for (Uint32 i = 0, n = a.size(); i < n; i++)
                {
                    out.append((char*)a[i].getData(), a[i].size() - 1);
                }
                break;
            }
            case RESP_INSTANCE:
            {
                out.append(
                    (char*)_instanceData.getData(),
                    _instanceData.size()-1);
                break;
            }
            case RESP_INSTANCES:
            {
                const Array<ArraySint8>& a = _instanceData;
                const Array<ArraySint8>& b = _referencesData;

                for (Uint32 i = 0, n = a.size(); i < n; i++)
                {
                    out << STRLIT("<VALUE.NAMEDINSTANCE>\n");
                    out.append((char*)b[i].getData(), b[i].size() - 1);
                    out.append((char*)a[i].getData(), a[i].size() - 1);
                    out << STRLIT("</VALUE.NAMEDINSTANCE>\n");
                }
                break;
            }
            case RESP_OBJECTS:
            {
                const Array<ArraySint8>& a = _instanceData;
                const Array<ArraySint8>& b = _referencesData;

                for (Uint32 i = 0, n = a.size(); i < n; i++)
                {
                    out << STRLIT("<VALUE.OBJECTWITHPATH>\n");
                    out.append((char*)b[i].getData(), b[i].size() - 1);
                    out.append((char*)a[i].getData(), a[i].size() - 1);
                    out << STRLIT("</VALUE.OBJECTWITHPATH>\n");
                }
                break;
            }
            case RESP_OBJECTPATHS:
            {
                // TODO: Check what to do in this case
                const Array<ArraySint8>& a = _instanceData;
                const Array<ArraySint8>& b = _referencesData;

                for (Uint32 i = 0, n = a.size(); i < n; i++)
                {
                    out << STRLIT("<VALUE.OBJECTWITHPATH>\n");
                    out.append((char*)b[i].getData(), b[i].size() - 1);
                    out.append((char*)a[i].getData(), a[i].size() - 1);
                    out << STRLIT("</VALUE.OBJECTWITHPATH>\n");
                }
            }
            default:
            {
                // TODO:
                // Argl, not nice, but not a problem yet, ignore this
            }
        }
    }

    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        fprintf(stderr,"Got CIM data...\n");
        fflush(stderr);
        // TODO: Set Marker for C++ data
        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                for (Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
                {
                    XmlWriter::appendInstanceNameElement(out,_instanceNames[i]);
                }
                break;
            }
            case RESP_INSTANCE:
            {
                if (_instances.size()>0)
                {
                    XmlWriter::appendInstanceElement(out, _instances[0]);
                }
                break;
            }
            case RESP_INSTANCES:
            {
                for (Uint32 i = 0, n = _instances.size(); i < n; i++)
                {
                    XmlWriter::appendValueNamedInstanceElement(
                        out, _instances[i]);
                }
                break;
            }
            case RESP_OBJECTS:
            {
                for (Uint32 i = 0; i < _objects.size(); i++)
                {
                    XmlWriter::appendValueObjectWithPathElement(
                        out,
                        _objects[i]);
                }
                break;
            }
            case RESP_OBJECTPATHS:
            {
                for (Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
                {
                    out << "<OBJECTPATH>\n";
                    XmlWriter::appendValueReferenceElement(
                        out,
                        _instanceNames[i],
                        false);
                    out << "</OBJECTPATH>\n";
                }
                break;
            }
            default:
            {
                // TODO:
                // Argl, not nice, but not a problem yet, ignore this
            }
        }
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        /*SCMODump dmp;
        dmp.dumpSCMOInstanceKeyBindings(_scmoInstances[i]);
        dmp.dumpInstanceProperties(_scmoInstances[i]);*/
        fprintf(
            stderr,"encodeXmlResponse(SCMO)=%d instances...\n",
            _scmoInstances.size());
        fflush(stderr);

        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
                {
                    SCMOXmlWriter::appendInstanceNameElement(
                        out,
                        _scmoInstances[i]);
                }
                break;
            }
            case RESP_INSTANCE:
            {
                if (_scmoInstances.size() > 0)
                {
                    SCMOXmlWriter::appendInstanceElement(out,_scmoInstances[0]);
                }
                break;
            }
            case RESP_INSTANCES:
            {
                for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
                {
                    SCMOXmlWriter::appendValueSCMOInstanceElement(
                        out,
                        _scmoInstances[i]);
                }
                break;
            }
            case RESP_OBJECTS:
            {
                for (Uint32 i = 0; i < _scmoInstances.size(); i++)
                {
                    SCMOXmlWriter::appendValueObjectWithPathElement(
                        out,
                        _scmoInstances[i]);
                }
                break;
            }
            case RESP_OBJECTPATHS:
            {
                for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
                {
                    out << "<OBJECTPATH>\n";
                    SCMOXmlWriter::appendValueReferenceElement(
                        out,
                        _scmoInstances[i],
                        false);
                    out << "</OBJECTPATH>\n";
                }
                break;
            }
            default:
            {
                // TODO:
                // Argl, not nice, but not a problem yet, ignore this
            }
        }
    }
    fprintf(
        stderr,
        "After XmlWrite()\n%s",
        out.getData());
    fflush(stderr);
}

// contrary to encodeXmlResponse this function encodes the Xml in a format
// not usable by clients
void CIMResponseData::encodeInternalXmlResponse(CIMBuffer& out)
{
    // TODO: Implement
    // Need the full switch here again
    // Should use the internal data available SCMO, C++ and InternalXML
    // to generate the InternalXML by CIMInternalEncoder and SCMOInternalEncoder
    fprintf(stderr, "Watch wat ya do'n! SCMO to InternalXml ? NO OOP yet.\n");
    fflush(stderr);
}

void CIMResponseData::_resolveToCIM()
{
    fprintf(
        stderr,
        "_resolveToCIM(encoding=%X,content=%X)\n",
        _encoding,
        _dataType);
    fflush(stderr);

    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        _resolveXmlToCIM();
    }
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinary();
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        _resolveSCMOToCIM();
    }
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_CIM);
}

void CIMResponseData::_resolveToSCMO()
{
    fprintf(
        stderr,
        "_resolveToSCMO(encoding=%X,content=%X)\n",
        _encoding,
        _dataType);
    fflush(stderr);

    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        _resolveXmlToSCMO();
    }
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinary();
    }
    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        _resolveCIMToSCMO();
    }
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_SCMO);
}

// helper functions to transform different formats into one-another
// functions work on the internal data and calling of them should be
// avoided whenever possible
void CIMResponseData::_resolveBinary()
{
    // Call magic here to resolve binary format
    fprintf(stderr, "Watch wat ya do'n! binary ? NO OOP yet.\n");
    fflush(stderr);

    switch (_dataType)
    {
        case RESP_INSTNAMES:
        {
            break;
        }
        case RESP_INSTANCE:
        {
            break;
        }
        case RESP_INSTANCES:
        {
            break;
        }
        case RESP_OBJECTS:
        {
            break;
        }
        case RESP_OBJECTPATHS:
        {
            break;
        }
        default:
        {
            // TODO:
            // Argl, not nice, but not a problem yet, ignore this
        }
    }
}

void CIMResponseData::_resolveXmlToCIM()
{
    switch (_dataType)
    {
        case RESP_INSTNAMES:
        {
            for (Uint32 i = 0; i < _referencesData.size(); i++)
            {
                CIMObjectPath cop;
                // Deserialize path:
                {
                    XmlParser parser((char*)_referencesData[i].getData());

                    if (XmlReader::getInstanceNameElement(parser, cop))
                    {
                        if (!_nameSpacesData[i].isNull())
                            cop.setNameSpace(_nameSpacesData[i]);

                        if (_hostsData[i].size())
                            cop.setHost(_hostsData[i]);
                    }
                }
                _instanceNames.append(cop);
            }
            break;
        }
        case RESP_INSTANCE:
        {
            CIMInstance cimInstance;
            // Deserialize instance:
            {
                XmlParser parser((char*)_instanceData[0].getData());

                if (!XmlReader::getInstanceElement(parser, cimInstance))
                {
                    cimInstance = CIMInstance();
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to resolve XML instance, parser error!");
                }
            }
            // Deserialize path:
            {
                XmlParser parser((char*)_referencesData[0].getData());
                CIMObjectPath cimObjectPath;

                if (XmlReader::getValueReferenceElement(parser, cimObjectPath))
                {
                    if (_hostsData.size())
                    {
                        cimObjectPath.setHost(_hostsData[0]);
                    }
                    if (!_nameSpacesData[0].isNull())
                    {
                        cimObjectPath.setNameSpace(_nameSpacesData[0]);
                    }
                    cimInstance.setPath(cimObjectPath);
                    // only if everything works we add the CIMInstance to the
                    // array
                    _instances.append(cimInstance);
                }
            }
            break;
        }
        case RESP_INSTANCES:
        {
            for (Uint32 i = 0; i < _instanceData.size(); i++)
            {
                CIMInstance cimInstance;
                // Deserialize instance:
                {
                    XmlParser parser((char*)_instanceData[i].getData());

                    if (!XmlReader::getInstanceElement(parser, cimInstance))
                    {
                        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                            "Failed to resolve XML instance."
                                " Creating empty instance!");
                        cimInstance = CIMInstance();
                    }
                }

                // Deserialize path:
                {
                    XmlParser parser((char*)_referencesData[i].getData());
                    CIMObjectPath cimObjectPath;

                    if (XmlReader::getInstanceNameElement(parser,cimObjectPath))
                    {
                        if (!_nameSpacesData[i].isNull())
                            cimObjectPath.setNameSpace(_nameSpacesData[i]);

                        if (_hostsData[i].size())
                            cimObjectPath.setHost(_hostsData[i]);

                        cimInstance.setPath(cimObjectPath);
                    }
                }

                _instances.append(cimInstance);
            }
            break;
        }
        case RESP_OBJECTS:
        {
            for (Uint32 i=0, n=_instanceData.size(); i<n; i++)
            {
                CIMObject cimObject;

                // Deserialize Objects:
                {
                    XmlParser parser((char*)_instanceData[i].getData());

                    CIMInstance cimInstance;
                    CIMClass cimClass;

                    if (XmlReader::getInstanceElement(parser, cimInstance))
                    {
                        cimObject = CIMObject(cimInstance);
                    }
                    else if (XmlReader::getClassElement(parser, cimClass))
                    {
                        cimObject = CIMObject(cimClass);
                    }
                    else
                    {
                        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                            "Failed to get XML object data!");
                    }
                }

                // Deserialize paths:
                {
                    XmlParser parser((char*)_referencesData[i].getData());
                    CIMObjectPath cimObjectPath;

                    if (XmlReader::getValueReferenceElement(
                            parser,
                            cimObjectPath))
                    {
                        if (!_nameSpacesData[i].isNull())
                            cimObjectPath.setNameSpace(_nameSpacesData[i]);

                        if (_hostsData[i].size())
                            cimObjectPath.setHost(_hostsData[i]);

                        cimObject.setPath(cimObjectPath);
                    }
                }
                _objects.append(cimObject);
            }
            break;
        }
        case RESP_OBJECTPATHS:
        {
            // TODO: ????
        }
        default:
        {
            // TODO:
            // Argl, not nice, but not a problem yet, ignore this
        }
    }
    // Xml was resolved, release Xml content now
    _referencesData.clear();
    _hostsData.clear();
    _nameSpacesData.clear();
    _instanceData.clear();
    // remove Xml Encoding flag
    _encoding &=(!RESP_ENC_XML);
    // add CIM Encoding flag
    _encoding |=RESP_ENC_CIM;
}

void CIMResponseData::_resolveXmlToSCMO()
{
    // Not optimal, can probably be improved
    // but on the other hand, since using the binary format this case should
    // actually not ever happen.
    _resolveXmlToCIM();
    _resolveCIMToSCMO();
}

void CIMResponseData::_resolveSCMOToCIM()
{
    switch(_dataType)
    {
        case RESP_INSTNAMES:
        case RESP_OBJECTPATHS:
        {
            for (Uint32 x=0, n=_scmoInstances.size(); x < n; x++)
            {
                CIMObjectPath newObjectPath;
                _scmoInstances[x].getCIMObjectPath(newObjectPath);
                _instanceNames.append(newObjectPath);
            }
            break;
        }
        case RESP_INSTANCE:
        {
            CIMInstance newInstance;
            _scmoInstances[0].getCIMInstance(newInstance);
            _instances.append(newInstance);
            break;
        }
        case RESP_INSTANCES:
        {
            for (Uint32 x=0, n=_scmoInstances.size(); x < n; x++)
            {
                CIMInstance newInstance;
                _scmoInstances[x].getCIMInstance(newInstance);
                _instances.append(newInstance);
            }
            break;
        }
        case RESP_OBJECTS:
        {
            for (Uint32 x=0, n=_scmoInstances.size(); x < n; x++)
            {
                CIMInstance newInstance;
                _scmoInstances[x].getCIMInstance(newInstance);
                _objects.append(CIMObject(newInstance));
            }
            break;
        }
        default:
        {
            // TODO:
            // Argl, not nice, but not a problem yet, ignore this
        }
    }

    _scmoInstances.clear();
    // remove CIM Encoding flag
    _encoding &=(!RESP_ENC_SCMO);
    // add SCMO Encoding flag
    _encoding |=RESP_ENC_CIM;
}

void CIMResponseData::_resolveCIMToSCMO()
{
    switch (_dataType)
    {
        case RESP_INSTNAMES:
        case RESP_OBJECTPATHS:
        {
            for (Uint32 i=0,n=_instanceNames.size();i<n;i++)
            {
                SCMOInstance addme =
                    _getSCMOFromCIMObjectPath(_instanceNames[i]);
                _scmoInstances.append(addme);
            }
            break;
        }
        case RESP_INSTANCE:
        {
            SCMOInstance addme =
                _getSCMOFromCIMInstance(_instances[0]);
            _scmoInstances.append(addme);
            break;
        }
        case RESP_INSTANCES:
        {
            for (Uint32 i=0,n=_instances.size();i<n;i++)
            {
                SCMOInstance addme =
                    _getSCMOFromCIMInstance(_instances[i]);
                _scmoInstances.append(addme);
            }
            break;
        }
        case RESP_OBJECTS:
        {
            // TODO: Implement, but how ???
            break;
        }
        default:
        {
            // TODO:
            // Argl, not nice, but not a problem yet, ignore this
            break;
        }
    }

    // remove CIM Encoding flag
    _encoding &=(!RESP_ENC_CIM);
    // add SCMO Encoding flag
    _encoding |=RESP_ENC_SCMO;
}


// Function to convert a CIMInstance into an SCMOInstance
SCMOInstance CIMResponseData::_getSCMOFromCIMInstance(
    const CIMInstance& cimInst)
{
    const CIMObjectPath& cimPath = cimInst.getPath();

    const CString nameSpace = cimPath.getNameSpace().getString().getCString();
    const CString className = cimPath.getClassName().getString().getCString();
    // TODO: What do when either or both are 0 ?

    SCMOClass * scmoClass = _getSCMOClass(
        (const char*)nameSpace,
        (const char*)className);
    // TODO: What do when there is no such class ?

    // TODO: Interrogate Thilo about need to call new
    SCMOInstance scmoInst = SCMOInstance(*scmoClass, cimInst);

    return scmoInst;
}

// Function to convert a CIMObjectPath into an SCMOInstance
SCMOInstance CIMResponseData::_getSCMOFromCIMObjectPath(
    const CIMObjectPath& cimPath)
{
    CString nameSpace = cimPath.getNameSpace().getString().getCString();
    CString className = cimPath.getClassName().getString().getCString();

    // TODO: What do when either or both are 0 ?

    SCMOClass * scmoClass = _getSCMOClass(
        (const char*)nameSpace,
        (const char*)className);

    // TODO: What do when there is no such class ?

    // TODO: Interrogate Thilo about need to call new
    SCMOInstance scmoRef = SCMOInstance(*scmoClass, cimPath);

    return scmoRef;
}

SCMOClass* CIMResponseData::_getSCMOClass(
    const char* nameSpace,
    const char* cls)
{
    SCMOClassCache* local = SCMOClassCache::getInstance();
    return local->getSCMOClass(
        nameSpace,
        strlen(nameSpace),
        cls,
        strlen(cls));
}

PEGASUS_NAMESPACE_END
