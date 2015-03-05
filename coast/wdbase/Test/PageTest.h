/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageTest_H
#define _PageTest_H

#include "TestCase.h"
#include "Context.h"

class PageTest : public testframework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	PageTest(TString tstrName);

	//!destroys the test case
	~PageTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!describe this testcase
	void FinishTest();
	//!describe this testcase
	void PrepareTest();

protected:
	Anything fActionConfig;
	Context fCtx;
};

#endif
