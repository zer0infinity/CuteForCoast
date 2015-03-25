/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "ConfigurableStoreResultMapper.h"
#include "AnythingUtils.h"

//---- ConfigurableStoreResultMapper ------------------------------------------------------------------
RegisterResultMapper(ConfigurableStoreResultMapper);

void ConfigurableStoreResultMapper::DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx) {
	StartTrace1(ConfigurableStoreResultMapper.DoGetDestinationAny, NotNull(key));
	String path = GetDestinationSlot(ctx), kPrefix(key);
	if (path.Length() > 0 && kPrefix.Length()) {
		path.Append(getDelim());
	}
	path.Append(kPrefix);

	Anything anyConfig;
	anyConfig["Store"] = Lookup("Store", "TmpStore");
	anyConfig["Slot"] = path;
	anyConfig["Delim"] = String().Append(getDelim());
	anyConfig["IndexDelim"] = String().Append(getIndexDelim());

	TraceAny(anyConfig, "StoreFinderConfig");
	StoreFinder::Operate(ctx, targetAny, anyConfig);
}
