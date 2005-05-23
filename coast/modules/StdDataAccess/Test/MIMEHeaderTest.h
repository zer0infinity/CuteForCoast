/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MIMEHeaderTest_H
#define _MIMEHeaderTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- MIMEHeaderTest ----------------------------------------------------------
//!TestCases description
class MIMEHeaderTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	MIMEHeaderTest(TString tstrName);

	//!destroys the test case
	~MIMEHeaderTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!describe this testcase
	void SimpleHeaderTest();
	//!describe this testcase
	void MultiPartHeaderTest();
	//!describe this testcase
	void PartHeaderTest();

protected:

};

#endif
