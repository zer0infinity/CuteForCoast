/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "LdapDataAccessTestAction.h"
#include "Tracer.h"
RegisterAction(LdapDataAccessTestAction);

bool LdapDataAccessTestAction::DoExecAction(String &action, Context &ctx, const ROAnything &config) {
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(LdapDataAccessTestAction.DoExecAction);
	TraceAny(config, "the config");
	return CallDataAccessAction::DoExecAction(action, ctx, config);
}
