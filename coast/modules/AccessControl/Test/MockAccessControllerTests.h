/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MockAccessControllerTests_H
#define _MockAccessControllerTests_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"
#include "FileAccessControllerTests.h"

//---- MockAccessControllerTests ------------------------------------------
//! <B>Tests the MockAccessControllers (MockUDAC, MockTDAC, MockEDAC)</B>
/*!
Each of the tested AccessControllers has an own config file.
*/
class MockAccessControllerTests : public FileAccessControllerTests
{
public:
	//--- constructors
	MockAccessControllerTests(TString tstrName) : FileAccessControllerTests(tstrName, "MockAccessControllerTestsConfig") {}
	~MockAccessControllerTests() {};

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	void testMockUDAC();
	void testMockTDAC();
	void testMockEDAC();
};

#endif
