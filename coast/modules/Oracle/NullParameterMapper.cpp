/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "NullParameterMapper.h"
#include "Tracer.h"
#include "AnyIterators.h"

RegisterParameterMapper(NullParameterMapper);

NullParameterMapper::NullParameterMapper(const char *name) :
		ParameterMapper(name) {
	StatTrace(NullParameterMapper.Ctor, name, coast::storage::Current());
}

IFAObject *NullParameterMapper::Clone(Allocator *a) const {
	StatTrace(NullParameterMapper.Clone, fName, coast::storage::Current());
	return new (a) NullParameterMapper(fName);
}

bool NullParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(NullParameterMapper.DoGetAny, NotNull(key));
	if (ParameterMapper::DoGetAny(key, value, ctx, script)) {
		Trace("value before replacement [" << (value.IsNull()?String("*"):value.AsString()) << "]");
		ROAnything roaTreatAsNullValues = Lookup("TreatAsNull");
		if (!value.IsNull() && !roaTreatAsNullValues.IsNull()) {
			ROAnything roaTestValue;
			AnyExtensions::Iterator<ROAnything> nullValIterator(roaTreatAsNullValues);
			while (nullValIterator.Next(roaTestValue)) {
				Trace("testing against listvalue [" << roaTestValue.AsString() << "]");
				if (roaTestValue.IsEqual(value)) {
					Trace("value matched in list [" << value.AsString() << "]");
					value = Anything(value.GetAllocator());
					break;
				}
			}
		}
		Trace("returning true and value [" << (value.IsNull()?String("*"):value.AsString()) << "]");
		return true;
	}
	Trace("returning false and value [" << (value.IsNull()?String("*"):value.AsString()) << "]");
	return false;
}
