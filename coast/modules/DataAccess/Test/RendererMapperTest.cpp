/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RendererMapperTest.h"
#include "TestSuite.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "DataMapper.h"
#include "Context.h"

//---- RendererMapperTest ----------------------------------------------------------------
RendererMapperTest::RendererMapperTest(TString tname) : TestCaseType(tname)
{
}

RendererMapperTest::~RendererMapperTest()
{
}

Test *RendererMapperTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RendererMapperTest, StdGetTest);
	ADD_CASE(testSuite, RendererMapperTest, GetOnAnyTest);

	return testSuite;
}

void RendererMapperTest::StdGetTest()
{
	Anything dummy;
	Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);

	// fConfig does not appear to be set properly
	RendererMapper mapper("renderertestmapper");
	mapper.Initialize("ParameterMapper");

	// test the overloaded get api
	String iString("<");
	{
		OStringStream iStream(&iString);
		t_assert(((ParameterMapper &)mapper).Get("NotRelevant", iStream, ctx));
	}
	iString << ">";
	assertEqual("<Get www.muc.de/~kenny http/1.0\r\n\r\n>", iString);
}

void RendererMapperTest::GetOnAnyTest()
{
	{
		Anything dummy;
		Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);
		RendererMapper mapper("renderertestmapper");
		mapper.Initialize("ParameterMapper");
		// test the overloaded get api
		Anything result;
		t_assert(mapper.Get("NotRelevant", result, ctx));
		assertEqual("Get www.muc.de/~kenny http/1.0\r\n\r\n", result.AsString());
	}
	{
		Anything dummy;
		Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);
		RendererMapper mapper("renderertestmapper");
		mapper.Initialize("ParameterMapper");
		// test nonexisting slot
		Anything result;
		t_assert(!mapper.Get("NonExisting", result, ctx));
		t_assert(result.IsNull());
	}
}

void RendererMapperTest::setUp ()
{
	std::iostream *Ios = coast::system::OpenStream("StdContext", "any");
	if ( Ios ) {
		fStdContextAny.Import((*Ios));
		delete Ios;
	}
}
