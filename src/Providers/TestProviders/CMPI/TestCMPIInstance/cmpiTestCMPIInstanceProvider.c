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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

#define _ClassName "TestCMPIInstance_Method"
#define _Namespace    "test/TestProvider"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/" \
    "TestCMPIInstance/tests/"

static const CMPIBroker *_broker;

/* ---------------------------------------------------------------------------*/
/*                       CMPI Helper function                        */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * make_ObjectPath (
    const CMPIBroker *broker,
    const char *ns,
    const char *className)
{
    CMPIObjectPath *objPath = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    PROV_LOG ("++++  make_ObjectPath: CMNewObjectPath");
    objPath = CMNewObjectPath (broker, ns, className, &rc);

    PROV_LOG ("++++  CMNewObjectPath : (%s)", strCMPIStatus (rc));
    CMAddKey (objPath, "ElementName", (CMPIValue *) className, CMPI_chars);

    return objPath;
}

CMPIInstance * make_Instance (const CMPIObjectPath * op)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance *ci = NULL;

    PROV_LOG ("++++ make_Instance: CMNewInstance");
    ci = CMNewInstance (_broker, op, &rc);
    PROV_LOG ("++++  CMNewInstance : (%s)", strCMPIStatus (rc));
    if (rc.rc == CMPI_RC_ERR_NOT_FOUND)
    {
        PROV_LOG (" ---- Class %s is not found in the %s namespace!",
            _ClassName, _Namespace);
        return NULL;
    }
    return ci;
}

// For Increasing the Coverage of CMPIInstance
static int _testInstance ()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance* instance = NULL;
    CMPIObjectPath* objPath = NULL;
    CMPIObjectPath* objPathFake = NULL;

    CMPIValue value;
    CMPIType type = CMPI_uint64;

    CMPIData retData;
    CMPIData returnedData1;

    CMPIString *name;

    const char* property_name = "s";
    const char* origin = "origin";

    int flag = 1;

    value.uint64 = PEGASUS_UINT64_LITERAL(5000000000);

    PROV_LOG("++++ _testInstance");

    objPath = make_ObjectPath(_broker, _Namespace, "TestCMPI_Instance");
    instance = make_Instance(objPath);
   
    /* Test cases to increase coverage in instSetPropertyWithOrigin function */

    /* Setting n64 property in class TestCMPI_Instance. So origin has been
       set to TestCMPI_Instance.*/
    rc = CMSetPropertyWithOrigin(instance,
        "n64",
        &value,
        type,
        "TestCMPI_Instance");
    if ( rc.rc == CMPI_RC_OK )
    {
        PROV_LOG("++++  SetPropertyWithOrigin for n64 property status : (%s)",
            strCMPIStatus(rc));
    }

    /* CMGetProperty is called to verify the value of the property just being
       set. */
    retData = CMGetProperty(instance, "n64", &rc);

    if (retData.type == CMPI_uint64 && 
           retData.value.uint64 == PEGASUS_UINT64_LITERAL(5000000000))
    {
         PROV_LOG("++++  CMGetProperty for n64 property status : (%s),"
             "value (%"PEGASUS_64BIT_CONVERSION_WIDTH"u)",
             strCMPIStatus(rc),
             retData.value.uint64);
    }

    /* Setting  non-member property of type CMPI_ref*/
    type= CMPI_ref;
    value.ref = objPath;

    rc = CMSetPropertyWithOrigin(instance, "objPath", &value, type, origin);
    if ( rc.rc == CMPI_RC_OK )
    {
        PROV_LOG("++++  SetPropertyWithOrigin status : (%s)",strCMPIStatus(rc));
    }

    retData = CMGetProperty(instance, "objPath", &rc);

    name = CMGetClassName(retData.value.ref, NULL);
    if (retData.type == CMPI_ref )
    {
         PROV_LOG("++++  CMGetProperty for property objPath status : (%s)"
             ", Class name(%s)",
             strCMPIStatus(rc),
             CMGetCharsPtr(name,NULL));
    }

    /* Setting non-memeber property of type other than CMPI_ref*/
    value.uint32 = 32;
    type = CMPI_uint32;
    rc = CMSetPropertyWithOrigin(instance, "n32", &value, type, NULL);
    if ( rc.rc == CMPI_RC_OK )
    {
        PROV_LOG("++++  SetPropertyWithOrigin status : (%s)",strCMPIStatus(rc));
    }

    retData = CMGetProperty(instance, "n32", &rc);

    if (retData.type == CMPI_uint32 && retData.value.uint32 == 32)
    {
         PROV_LOG("++++  CMGetProperty for n32 property status : (%s), (%d)",
             strCMPIStatus(rc),
             retData.value.uint32);
    }

    /* Testing error path for instGetProperty with NULL property name*/
    returnedData1 = CMGetProperty(instance, NULL, &rc);
    if (rc.rc == CMPI_RC_ERR_INVALID_PARAMETER )
    {
        PROV_LOG("++++ CMGetProperty with NULL property name status: (%s) ",
            strCMPIStatus(rc));
    }
    /*-----------------------------------------------------------------------*/ 

    /* Test cases to cover error paths for instSetObjectPath and 
       instSetPropertyFilter*/

    /* Testing error path by passing CMPIObjectPath with class name different 
       from the classname for CMPIInstance in CMSetObjectPath*/

    objPathFake = CMNewObjectPath(_broker,
        _Namespace,
        "TestCMPIInstance_Method",
        NULL);
    rc = CMSetObjectPath(instance, objPathFake);
    if ( rc.rc == CMPI_RC_ERR_FAILED || rc.rc == CMPI_RC_ERR_NOT_SUPPORTED)
    {
        PROV_LOG("++++  Test for CMSetObjectPath with wrong input passed");
    }

    /* Testing error path by passing NULL to instSetObjectPath*/
    rc = CMSetObjectPath(instance, NULL);
    if ( rc.rc == CMPI_RC_ERR_INVALID_PARAMETER || 
        rc.rc == CMPI_RC_ERR_NOT_SUPPORTED)
    {
        PROV_LOG("++++  Test for CMSetObjectPath with NULL input passed ");
    }

    /* Testing error paths by setting handle to CMPIInstance to NULL and using
       that in calls to instSetObjectPath and instSetPropertyFilter*/
    instance->hdl = NULL ;
    rc = CMSetObjectPath(instance, NULL);
    if ( rc.rc == CMPI_RC_ERR_INVALID_HANDLE ||
        rc.rc == CMPI_RC_ERR_NOT_SUPPORTED)
    {
        PROV_LOG("++++  CMSetObjectPath with NULL handle for"
            " CMPIInstance passed");
    }
    PROV_LOG("++++ CMSetObjectPath : (%s) ",strCMPIStatus(rc));

    rc = CMSetPropertyFilter(instance,&property_name,NULL);
    if( rc.rc == CMPI_RC_ERR_INVALID_HANDLE )
    {
        PROV_LOG("++++  CMSetPropertyFilter with NULL handle for CMPIInstance"
            " status : (%s)", strCMPIStatus (rc));
    }
    /*-----------------------------------------------------------------------*/
    return flag;

} // _testInstance end


