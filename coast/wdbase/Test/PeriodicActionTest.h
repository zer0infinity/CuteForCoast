/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PeriodicActionTest_H
#define _PeriodicActionTest_H

#include "TestCase.h"
#include "Threads.h"

class Context;

//!tests for PeriodicAction thread
class PeriodicActionTest : public testframework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	PeriodicActionTest(TString tstrName);

	//!destroys the test case
	~PeriodicActionTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!test periodic action thread
	void PeriodicTest();

	//!keep it simple just increment counter
	static void ActionCalled(Context &ctx);

protected:
	static long fgCalled;
	static Condition fgCalledCond;
	static Mutex fgCalledMutex;
};

#endif
