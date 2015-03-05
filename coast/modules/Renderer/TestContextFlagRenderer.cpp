/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestContextFlagRenderer.h"
#include "Tracer.h"

//---- TestContextFlagRenderer ---------------------------------------------------------------
RegisterRenderer(TestContextFlagRenderer);

TestContextFlagRenderer::TestContextFlagRenderer(const char *name) : Renderer(name) { }

TestContextFlagRenderer::~TestContextFlagRenderer() { }

void TestContextFlagRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(TestContextFlagRenderer.RenderAll);

	String flagName;
	Renderer::RenderOnString(flagName, ctx, config["FlagName"]);

	if (ctx.Lookup(flagName, 0L)) {
		Renderer::Render(reply, ctx, config["WhenSet"]);
	} else {
		Renderer::Render(reply, ctx, config["WhenNotSet"]);
	}
}
