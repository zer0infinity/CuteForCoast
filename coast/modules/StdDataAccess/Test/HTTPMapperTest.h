/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPMapperTest_H
#define _HTTPMapperTest_H

#include "WDBaseTestPolicies.h"
#include "Anything.h"

class Anything;

//---- HTTPMapperTest ----------------------------------------------------------
//!TestClass for for HTMLParser functionality
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class HTTPMapperTest : public testframework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:

	//--- constructors
	HTTPMapperTest(TString tstrName);
	~HTTPMapperTest();

	//--- public api
	//!generate HTMLParser test suite
	//! this method generates the tests for the HTMLParser classed
	//! \return a new test is created by this method
	static Test *suite ();

protected:
	//--- tests for public api of class connector

	//!tests htmlparser with a file from config
	void FDTest1();
	void FDTest2();
	void FDTest3();
	void FDTest4();

	void HTTPBodyMapperBadStream();

	void GetTestInput(Anything &testInput, const char *testname);
	String PrepareResults(ROAnything resultsAsAny);
};

#endif
