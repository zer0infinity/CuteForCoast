/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "FirstNonEmptyRenderer.h"
RegisterRenderer(FirstNonEmptyRenderer);

void FirstNonEmptyRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(FirstNonEmptyRenderer.RenderAll);
	for (long i = 0, sz = config.GetSize(); i < sz; ++i) {
		String result;
		Renderer::RenderOnString(result, ctx, config[i]);
		TraceAny(config[i], "Trying at index [" << i << "] with result [" << result << "]");
		if (result.Length() > 0) {
			Trace("Returning result of index  [" << i << "] [" << result << "]");
			reply << result;
			return;
		}
	}
}
