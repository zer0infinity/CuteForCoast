/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyImplsTest_H
#define _AnyImplsTest_H

#include "cute.h"//lint !e537

//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class AnyImplsTest {
public:
	//--- constructors

	/*! \param name name of the test */
	AnyImplsTest();

	//! destroys the test case
	~AnyImplsTest();

	static void runAllTests(cute::suite &s);

	//! describe this testcase
	void ThisToHexTest();
};

#endif
