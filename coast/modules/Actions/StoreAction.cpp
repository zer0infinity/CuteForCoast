/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "StoreAction.h"
#include "Renderer.h"
#include "AnythingUtils.h"

RegisterAction(StoreAction);

bool StoreAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(StoreAction.DoExecAction);

	ROAnything destConfig;
	ROAnything valueConfig;
	if (!(config.LookupPath(destConfig, "Destination") && config.LookupPath(valueConfig, "Value"))) {
		return false;
	}

	TraceAny(valueConfig, "Value config");
	Anything value;
	if (config["DoRender"].AsBool(true)) {
		String renderedValue;
		Renderer::RenderOnString(renderedValue, ctx, valueConfig);
		Trace("Rendered value: [" << renderedValue << "]");
		value = renderedValue;
	} else {
		value = valueConfig.DeepClone();
	}
	TraceAny(destConfig, "Destination");

	StorePutter::Operate(value, ctx, destConfig);
	return true;
}
