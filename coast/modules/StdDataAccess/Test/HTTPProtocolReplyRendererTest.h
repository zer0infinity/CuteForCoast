/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPProtocolReplyRendererTest_H
#define _HTTPProtocolReplyRendererTest_H

#include "TestCase.h"

//---- HTTPProtocolReplyRendererTest ----------------------------------------------------------
//! TestCases description
class HTTPProtocolReplyRendererTest: public testframework::TestCase {
public:
	//! TestCase constructor
	//! \param name name of the test
	HTTPProtocolReplyRendererTest(TString tstrName) :
		TestCaseType(tstrName) {
	}

	//! builds up a suite of testcases for this test
	static Test *suite();

	//! check if a success HTTP reply can be rendered
	void RequestSuccessfulReplyLine();
	//! check for a non-rendered reason phrase
	void ReasonLessReplyLine();
	//! check for a non-rendered reason phrase in case of a error code
	void ReasonLessErrorReplyLine();

	//! check for handling of "Connection: close"
	void ConnectionCloseTest();

	//! check the status code to reason phrase mapper
	void DefaultReasonPhraseTest();
};

#endif
