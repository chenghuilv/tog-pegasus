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
//%////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Server/EnumerationContext.h>
#include <Pegasus/Server/EnumerationContextTable.h>
#include <Pegasus/Server/CIMOperationRequestDispatcher.h>
#include <Pegasus/General/Guid.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/CIMResponseData.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static Boolean verbose;
#define VCOUT if (verbose) cout

// Reduce creation of simple context to one line for these tests.
EnumerationContext* createSimpleContext(EnumerationContextTable& enumTable,
                                        String& name)
{
    const CIMNamespaceName ns = "/test/namespace";
    Boolean continueOnError = false;
    Uint32Arg timeOut = 50;
    EnumerationContext* en = enumTable.createContext(
        ns,
        timeOut,
        continueOnError,
        CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
        CIMResponseData::RESP_INSTANCES);
    name = en->getContextId();
    return en;
}
// Create
void test01()
{
    VCOUT << "Start test01" << endl;
    EnumerationContextTable enumTable(1000);
    cout << "line " << __LINE__ << endl;
    String createdContextName1;
    EnumerationContext* en1 = createSimpleContext(enumTable,
                                                     createdContextName1);

    PEGASUS_TEST_ASSERT(en1->responseCacheSize() == 0);

    //String createdContextName1 = et.createContext(enContext1);

    String createdContextName2;
    EnumerationContext* en2 = createSimpleContext(enumTable,
                                             createdContextName2);

    cout << "line " << __LINE__ << endl;
    EnumerationContext* enFound2 = enumTable.find(createdContextName2);

    PEGASUS_TEST_ASSERT(enFound2->valid());

    PEGASUS_TEST_ASSERT(enFound2 != 0);

    // test find on string not in table.
    PEGASUS_TEST_ASSERT(enumTable.find(String("xxxx")) == 0);

    PEGASUS_TEST_ASSERT(enumTable.size() == 2);

    cout << "line " << __LINE__ << endl;
    en1->setClientClosed();
    en1->setProvidersComplete();

    en2->setClientClosed();
    en2->setProvidersComplete();
    en1->lockContext();
    en2->lockContext();
    cout << "line " << __LINE__ << endl;
    enumTable.releaseContext(en1);
    enumTable.releaseContext(en2);

    cout << "line " << __LINE__ << endl;
    PEGASUS_TEST_ASSERT(enumTable.size() == 0);

    //enumTable.removeContextTable();
    VCOUT << "EnumTest End test01 successful" << endl;
}

// test creating a lot of contexts and then finding them
void test02()
{
    VCOUT << "Start test02" << endl;
    // Create the enumeration table
    EnumerationContextTable enumTable(1000);

    // Build a large set of contexts
    Uint32 testSize = 1000;
    Array<String> rtndContextNames;

    PEGASUS_TEST_ASSERT(enumTable.size() == 0);

    // Build a set of contexts defined by testSize
    for (Uint32 i = 0 ; i < testSize ; i++)
    {
        String createdContextName;
        PEGASUS_TEST_ASSERT(createSimpleContext(enumTable,createdContextName));

        rtndContextNames.append(createdContextName);
        PEGASUS_TEST_ASSERT(rtndContextNames[i].size() != 0);
    }

    PEGASUS_TEST_ASSERT(enumTable.size() == testSize);

    // confirm that we can find all of the created contexts
    for (Uint32 i = 0 ; i < testSize ; i++)
    {
        EnumerationContext* en = enumTable.find(rtndContextNames[i]);
        PEGASUS_TEST_ASSERT(en != 0);
        PEGASUS_TEST_ASSERT(en->valid());
    }
    PEGASUS_TEST_ASSERT(enumTable.size() == testSize);

    // Confirm that contexts are unique.
    // KS_TODO the uniqueness test.

    // remove all contexts from table.
    for (Uint32 i = 0 ; i < testSize ; i++)
    {
        EnumerationContext* en = enumTable.find(rtndContextNames[i]);
        en->setProvidersComplete();
        en->setClientClosed();
        en->lockContext();
        enumTable.releaseContext(en);
    }

    PEGASUS_TEST_ASSERT(enumTable.size() == 0);

    enumTable.removeContextTable();

    VCOUT << "test02 Complete success" << endl;
}

