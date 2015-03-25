/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CallRendererTest_H
#define _CallRendererTest_H

#include "RendererTest.h"

//! test the new Call (aka Lambda) Renderer
/*!
 Testcase to create test-first the proposed CallRenderer.
 This renderer should allow the reuse of complex renderer specifications
 by passing parameters to the renderer via the context, thus local
 throughout a page
 this may contain <B>HTML-Tags</B>
 */
class CallRendererTest: public RendererTest {
public:
	CallRendererTest(TString tstrName) :
		RendererTest(tstrName) {
	}
	//! builds up a suite of testcases for this test
	static Test *suite();

	//! Call a CallRenderer with an empty config
	void EmptyCallTest();
	//! Call a CallRenderer with an simple lookup config
	void LookupCallTest();
};

#endif
