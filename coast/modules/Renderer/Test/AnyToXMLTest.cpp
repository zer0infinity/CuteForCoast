/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "Context.h"
#include "TestSuite.h"
#include "AnyToXMLTest.h"

AnyToXMLTest::AnyToXMLTest(TString tname)
	: TestCaseType(tname)
	, fAnyToXMLRenderer("TestedRenderer")
{}

AnyToXMLTest::~AnyToXMLTest()
{}

void AnyToXMLTest::setUp()
{
	String configFilename("AnyToXMLTestConfig");

	std::istream *ifp = coast::system::OpenStream(configFilename, "any");

	if (ifp) {
		fConfig.Import(*ifp, configFilename);
		delete ifp;
	}
};

// Produce an XML out of an Anything
void AnyToXMLTest::AnythingToXMLTest()
{
	DoTest("Test1", "XMLOutput");
	DoTest("Test2", "XMLOutput");
}

// Production of XML fails in theses cases
void AnyToXMLTest::WrongInputTests()
{

	DoTest("BadTest1", "EmptyOutput");
	DoTest("BadTest2", "EmptyOutput");
	DoTest("BadTest3", "EmptyOutput");
	DoTest("BadTest4", "EmptyOutput");
	DoTest("FirstSlotWithUnnamedChildsTest", "EmptyOutput");
	DoTest("UnnamedWithinNamedTest", "UnnamedWithinNamedOutput");
	DoTest("NamedWithinUnnamedTest", "NamedWithinUnnamedOutput");
	DoTest("UnnamedChildWithUnnamedChildsTest", "UnnamedChildWithUnnamedChildsOutput");
}

void AnyToXMLTest::DoTest(String RendererConfig, String ExpectedOutput)
{
	Anything dummy;
	Context c(dummy, fConfig, 0, 0, 0, 0);
	OStringStream reply;
	fAnyToXMLRenderer.RenderAll(reply, c, fConfig[RendererConfig]);
	assertEqual(fConfig[ExpectedOutput].AsString("X"), reply.str());
}

Test *AnyToXMLTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnyToXMLTest, AnythingToXMLTest);
	ADD_CASE(testSuite, AnyToXMLTest, WrongInputTests);
	return testSuite;
}

