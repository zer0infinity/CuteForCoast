/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConversionUtilsTest_H
#define _ConversionUtilsTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- ConversionUtilsTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ConversionUtilsTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param name name of the test */
	ConversionUtilsTest(TString tstrName);

	//! destroys the test case
	~ConversionUtilsTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! describe this testcase
	void GetValueFromBufferTest();
};

#endif