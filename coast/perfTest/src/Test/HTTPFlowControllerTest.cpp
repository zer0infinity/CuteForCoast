/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPFlowControllerTest.h"
#include "TestSuite.h"
#include "CheckStores.h"

//---- HTTPFlowControllerTest ----------------------------------------------------------------
HTTPFlowControllerTest::HTTPFlowControllerTest(TString tstrName) :
	ConfiguredActionTest(tstrName) {
	StartTrace(HTTPFlowControllerTest.HTTPFlowControllerTest);
}

TString HTTPFlowControllerTest::getConfigFileName() {
	return "HTTPFlowControllerTestConfig";
}

// Checks if the tmpStore is prepared according the FlowController's configuration
void HTTPFlowControllerTest::DoTest(Anything testCase, const char *testCaseName) {
	StartTrace1(HTTPFlowControllerTest.DoTest, "<" << testCaseName << ">");
	DoTestWithContext(testCase, testCaseName, fCtx);
	Anything anyFailureStrings;
	coast::testframework::CheckStores(anyFailureStrings, testCase["Result"], fCtx, testCaseName, coast::testframework::exists);
	for ( long sz=anyFailureStrings.GetSize(),i=0; i<sz;++i ) {
		t_assertm(false, anyFailureStrings[i].AsString().cstr());
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPFlowControllerTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPFlowControllerTest, TestCases);
	return testSuite;
}
