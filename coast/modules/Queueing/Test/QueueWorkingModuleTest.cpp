/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "QueueWorkingModuleTest.h"
#include "QueueWorkingModule.h"
#include "TestSuite.h"
#include "Queue.h"
#include "Tracer.h"

//---- QueueWorkingModuleTest ----------------------------------------------------------------
QueueWorkingModuleTest::QueueWorkingModuleTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(QueueWorkingModuleTest.QueueWorkingModuleTest);
}

TString QueueWorkingModuleTest::getConfigFileName()
{
	return "QueueWorkingModuleTestConfig";
}

QueueWorkingModuleTest::~QueueWorkingModuleTest()
{
	StartTrace(QueueWorkingModuleTest.Dtor);
}

void QueueWorkingModuleTest::InitFinisNoModuleConfigTest()
{
	StartTrace(QueueWorkingModuleTest.InitFinisNoModuleConfigTest);
	AnyQueueWorkingModule aModule("QueueWorkingModule");
	t_assertm( !aModule.Init(GetTestCaseConfig()["ModuleConfig"]), "module init should have failed due to missing configuration" );
}

void QueueWorkingModuleTest::InitFinisDefaultsTest()
{
	StartTrace(QueueWorkingModuleTest.InitFinisDefaultsTest);
	AnyQueueWorkingModule aModule("QueueWorkingModule");
	if ( t_assertm( aModule.Init(GetTestCaseConfig()["ModuleConfig"]), "module init should have succeeded" ) )
	{
		if ( t_assert( aModule.GetContext() != NULL ) )
		{
			assertAnyEqual(GetTestCaseConfig()["ModuleConfig"]["QueueWorkingModule"], aModule.GetContext()->GetEnvStore());
		}
		if ( t_assert( aModule.GetQueue() != NULL ) )
		{
			assertEqualm(0L, aModule.GetQueue()->GetSize(), "expected queue to be empty");
			Anything anyStatistics;
			aModule.GetQueueStatistics(anyStatistics);
			assertEqualm(100L, anyStatistics["QueueSize"].AsLong(-1L), "expected default queue size to be 100");
		}
		// terminate module and perform some checks
		t_assert( aModule.Finis() );
		t_assert( aModule.GetContext() == NULL );
		t_assert( aModule.GetQueue() == NULL );
	}
}

void QueueWorkingModuleTest::InitFinisTest()
{
	StartTrace(QueueWorkingModuleTest.InitFinisTest);
	AnyQueueWorkingModule aModule("QueueWorkingModule");
	if ( t_assertm( aModule.Init(GetTestCaseConfig()["ModuleConfig"]), "module init should have succeeded" ) )
	{
		if ( t_assert( aModule.GetContext() != NULL ) )
		{
			// check for invalid Server
			t_assertm( NULL == aModule.GetContext()->GetServer(), "server must be NULL because of non-existing server");
			assertAnyEqual(GetTestCaseConfig()["ModuleConfig"]["QueueWorkingModule"], aModule.GetContext()->GetEnvStore());
		}
		if ( t_assert( aModule.GetQueue() != NULL ) )
		{
			assertEqualm(0L, aModule.GetCurrentSize(), "expected queue to be empty");
			Anything anyStatistics;
			aModule.GetQueueStatistics(anyStatistics);
			assertEqualm(25L, anyStatistics["QueueSize"].AsLong(-1L), "expected queue size to be 25 as configured");
		}

		// terminate module and perform some checks
		t_assert( aModule.Finis() );
		t_assert( aModule.GetContext() == NULL );
		t_assert( aModule.GetQueue() == NULL );
	}
}

void QueueWorkingModuleTest::GetAndPutbackTest()
{
	StartTrace(QueueWorkingModuleTest.GetAndPutbackTest);
	AnyQueueWorkingModule aModule("QueueWorkingModule");
	typedef AnyQueueWorkingModule::QueueType QueueType;
	// set modules fAlive-field to enable working of the functions
	// first check if they don't work
	{
		Anything anyMsg;
		// must fail
		assertEqual( QueueType::eDead, aModule.PutElement(anyMsg, false) );
		// fails because of uninitialized queue
		assertEqual( QueueType::eDead, aModule.GetElement(anyMsg,false) );
	}
	aModule.IntInitQueue(GetTestCaseConfig()["ModuleConfig"]["QueueWorkingModule"]);
	{
		Anything anyMsg;
		// must still fail because of dead-state
		assertEqual( QueueType::eDead, aModule.GetElement(anyMsg,false) );
	}
	aModule.fAlive = 0xf007f007;
	if ( t_assertm( aModule.GetQueue() != NULL , "queue should be created" ) )
	{
		// queue is empty, so retrieving an element with tryLock=true should
		// return immediately and fail.
		{
			Anything anyElement;
			assertEqual( QueueType::eEmpty, aModule.GetElement(anyElement,true) );
		}
		// queue size is 1, so we load it with 1 element
		{
			Anything anyMsg;
			anyMsg["Number"] = 1;
			// this one must succeed
			assertEqual( QueueType::eSuccess, aModule.PutElement(anyMsg, false) );
		}
		// now putback one message
		{
			Anything anyMsg;
			anyMsg["Number"] = 2;
			// this one must fail
			if ( assertEqual( QueueType::eFull, aModule.PutElement(anyMsg, true) ) )
			{
				aModule.PutBackElement(anyMsg);
				assertEqualm(1, aModule.fFailedPutbackMessages->GetSize(), "expected overflow buffer to contain an element");
			}
		}
		Anything anyElement;
		if ( assertEqual( QueueType::eSuccess, aModule.GetElement(anyElement) ) )
		{
			assertEqualm( 2, anyElement["Number"].AsLong(-1L), "expected to get putback element first");
		}
		assertEqualm( 0, aModule.fFailedPutbackMessages->GetSize(), "expected overflow buffer to be empty now");
		if ( assertEqual( QueueType::eSuccess, aModule.GetElement(anyElement) ) )
		{
			assertEqualm( 1, anyElement["Number"].AsLong(-1L), "expected to get regular queue element last");
		}
		assertEqualm( 0, aModule.GetCurrentSize(), "expected queue to be empty now");
		aModule.IntCleanupQueue();
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *QueueWorkingModuleTest::suite ()
{
	StartTrace(QueueWorkingModuleTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, QueueWorkingModuleTest, InitFinisNoModuleConfigTest);
	ADD_CASE(testSuite, QueueWorkingModuleTest, InitFinisDefaultsTest);
	ADD_CASE(testSuite, QueueWorkingModuleTest, InitFinisTest);
	ADD_CASE(testSuite, QueueWorkingModuleTest, GetAndPutbackTest);

	return testSuite;
}
