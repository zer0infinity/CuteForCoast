/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file lgpl.txt.
 */

#include "test_dummy.h"

void TestDummy::dummyTest() {
	ASSERT(1);
	ASSERT_EQUAL(1, 1);
	ASSERT_EQUAL_DELTA(1.2, 1.2000000001, 1E-5);
	ASSERT_EQUAL("Test", "Test");
	ASSERT_EQUAL(0, false);
	ASSERT_EQUAL(1, true);
}

void TestDummy::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(TestDummy, dummyTest));
}
