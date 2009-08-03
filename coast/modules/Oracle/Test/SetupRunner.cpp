/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "ConfiguredActionTest.h"
#include "MultiThreadedTest.h"
#include "StatementDescriptionTest.h"

void setupRunner(TestRunner &runner)
{
	// execute tests in test dir with defined structure
	ADD_SUITE(runner, ConfiguredActionTest);
	ADD_SUITE(runner, MultiThreadedTest);
	ADD_SUITE(runner, StatementDescriptionTest);
}