// Test Enumeration Timeout.
//void test03()
//{
//    VCOUT << "Start test03, Test Enumeration Timeouts" << endl;
//    EnumerationContextTable enumTable(1000);
//
//    String createdContextName;
//
//    const CIMNamespaceName ns = "/test/namespace";
//    Boolean continueOnError = false;
//    Uint32Arg x = 2;
//    EnumerationContext* en = enumTable.createContext(
//        ns,
//        x,
//        continueOnError,
//        CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
//        CIMResponseData::RESP_INSTANCES,
//        createdContextName);
//
//    // test expired timer test before we start a timer.
//    Array<String> contexts;
//
//    Uint32 activeEnumerations = 0;
//    activeEnumerations = enumTable.findExpiredContexts(contexts);
//    PEGASUS_TEST_ASSERT(activeEnumerations == 0);
//    PEGASUS_TEST_ASSERT(contexts.size() == 0);
//
//    // test the timer on the single context
//
//    en->startTimer();
//    activeEnumerations = enumTable.findExpiredContexts(contexts);
//    PEGASUS_TEST_ASSERT(activeEnumerations == 1);
//    ///////////PEGASUS_TEST_ASSERT(contexts.size() == 0);
//
//    Uint32 seconds = 0;
//    Array<String> list;
//    Uint32 actives;
//    Array<String> expiredList;
//    while ((actives = enumTable.findExpiredContexts(list)) != 0)
//    {
//        PEGASUS_TEST_ASSERT(actives == 1);
//        for (Uint32 i = 0 ; i < list.size() ; i++)
//        {
//            cout << list[i] << endl;
//            expiredList.append(list[i]);
//        }
//        seconds++;
//        sleep(1);
//    }
//    VCOUT << "seconds " << seconds << endl;
//    PEGASUS_TEST_ASSERT(seconds >= 2 && seconds <= 4);
//
//    if (expiredList.size() > 0)
//    {
//        EnumerationContext* en = enumTable.find(expiredList[0]);
//
//        PEGASUS_TEST_ASSERT(en->valid());
//
//        en->setProvidersComplete();
//
//        enumTable.remove(createdContextName);
//    }
//    else
//    {
//        PEGASUS_ASSERT(false);
//    }
//
//    PEGASUS_TEST_ASSERT(enumTable.size() == 0);
//
//    VCOUT << "test03 Complete success" << endl;
//}
//
//
//// Test Enumeration Timeout with multiple contexts.
//void test03a()
//{
//    VCOUT << "Start test03a" << endl;
//    VCOUT << "Start test03a, Test Multiple Enumeration Timeouts" << endl;
//
//    EnumerationContextTable enumTable(1000);
//
//    // Insert multiple context entries into the enumeration table.
//
//    String createdContextXName;
//    EnumerationContext* x = createSimpleContext(enumTable,
//                                                 createdContextXName);
//
//
//    // Add new entry, start both timers and test for
//    // completions.
//    String createdContextYName;
//    EnumerationContext* y = createSimpleContext(enumTable,
//                                                 createdContextYName);
//
//
//    String createdContextZName;
//    EnumerationContext* z = createSimpleContext(enumTable,
//                                                 createdContextZName);
//
//
//    // Start the interoperation timer for all contexts
//
//    x->startTimer();
//    y->startTimer();
//    z->startTimer();
//
//    // Test for timeouts.
//
//    Array<String> timedOutList;
//    Uint32 activeTimers = 0;
//    Uint32 seconds = 0;
//    while ((activeTimers = enumTable.findExpiredContexts(timedOutList)) != 0)
//    {
//         VCOUT << "activeTimers = " << activeTimers << " at "
//             << seconds << " seconds. timeouts count "
//             << timedOutList.size() << endl;
//
//        // Confirm that returned timed out contexts are valid
//        if (timedOutList.size() != 0)
//        {
//            for (Uint32 i = 0 ; i < timedOutList.size() ; i++)
//            {
//                VCOUT << "context " << timedOutList[i]
//                    << " Timed Out at " << seconds << " seconds."
//                    << "timeouts count "<< timedOutList.size()
//                    << " i " << i <<  endl;
//                EnumerationContext* en = enumTable.find(timedOutList[i]);
//                en->valid();
//
//                // Test the context against the id and time for each entry
//                // to determine the timeout times are correct.
//
//                if (en->getContextName() == x->getContextName())
//                {
//                    PEGASUS_TEST_ASSERT(en->getContextName() ==
//                                        createdContextXName);
//                    PEGASUS_TEST_ASSERT(seconds == 3);
//                }
//                if (en->getContextName() == y->getContextName())
//                {
//                    PEGASUS_TEST_ASSERT(en->getContextName() ==
//                                        createdContextYName);
//                    PEGASUS_TEST_ASSERT(seconds == 6);
//                }
//                if (en->getContextName() == z->getContextName())
//                {
//                    PEGASUS_TEST_ASSERT(en->getContextName() ==
//                                        createdContextZName);
//                    cout << "seconds " << seconds << endl;
//                    PEGASUS_TEST_ASSERT(seconds == 1);
//                }
//            }
//        }
//        seconds++;
//        sleep(1);
//    }
//    VCOUT << "all timers off now. seconds ="
//        << seconds << endl;
//    PEGASUS_TEST_ASSERT(enumTable.size() == 3);
//
//    // clear the table are test for size 0
//
//    PEGASUS_ASSERT(!enumTable.remove(createdContextXName));
//    enumTable.remove(createdContextYName);
//    enumTable.remove(createdContextZName);
//    // should not be removed because provider state not complete.
//    PEGASUS_TEST_ASSERT(enumTable.size() == 3);
//
//    EnumerationContext* en1 = enumTable.find(createdContextXName);
//    en1->setProvidersComplete();
//    PEGASUS_ASSERT(enumTable.remove(createdContextXName));
//    PEGASUS_TEST_ASSERT(enumTable.size() == 2);
//
//    EnumerationContext* en2 = enumTable.find(createdContextYName);
//    en2->setProvidersComplete();
//    enumTable.remove(createdContextYName);
//    PEGASUS_TEST_ASSERT(enumTable.size() == 1);
//
//    EnumerationContext* en3 = enumTable.find(createdContextZName);
//    en3->setProvidersComplete();
//    enumTable.remove(createdContextZName);
//    PEGASUS_TEST_ASSERT(enumTable.size() == 0);
//
//    enumTable.clear();
//
//    VCOUT << "test03a Complete success" << endl;
//}

