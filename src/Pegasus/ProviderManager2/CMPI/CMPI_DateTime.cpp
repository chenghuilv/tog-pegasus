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

#include "CMPI_Version.h"

#include "CMPI_DateTime.h"
#include "CMPI_Ftabs.h"

#include <time.h>
#if !defined(PEGASUS_OS_TYPE_WINDOWS)
#include <sys/time.h>
#endif
#include <string.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{
    // Taken from CIMDateTime.cpp.  This, when added to the POSIX 1970
    // microseconds epoc, produces a 1 BCE epoch as used by CIMDateTime.
    static const Uint64 POSIX_1970_EPOCH_OFFSET =
        PEGASUS_UINT64_LITERAL(62167219200000000);

    static CMPIStatus dtRelease(CMPIDateTime* eDt)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_DateTime:dtRelease()");
        CIMDateTime* dt = (CIMDateTime*)eDt->hdl;
        if (dt)
        {
            delete dt;
            (reinterpret_cast<CMPI_Object*>(eDt))->unlinkAndDelete();
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_OK);
        }
        else
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
    }

    CMPIDateTime *newDateTime()
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_DateTime:newDateTime()");
        CIMDateTime *dt = new CIMDateTime();
        *dt = CIMDateTime::getCurrentDateTime();
        CMPIDateTime* cmpiDateTime = 
            reinterpret_cast<CMPIDateTime*>(new CMPI_Object(dt));
        PEG_METHOD_EXIT();
        return cmpiDateTime;
    }

    CMPIDateTime *newDateTimeBin(CMPIUint64 tim, CMPIBoolean interval)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_DateTime:newDateTimeBin()");
        if (!interval)
        {
            tim += POSIX_1970_EPOCH_OFFSET;
        }
        CIMDateTime *dt = new CIMDateTime(tim, interval);
        CMPIDateTime* cmpiDateTime = 
            reinterpret_cast<CMPIDateTime*>(new CMPI_Object(dt));
        PEG_METHOD_EXIT();
        return cmpiDateTime;
    }

    CMPIDateTime *newDateTimeChar(const char *strTime)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_DateTime:newDateTimeChar()");
        CIMDateTime *dt = new CIMDateTime();
        try
        {
            *dt = String(strTime);
        }
        catch ( ... )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL2,
                "Exception: Unknown Exception in newDateTimeChar()");
            delete dt;
            PEG_METHOD_EXIT();
            return NULL;
        }
        CMPIDateTime* cmpiDateTime = 
            reinterpret_cast<CMPIDateTime*>(new CMPI_Object(dt));
        PEG_METHOD_EXIT();
        return cmpiDateTime;
    }

    static CMPIDateTime* dtClone(const CMPIDateTime* eDt, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_DateTime:dtClone()");
        CIMDateTime* dt = (CIMDateTime*)eDt->hdl;
        if (!dt)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL2,
                "Received invalid Handle - eDt->hdl...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return NULL;
        }
        CIMDateTime* cDt = new CIMDateTime(dt->toString());
        CMPI_Object* obj = new CMPI_Object(cDt);
        obj->unlink();
        CMPIDateTime* neDt = reinterpret_cast<CMPIDateTime*>(obj);
        CMSetStatus(rc, CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return neDt;
    }

    static CMPIBoolean dtIsInterval(
        const CMPIDateTime* eDt,
        CMPIStatus* rc)
    {
        CIMDateTime* dt = (CIMDateTime*)eDt->hdl;
        if (!dt)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL2,
                "Received invalid Handle in CMPI_DateTime:dtIsInterval");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return false;
        }

        CMSetStatus(rc, CMPI_RC_OK);
        return dt->isInterval();
    }

    static CMPIString *dtGetStringFormat(
        const CMPIDateTime* eDt,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_DateTime:dtGetStringFormat()");
        CIMDateTime* dt = (CIMDateTime*)eDt->hdl;
        if (!dt)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL2,
                "Received invalid Handle eDt->hdl in \
                CMPI_DateTime:dtGetStringFormat");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return NULL;
        }
        CMPIString *str =
            reinterpret_cast<CMPIString*>(new CMPI_Object(dt->toString()));
        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return str;
    }

    static CMPIUint64 dtGetBinaryFormat(
        const CMPIDateTime* eDt,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_DateTime:dtGetBinaryFormat()");
        CIMDateTime* dt = (CIMDateTime*)eDt->hdl;
        if (!dt)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL2,
                "Received invalid Handle eDt->hdl in \
                CMPI_DateTime:dtGetBinaryFormat");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return 0;
        }
        CMPIUint64 tim = dt->toMicroSeconds();
        if (!dt->isInterval())
        {
            tim -= POSIX_1970_EPOCH_OFFSET;
        }
        PEG_METHOD_EXIT();
        return tim;
    }
}

static CMPIDateTimeFT dateTime_FT =
{
    CMPICurrentVersion,
    dtRelease,
    dtClone,
    dtGetBinaryFormat,
    dtGetStringFormat,
    dtIsInterval,
};

CMPIDateTimeFT *CMPI_DateTime_Ftab = &dateTime_FT;

PEGASUS_NAMESPACE_END





