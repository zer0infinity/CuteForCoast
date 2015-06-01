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

#ifndef _GenericXMLParserTest_H
#define _GenericXMLParserTest_H

#include "cute.h"
#include "StringStream.h"
#include "Tracer.h"
#include "AnyIterators.h"
#include "AssertionAnything.h"
#include "SystemFile.h"

//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class GenericXMLParserTest : public Assertion {
public:
	//! TestCase constructor
	//! \param name name of the test
	GenericXMLParserTest();

	//! destroys the test case
	~GenericXMLParserTest();

	//! builds up a suite of testcases for this test
	static void runAllTests(cute::suite &s);
	ROAnything GetTestCaseConfig(String strClassName = "", String strTestName = "");
	//! describe this testcase
	void simpleEmptyTag();
	void simpleAttributeTag();
	void simpleBodyTag();
	void simpleAttributeBodyTag();
	void simpleTagWithComment();
	void simpleNestedTags();
	void simpleExampleXML();
	void simpleDTDExampleXML();
	void simpleXMLError();
	void simpleParsePrint();
	void configuredTests();
};

#endif
