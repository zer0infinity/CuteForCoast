/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-modules used -----------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- interface include --------------------------------------------------------
#include "HTTPProtocolReplyRendererTest.h"
#include "HTTPProtocolReplyRenderer.h"
#include "StringStream.h"
#include "Context.h"

//---- HTTPProtocolReplyRendererTest ----------------------------------------------------------------
HTTPProtocolReplyRendererTest::HTTPProtocolReplyRendererTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(HTTPProtocolReplyRendererTest.Ctor);
}

HTTPProtocolReplyRendererTest::~HTTPProtocolReplyRendererTest()
{
	StartTrace(HTTPProtocolReplyRendererTest.Dtor);
}

// setup for this TestCase
void HTTPProtocolReplyRendererTest::setUp ()
{
	StartTrace(HTTPProtocolReplyRendererTest.setUp);
}

void HTTPProtocolReplyRendererTest::tearDown ()
{
	StartTrace(HTTPProtocolReplyRendererTest.tearDown);
}

void HTTPProtocolReplyRendererTest::testCase()
{
	StartTrace(HTTPProtocolReplyRendererTest.testCase);
}
void HTTPProtocolReplyRendererTest::ReasonLessErrorReplyLine()
{
	StartTrace(HTTPProtocolReplyRendererTest.ReasonLessErrorReplyLine);
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");
	Context c;
	OStringStream response;
	c.GetTmpStore()["HTTPStatus"]["ResponseCode"] = 500L;
	r.RenderAll(response, c, ROAnything());
	assertEqual("HTTP/1.1 500 Server Error\r\nConnection: close\r\n", response.str());
	assertEqual(500L, c.Lookup("HTTPStatus.ResponseCode", -1L));

}

void HTTPProtocolReplyRendererTest::ReasonLessReplyLine()
{
	StartTrace(HTTPProtocolReplyRendererTest.ReasonLessReplyLine);
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");
	Context c;
	OStringStream response;
	r.RenderAll(response, c, ROAnything());
	assertEqual("HTTP/1.1 200 OK\r\nConnection: close\r\n", response.str());
	assertEqual(200L, c.Lookup("HTTPStatus.ResponseCode", -1L));
}

void HTTPProtocolReplyRendererTest::ConnectionCloseTest()
{
	StartTrace(HTTPProtocolReplyRendererTest.ConnectionCloseTest);
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");

	Anything config;

	config["PersistentConnections"] = 1L;

	Context c(config);

	OStringStream response;
	r.RenderAll(response, c, ROAnything());
	assertEqual("HTTP/1.1 200 OK\r\nConnection: close\r\n", response.str());

	Anything tmpStore(c.GetTmpStore());
	tmpStore["Keep-Alive"] = 1L;

	OStringStream response2;
	r.RenderAll(response2, c, ROAnything());
	assertEqual("HTTP/1.1 200 OK\r\n", response2.str());
}

void HTTPProtocolReplyRendererTest::RequestSuccessfulReplyLine()
{
	StartTrace(HTTPProtocolReplyRendererTest.RequestSuccessfulReplyLine);
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");
	Context c;
	Anything tmpStore(c.GetTmpStore());
	tmpStore["HTTPStatus"]["HTTPVersion"] = "HTTP/1.1";
	tmpStore["HTTPStatus"]["ResponseCode"] = 200L;
	tmpStore["HTTPStatus"]["ResponseMsg"] = "OK";
	OStringStream response;
	r.RenderAll(response, c, ROAnything());
	assertEqual("HTTP/1.1 200 OK\r\nConnection: close\r\n", response.str());
	OStringStream response2;
	tmpStore["HTTPStatus"]["HTTPVersion"] = "HTTP/1.0";
	tmpStore["HTTPStatus"]["ResponseMsg"] = "OKEYDOKEY";
	r.RenderAll(response2, c, ROAnything());
	assertEqual("HTTP/1.0 200 OKEYDOKEY\r\n", response2.str());
}

void HTTPProtocolReplyRendererTest::DefaultReasonPhraseTest()
{
	StartTrace(HTTPProtocolReplyRendererTest.DefaultReasonPhraseTest);

	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");

	assertEqual("OK", r.DefaultReasonPhrase(200L));

	assertEqual("Unknown Error", r.DefaultReasonPhrase(555));
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPProtocolReplyRendererTest::suite ()
{
	StartTrace(HTTPProtocolReplyRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPProtocolReplyRendererTest, testCase));
	testSuite->addTest (NEW_CASE(HTTPProtocolReplyRendererTest, RequestSuccessfulReplyLine));
	testSuite->addTest (NEW_CASE(HTTPProtocolReplyRendererTest, ReasonLessReplyLine));
	testSuite->addTest (NEW_CASE(HTTPProtocolReplyRendererTest, ConnectionCloseTest));
	testSuite->addTest (NEW_CASE(HTTPProtocolReplyRendererTest, ReasonLessErrorReplyLine));
	testSuite->addTest (NEW_CASE(HTTPProtocolReplyRendererTest, DefaultReasonPhraseTest));

	return testSuite;
}
