/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RendererTest.h"
#include "TestSuite.h"
#include <iostream>

RendererTest::RendererTest(TString tname) :
	TestCaseType(tname), fContext(Anything(), Anything(), 0, 0, 0, 0), fReply(), fConfig() {
}

void RendererTest::setUp() {
	fContext.SetLanguage("D"); // set a default language explicitely
}

Test *RendererTest::suite()
/* what: return the whole suite of tests for renderers, we could add the suites
 of the derived classes here, but we would need to include all the headers
 of the derived classes which is not nice.
 Add cases for the generic Renderer here (if you find any that is);
 */
{
	TestSuite *testSuite = new TestSuite;
	return testSuite;
}

void RendererTest::printResult(const char *str0, const char *str1) {
	std::cerr << std::endl << "  " << str0 << " :  " << str1 << std::endl;
}

void RendererTest::printReplyBody(const char *MethodeName) {
	String nameBuffer(MethodeName);
	std::cerr << nameBuffer << " :  " << fReply.str() << std::endl;
}
