/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ThreadPoolTest_H
#define _ThreadPoolTest_H

#include "TestCase.h"

class ThreadPoolTest: public testframework::TestCase {
public:
	ThreadPoolTest(TString tname) :
		TestCaseType(tname) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!Tests Init Start Join
	void JoinTest();

	//!Tests Init Start Terminate
	void TerminateTest();

	void PoolManagerTest();
};

#endif
