/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WDModuleTest_H
#define _WDModuleTest_H

#include "TestCase.h"

class Registry;

class WDModuleTest : public testframework::TestCase
{
public:
	//!TestCases
	//! \param name name of the test
	WDModuleTest(TString tstrName);

	//!destroys the test case
	~WDModuleTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	void InstallTest ();
	void Install2Test ();
	void ResetTest ();
	void TerminateTest ();
	void ResetWithDiffConfigsTest ();

protected:
	Registry *fOrigWDModuleRegistry;
};

#endif
