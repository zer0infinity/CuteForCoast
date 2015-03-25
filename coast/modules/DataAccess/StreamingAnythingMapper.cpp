/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StreamingAnythingMapper.h"
#include "Timers.h"

//---- AnythingToStreamMapper ----------------------------------------------------------------
RegisterParameterMapper(AnythingToStreamMapper);

bool AnythingToStreamMapper::DoFinalGetStream(const char *key, std::ostream &os, Context &ctx)
{
	StartTrace1(AnythingToStreamMapper.DoFinalGetStream, NotNull(key));
	if ( key ) {
		// use the superclass mapper to get the anything
		Anything anyValue;
		DoFinalGetAny(key, anyValue, ctx);

		TraceAny(anyValue, "value fetched from context");

		if ( !anyValue.IsNull() ) {
			DAAccessTimer(AnythingToStreamMapper.DoFinalGetStream, "exporting to stream", ctx);
			os << anyValue << std::flush;
			TraceAny(anyValue, "written to stream:");
			return true;
		}
		Trace("Nothing written to stream, value was null.");
	}
	return false;
}

//---- StreamToAnythingMapper ----------------------------------------------------------------
RegisterResultMapper(StreamToAnythingMapper);

bool StreamToAnythingMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script)
{
	StartTrace1(StreamToAnythingMapper.DoPutStream, NotNull(key));
	Anything anyResult;
	bool importok;
	{
		DAAccessTimer(StreamToAnythingMapper.DoPutStream, "importing from stream", ctx);
		importok = anyResult.Import(is);
	}
	if ( importok ) {
		TraceAny(anyResult, "anything imported from stream:");
		importok = DoPutAny(key, anyResult, ctx, script);
	} else {
		SYSWARNING("importing Anything from stream failed!");
	}
	return importok;
}
