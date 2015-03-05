/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConfiguredLookupAdapterTest.h"
#include "ConfiguredLookupAdapter.h"
#include "TestSuite.h"

ConfiguredLookupAdapterTest::ConfiguredLookupAdapterTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(ConfiguredLookupAdapterTest.ConfiguredLookupAdapterTest);
}

TString ConfiguredLookupAdapterTest::getConfigFileName()
{
	return "ConfiguredLookupAdapterTestConfig";
}

ConfiguredLookupAdapterTest::~ConfiguredLookupAdapterTest()
{
	StartTrace(ConfiguredLookupAdapterTest.Dtor);
}

void ConfiguredLookupAdapterTest::LookupTest()
{
	StartTrace(ConfiguredLookupAdapterTest.LookupTest);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(caseConfig) ) {
		ROAnything conf(caseConfig["Config"]);
		ROAnything def(caseConfig["Default"]);
		ConfiguredLookupAdapter cla(conf, def);
		assertEqual(caseConfig["ExpectedString"].AsString(), cla.Lookup(caseConfig["LookupPathString"].AsString(), ""));
		assertEqual(caseConfig["ExpectedLong"].AsLong(1L), cla.Lookup(caseConfig["LookupPathLong"].AsString(""), 0L));
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *ConfiguredLookupAdapterTest::suite ()
{
	StartTrace(ConfiguredLookupAdapterTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConfiguredLookupAdapterTest, LookupTest);
	return testSuite;
}