// test the cache get functions.
void test04()
{
    VCOUT << "Start test04" << endl;
    // Create an entry for the cache

    // define an EnumerationContext
    EnumerationContextTable enumTable(1000);

    String createdContextName;
    EnumerationContext* en = createSimpleContext(enumTable,
                                                 createdContextName);

    PEGASUS_TEST_ASSERT(createdContextName == en->getContextId());


    // Put them into the cache
    VCOUT << "add response instance to cache" << endl;
    // Add one instance to cache.  Should not cause wait to end.
    for (Uint32 i = 0; i < 12; i++)
    {
        {
            QueueIdStack x;
            CIMEnumerateInstancesResponseMessage* response
                =  new CIMEnumerateInstancesResponseMessage(
                    "",
                    CIMException(),
                    x);

            en->putCache((CIMResponseMessage*&)response, false);
        }
    }
    //  get from the cache
    VCOUT << "getCacheResponseData" << endl;
    CIMResponseData from(CIMResponseData::RESP_INSTANCES);
    en->getCache(10, from);
    PEGASUS_TEST_ASSERT(from.size() == 10);
    enumTable.removeContextTable();
}


// test the cache condition variable functions. This includes thread functions
// and the test function.

Boolean threadComplete;
ThreadReturnType PEGASUS_THREAD_CDECL setProvidersComplete(void * parm)
{
    VCOUT << "Thread Start" << endl;
    Thread* my_thread = (Thread *)parm;
    EnumerationContext* enumerationContext =
        (EnumerationContext *)my_thread->get_parm();

    PEGASUS_ASSERT(enumerationContext->valid());

    // Add a test where we add some small number of items to the
    // cache.

    // signal again after setting providersComplete.
    // This one should wake up the condition.
    sleep(1);
    threadComplete = true;
    VCOUT << "Thread Sleep complete" << endl;
    enumerationContext->setProvidersComplete();
    VCOUT << "setProvidersComplete() Signaled" << endl;
    return ThreadReturnType(0);
}