/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
CMPIStatus TestCMPIInstanceProviderMethodCleanup (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIInstanceProviderInvokeMethod (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *methodName,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIString *class = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIData data;

    CMPIString *argName = NULL;

    CMPIInstance *instance = NULL;
    CMPIInstance *paramInst = NULL;

    unsigned int arg_cnt = 0, index = 0;

    CMPIUint32 oper_rc = 1;

    char *result = NULL;


    PROV_LOG_OPEN ("TestCMPI_Instance", _ProviderLocation);

    PROV_LOG ("--- %s CMPI InvokeMethod() called", _ClassName);

    class = CMGetClassName (ref, &rc);

    PROV_LOG ("InvokeMethod: checking for correct classname [%s]",
        CMGetCharPtr (class));

    PROV_LOG ("Calling CMGetArgCount");
    arg_cnt = CMGetArgCount (in, &rc);
    PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

    PROV_LOG ("InvokeMethod: We have %d arguments for operation [%s]: ",
        arg_cnt, 
        methodName);

    if (arg_cnt > 0)
    {
        PROV_LOG ("Calling CMGetArgAt");
        for (index = 0; index < arg_cnt; index++)
        {
            data = CMGetArgAt (in, index, &argName, &rc);
            if (data.type == CMPI_uint32)
            {
                PROV_LOG ("#%d: %s (uint32), value: %d",
                    index,
                    CMGetCharPtr (argName),
                    data.value.uint32);
            }
            else if (data.type == CMPI_string)
            {
                PROV_LOG ("#%d: %s (string) value: %s",
                    index,
                    CMGetCharPtr (argName),
                    CMGetCharPtr (data.value.string));
            }
            else
            {
                PROV_LOG ("#%d: %s (type: %x)",
                    index,
                    CMGetCharPtr (argName),
                    data.type);
            }
            CMRelease (argName);
        }
    }
    if (strncmp (CMGetCharPtr (class), _ClassName, strlen (_ClassName)) == 0)
    {
        if (strncmp("testInstance", 
            methodName,
            strlen ("testInstance"))== 0)
        {
            oper_rc = _testInstance();
            CMReturnData (rslt, (CMPIValue *) &oper_rc, CMPI_uint32);
            CMReturnDone (rslt);
        }
        else
        {
            PROV_LOG ("++++ Could not find the %s operation", methodName);
            CMSetStatusWithChars (_broker, &rc,
                CMPI_RC_ERR_NOT_FOUND, methodName);
        }
    }
    PROV_LOG ("--- %s CMPI InvokeMethod() exited", _ClassName);
    PROV_LOG_CLOSE();
    return rc;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (TestCMPIInstanceProvider,
    TestCMPIInstanceProvider,
    _broker,
    CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
