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
#ifdef PEGASUS_PLATFORM_ZOS_ZSERIES_IBM
#include <pthread.h>
#endif

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#else
#if defined(CMPI_PLATFORM_AIX_RS_IBMCXX) \
    || defined(CMPI_PLATFORM_PASE_ISERIES_IBMCXX)
#include <time.h>
#endif
#include <sys/time.h>
#endif

#define _IndClassName "TestCMPI_IndicationStressTestClass"

#ifdef CMPI_VER_100
static const CMPIBroker *_broker;
#else
static CMPIBroker *_broker;
#endif

#define CMPI_true  1
#define CMPI_false  0

static CMPIObjectPath *objPath;
static CMPIBoolean _enabled = CMPI_false;
static CMPIUint32 _nextUID = 1;
static CMPIUint32 _numSubscriptions = 0;
static CMPIIndicationMI *_indMI = NULL;
static CMPIArray *correlatedIndications = NULL;
static char *namespaceName;


/* ------------------------------------------------------------------------ */
/*                            Helper functions.                             */
/* ------------------------------------------------------------------------ */

CMPIInstance *getIndicationInstance(CMPIUint64 seqNum, char *UIDbuffer)
{
    CMPIInstance *indicationInstance =
                  CMNewInstance (_broker, objPath, NULL);
    CMPIDateTime *currentDateTime =
                  CMNewDateTime (_broker,NULL);
    CMSetProperty (indicationInstance, "IndicationIdentifier",
                   UIDbuffer, CMPI_chars);
    CMSetProperty (indicationInstance, "CorrelatedIndications",
                   &correlatedIndications, CMPI_stringA);
    CMSetProperty (indicationInstance, "IndicationTime",
                   &currentDateTime, CMPI_dateTime);
    CMSetProperty (indicationInstance, "IndicationSequenceNumber",
                   &seqNum, CMPI_uint64);

    return indicationInstance;
}


/* ---------------------------------------------------------------------------*/
/*                       Indication Provider Interface                        */
/* ---------------------------------------------------------------------------*/

CMPIStatus TestCMPIIndicationStressTestProviderIndicationCleanup(
                                           CMPIIndicationMI * mi,
                                           const CMPIContext * ctx,
                                           CMPIBoolean term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderAuthorizeFilter(
                                           CMPIIndicationMI * mi,
                                           const CMPIContext * ctx,
                                           const CMPISelectExp * se,
                                           const char *ns,
                                           const CMPIObjectPath * op,
                                           const char *user)
{
    if (strcmp (ns, _IndClassName) )
    {
        CMReturn (CMPI_RC_ERR_INVALID_CLASS);
    }

    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderMustPoll(
                                    CMPIIndicationMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPISelectExp * se,
                                    const char *ns, const CMPIObjectPath * op)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIIndicationStressTestProviderActivateFilter(
                                          CMPIIndicationMI * mi,
                                          const CMPIContext * ctx,
                                          const CMPISelectExp * se,
                                          const char *ns,
                                          const CMPIObjectPath * op,
                                          CMPIBoolean firstActivation)
{
    _numSubscriptions++;

    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderDeActivateFilter(
                                            CMPIIndicationMI * mi,
                                            const CMPIContext * ctx,
                                            const CMPISelectExp * se,
                                            const char *ns,
                                            const CMPIObjectPath * op,
                                            CMPIBoolean lastActivation)
{
    _numSubscriptions--;

    if (_numSubscriptions == 0)
    {
        _enabled = CMPI_false;
    }

    CMReturn (CMPI_RC_OK);
}


CMPIStatus TestCMPIIndicationStressTestProviderEnableIndications(
                                              CMPIIndicationMI * mi,
                                              const CMPIContext * ctx)
{
    _enabled = CMPI_true;

    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderDisableIndications(
                                               CMPIIndicationMI * mi,
                                               const CMPIContext * ctx)
{
    if (_numSubscriptions == 0)
    {
        _enabled = CMPI_false;
    }
    CMReturn (CMPI_RC_OK);
}



/* ---------------------------------------------------------------------------*/
/*                       Method Provider Interface                        */
/* ---------------------------------------------------------------------------*/

CMPIStatus TestCMPIIndicationStressTestProviderMethodCleanup(
                                     CMPIMethodMI * mi,
                                     const CMPIContext * ctx,
                                     CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderInvokeMethod(
                                    CMPIMethodMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref,
                                    const char *methodName,
                                    const CMPIArgs * in, CMPIArgs * out)
{
    CMPIBoolean sendIndication = CMPI_false;
    CMPIUint32 indicationSendCount = 0;
    CMPIInstance *indicationInstance = 0;
    CMPIData data;
    char UIDbuffer[32];
    int i = 0;
    CMPIUint32 seqNum;

    if (!strcmp(methodName, "getSubscriptionCount"))
    {
        CMReturnData (rslt, (CMPIValue *)&_numSubscriptions, CMPI_uint32);
        CMReturnDone(rslt);
        CMReturn (CMPI_RC_OK);
    }

    data = CMGetArgAt (in, 0, NULL, NULL);
    if (data.type == CMPI_uint32)
    {
        indicationSendCount = data.value.uint32;
    }

    // Get the Namespace to deliver the indication.
    data = CMGetArgAt (in, 1, NULL, NULL);
    if (data.type == CMPI_string)
    {
        namespaceName = strdup(CMGetCharPtr (data.value.string));
    }
    else
    {
        CMReturn (CMPI_RC_ERR_FAILED);
    }
    if (_enabled)
    {
        CMReturnData (rslt, (CMPIValue *)&i, CMPI_sint32);
        CMReturnDone (rslt);
    }

    objPath = CMNewObjectPath (_broker, namespaceName, _IndClassName, NULL);

    if (indicationSendCount > 0)
    {
        for (seqNum = 1; seqNum < indicationSendCount+1; seqNum++)
        {
             sprintf(UIDbuffer, "%d", _nextUID++);
             indicationInstance = getIndicationInstance (seqNum, UIDbuffer);
             CBDeliverIndication (_broker, ctx, namespaceName,
                                           indicationInstance);
        }
    }
    free (namespaceName);
    CMRelease (objPath);

    CMReturn (CMPI_RC_OK);
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMIndicationMIStub (TestCMPIIndicationStressTestProvider,
                    TestCMPIIndicationStressTestProvider, _broker, CMNoHook )


CMMethodMIStub (TestCMPIIndicationStressTestProvider,
                TestCMPIIndicationStressTestProvider, _broker, CMNoHook)

/* ---------------------------------------------------------------------------*/
/*                      end of TestCMPIProvider                               */
/* ---------------------------------------------------------------------------*/


