/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataMapperTest_H
#define _DataMapperTest_H

#include "WDBaseTestPolicies.h"

//---- DataMapperTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class DataMapperTest : public testframework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors
	DataMapperTest(TString tstrName);
	~DataMapperTest();

	static Test *suite ();

	virtual TString getConfigFileName() {
		return "StdContext";
	}

	//--- public api
	void StdGetTest();
	void NegativGetTest();

	void FixedSizeTest();
	void UppercaseTest();
};

#endif