// Test ProvidersComplete flag setting and getCacheResponseData
void test06()
{
    VCOUT << "Test06 Started" << endl;
    EnumerationContextTable enumTable(1000);

    // Create an enumeration context
    String enContextIdStr;
    EnumerationContext* en = createSimpleContext(enumTable,
                                                 enContextIdStr);

    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 0);
    threadComplete = false;

    Thread * th = new Thread(setProvidersComplete, (void *)en, false);

    th->run();

    // call the testresponseCacheSize Condition. Should return only when the
    // thread has signaled.
    VCOUT << "getCacheResponseData" << endl;
    CIMResponseData from(CIMResponseData::RESP_INSTANCES);
    en->getCache(10, from);

    th->join();
    delete th;
    en->lockContext();
    enumTable.releaseContext(en);
}
ThreadReturnType PEGASUS_THREAD_CDECL setresponseCacheSize(void * parm)
{
    VCOUT << "Thread Start" << endl;
    Thread* my_thread = (Thread *)parm;
    EnumerationContext* enumerationContext =
        (EnumerationContext *)my_thread->get_parm();

    PEGASUS_ASSERT(enumerationContext->valid());
    QueueIdStack x;
//  VCOUT << "create request" << endl;
//  CIMEnumerateInstancesRequestMessage* request =
//      new CIMEnumerateInstancesRequestMessage(
//          "", CIMNamespaceName(), CIMName(), true, true,
//          true, CIMPropertyList(),x);

//  VCOUT << "Create poA" << endl;
//  OperationAggregate* poA = new OperationAggregate(
//      new CIMEnumerateInstancesRequestMessage(*request),
//          request->className,
//          request->nameSpace,
//          0,
//          false, true);

    VCOUT << "add response instance to cache" << endl;
    // Add one instance to cache.  Should not cause wait to end.
    {
        CIMEnumerateInstancesResponseMessage* response
            =  new CIMEnumerateInstancesResponseMessage(
                "",
                CIMException(),
                x);

        enumerationContext->putCache((CIMResponseMessage*&)response, false);
    }
    // signal again after setting providersComplete.
    // This one should wake up the condition.
    sleep(1);
    threadComplete = true;
    VCOUT << "Thread Sleep complete" << endl;
    {
        // add second instance to cache
        CIMEnumerateInstancesResponseMessage* response
            =  new CIMEnumerateInstancesResponseMessage(
                "",
                CIMException(),
                x);

        enumerationContext->putCache((CIMResponseMessage*&)response, false);
    }
    VCOUT << "setresponseCacheSize to 2 items ThreadComplete" << endl;
    return ThreadReturnType(0);
}

// Test cache size  setting and getCacheResponseData
void test07()
{
    EnumerationContextTable enumTable(1000);

    // Create an enumeration context
    String enContextIdStr;
    EnumerationContext* en = createSimpleContext(enumTable,
                                                 enContextIdStr);

    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 0);
    threadComplete = false;
    // Create a thread with Enumeration Context as parameter
//  char * param;
    Thread * th = new Thread(setresponseCacheSize, (void *)en, false);

    th->run();

    // call the testresponseCacheSize Condition. Should return only when the
    // thread has signaled.
    VCOUT << "getCacheResponseData Wait" << endl;
    CIMResponseData from(CIMResponseData::RESP_INSTANCES);
    cout << en->responseCacheSize();
    en->getCache(1, from);
    VCOUT << "getCacheResponseData Wait return" << endl;
    PEGASUS_TEST_ASSERT(threadComplete);

    th->join();
    delete th;
}


int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;
    test01();
    test02();
    // Removed because we modified whole timer concept and these no
    // longer work.
//  test03();
//  test03a();

    // putcache and getResponseCache test functions.  Text the condition
    // variable wait logic.
    test06();

    // KS_TODO we have an issue with this test.
    //test07();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
