/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestRenderer.h"

RegisterRenderer(TestRenderer);

TestRenderer::TestRenderer(const char *name) : Renderer(name) { }

TestRenderer::~TestRenderer() { }

void TestRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(TestRenderer.RenderAll);
	reply << "output from TestRenderer";
}
