/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WriteFileDAImplTest_H
#define _WriteFileDAImplTest_H

#include "ConfiguredActionTest.h"

//---- WriteFileDAImplTest ----------------------------------------------------------
//!Tests the WriteFileDAImpl
class WriteFileDAImplTest : public ConfiguredActionTest
{
public:
	//--- constructors

	//!constructor
	//! \param name name of the test
	WriteFileDAImplTest(TString tstrName);

	//!destroys the test case
	~WriteFileDAImplTest();

	//--- public api

	//!builds up a suite of Tests for this test
	static Test *suite ();

	TString getConfigFileName();

	//!describe this Test
	void WriteFileTest();

protected:
	bool CompareResult(TString strResult);
};

#endif
