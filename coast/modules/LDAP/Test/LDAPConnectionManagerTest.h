/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPConnectionManagerTest_H
#define _LDAPConnectionManagerTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- LDAPConnectionManagerTest ----------------------------------------------------------
//! <B>Tests LDAPConnecton and LDAPErrorHandler</B>
class LDAPConnectionManagerTest : public ConfiguredTestCase
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	LDAPConnectionManagerTest(TString name);

	//! destroys the test case
	~LDAPConnectionManagerTest();

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this Test
	void testLDAPConnectionManager();
};

#endif
