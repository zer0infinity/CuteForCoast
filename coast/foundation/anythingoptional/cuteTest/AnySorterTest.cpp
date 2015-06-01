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

#include "AnySorterTest.h"
#include "AnySorter.h"

AnySorterTest::AnySorterTest() {
}

AnySorterTest::~AnySorterTest() {
	StartTrace(AnySorterTest.Dtor);
}

void AnySorterTest::SorterTest()
{
	StartTrace(AnySorterTest.SorterTest);
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		TraceAny(cConfig, "the config");
		Anything sorted;
		AnySorter::EMode mode(cConfig["Mode"].AsString() == "asc" ? AnySorter::asc : AnySorter::desc);
		bool sortCritIsNumber(cConfig["SortCritIsNumber"].AsBool(0));
		sorted = cConfig["TestArray"].DeepClone();
		AnySorter::SortByKeyInArray(cConfig["SortKey"].AsString(), sorted, mode, sortCritIsNumber);
		ASSERT_ANY_EQUAL(	cConfig["ExpectedResult"], sorted);
	}
}

ROAnything AnySorterTest::GetTestCaseConfig(String strClassName, String strTestName) {
	Anything fConfig;
	if(!coast::system::LoadConfigFile(fConfig, "AnySorterTest", "any")) {
		ASSERT_EQUAL("'read AnySorterTest.any'", "'could not read AnySorterTest.any'");
	}
	return fConfig[strClassName][strTestName];
}

void AnySorterTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnySorterTest, SorterTest));
}
