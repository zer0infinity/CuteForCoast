/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Mapper.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Registry.h"
#include "Dbg.h"
#include "Timers.h"

//--- c-library modules used ---------------------------------------------------

//---- MappersModule -----------------------------------------------------------
RegisterModule(MappersModule);

MappersModule::MappersModule(const char *name) : WDModule(name)
{
}

MappersModule::~MappersModule()
{
}

bool MappersModule::Init(const Anything &config)
{
	StartTrace(MappersModule.Init);
	// installation of different mapping objects for the different backend objects
	if (config.IsDefined("Mappers")) {
		Anything mappers(config["Mappers"]);

		AliasInstaller ai1("ParameterMapper");
		bool ret = RegisterableObject::Install(mappers["Input"], "ParameterMapper", &ai1);

		AliasInstaller ai2("ResultMapper");
		return RegisterableObject::Install(mappers["Output"], "ResultMapper", &ai2) && ret;
	}
	return false;
}

bool MappersModule::ResetFinis(const Anything &config)
{
	// installation of different mapping objects for the different backend objects
	AliasTerminator at1("ParameterMapper");
	bool ret = RegisterableObject::ResetTerminate("ParameterMapper", &at1);

	AliasTerminator at2("ResultMapper");
	return RegisterableObject::ResetTerminate("ResultMapper", &at2) && ret;
}

bool MappersModule::Finis()
{
	bool retVal;
	retVal = StdFinis("ParameterMapper", "ParameterMapper");

	return StdFinis("ResultMapper", "ResultMapper") && retVal;
}

//---- ParameterMapper ------------------------------------------------------------------------
RegisterParameterMapper(ParameterMapper);
RegisterInputMapperAlias(Mapper, ParameterMapper);
RegCacheImpl(ParameterMapper);

ParameterMapper::ParameterMapper(const char *name) : ConfNamedObject(name)
{
}

ParameterMapper::~ParameterMapper()
{
}

IFAObject *ParameterMapper::Clone() const
{
	return new ParameterMapper(fName);
}
//--- Registry -----

ParameterMapper *ParameterMapper::FindInputMapper(const char *name)
{
	return FindParameterMapper(name);
}

ResultMapper *ResultMapper::FindOutputMapper(const char *name)
{
	return FindResultMapper(name);
}

bool ParameterMapper::DoLoadConfig(const char *category)
{
	StartTrace(ParameterMapper.DoLoadConfig);
	Trace("category: " << category << " fName: " << fName);

	if ( ConfNamedObject::DoLoadConfig(category) && fConfig.IsDefined(fName) ) {
		Trace("Meta-file for " << category << " found. Extracting config for " << fName);
		// trx impls use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		TraceAny(fConfig, "Extracted fConfig: (Returning true)");
		return true;
	}
	fConfig = Anything();
	Trace("No config found. Returning false.");
	return false;
}

ROAnything ParameterMapper::DoSelectScript(const char *key, ROAnything script)
{
	StartTrace1(ParameterMapper.DoSelectScript, "getting key [" << NotNull(key) << "]");
	return script[key];
}

bool ParameterMapper::DoGetConfigName(const char *category, const char *, String &configFileName)
{
	StartTrace(ParameterMapper.DoGetConfigName);

	if (String("ParameterMapper").IsEqual(category)) {
		configFileName = "InputMapperMeta";    // keep legacy name to avoid config problems
	} else {
		configFileName = category;
		configFileName << "Meta";
	}
	return true;
}

bool ParameterMapper::Get(const char *key, int &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , int &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = (int)anyValue[0L].AsLong(value);
			return true;
		}
	}
	return false;
}

#if !defined(BOOL_NOT_SUPPORTED)
bool ParameterMapper::Get(const char *key, bool &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , bool &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			TraceAny(anyValue, "value-any");
			value = anyValue[0L].AsBool(value);
			Trace("returning: " << value);
			return true;
		}
	}
	return false;
}
#endif

bool ParameterMapper::Get(const char *key, long &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , long &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			TraceAny(anyValue, "value-any");
			value = anyValue[0L].AsLong();
			Trace("returning: " << value);
			return true;
		}
		Trace("value not found, returning false");
	}
	return false;
}

bool ParameterMapper::Get(const char *key, float &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , float &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = (float)anyValue[0L].AsDouble(value);
			return true;
		}
	}
	return false;
}

bool ParameterMapper::Get(const char *key, double &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , double &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = anyValue[0L].AsDouble(value);
			return true;
		}
	}
	return false;
}

bool ParameterMapper::Get(const char *key, String &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , String &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = anyValue[0L].AsCharPtr();
			return true;
		}
	}
	return false;
}

bool ParameterMapper::Get(const char *key, Anything &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx)");
	DAAccessTimer(ParameterMapper.Get, key << " (Anything)", ctx);

	Anything anyValue;
	if (DoGetAny(key, anyValue, ctx, DoSelectScript(key, fConfig))) {
		value = anyValue;
		return true;
	}
	return false;
}

bool ParameterMapper::Get(const char *key, ostream &os, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , ostream &os, Context &ctx)");
	DAAccessTimer(ParameterMapper.Get, key << " (stream)", ctx);

	return DoGetStream(key, os, ctx, DoSelectScript(key, fConfig));
}

// convenience method
void ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(Anything &var, ROAnything theValue)
{
	StartTrace(ParameterMapper.PlaceIntoAnyOrAppendIfNotEmpty);
	if (var.IsNull()) {
		var = theValue.DeepClone();
		TraceAny(theValue, "Placing value into Anything...");
	} else {
		var.Append(theValue.DeepClone());
		TraceAny(theValue, "Appending value to given Anything");
	}
}

bool ParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script)
{
	StartTrace1(ParameterMapper.DoGetAny, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx, ROAnything script)");

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		return DoFinalGetAny(key, value, ctx);
	}

	if (script.GetType() != Anything::eArray) {
		// we found a simple value, append it, Append because of scripting
		Trace("Script is a simple value:" << script.AsString());
		PlaceIntoAnyOrAppendIfNotEmpty(value, script);
		return true;
	}

	TraceAny(script, "Got a script. Starting interpretation foreach slot...");
	// now for the scripting case, similar to Renderers
	// interpret as long as you get return values. stop if you don't
	bool retval = true;
	for (long i = 0; retval && i < script.GetSize(); i++) {
		String slotname(script.SlotName(i));
		ParameterMapper *m;
		if (slotname.Length() <= 0) {
			Trace("Anonymous slot, call myself again with script[" << i << "]");
			retval = DoGetAny(key, value, ctx, script[i]);
		} else if ((m = ParameterMapper::FindParameterMapper(slotname))) {
			Trace("Slotname equals mapper: " << slotname);
			if (script[i].IsNull()) {
				// no more config in script, use original mappers config
				// fallback to orignal mapping with m's config
				Trace("Slotval is null. Calling " << slotname << " with it's default config...");
				retval = m->DoGetAny(key, value, ctx, m->DoSelectScript(key, m->fConfig));
			} else {
				Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
				retval = m->DoGetAny(key, value, ctx, m->DoSelectScript(key, script[i]));
			}
		} else {
			Trace("Slotname " << slotname << " is not a mapper (not found).");
			Trace("Using slot as new key and call myself again with script[" << i << "]");
			retval = DoGetAny(slotname, value, ctx, script[i]);
		}
	}
	return retval;
}
bool ParameterMapper::DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything script)
{
	StartTrace1(ParameterMapper.DoGetStream, "( \"" << NotNull(key) << "\" , ostream &os, Context &ctx, ROAnything script)");

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		return DoFinalGetStream(key, os, ctx);
	}

	if (script.GetType() != Anything::eArray) {
		// we found a simple value, append it, Append because of scripting
		Trace("Script is a simple value, write it on stream...");
		os << script.AsCharPtr();
		return true;
	}

	// now for the scripting case, similar to Renderers
	TraceAny(script, "Got a script. Starting interpretation foreach slot...");
	bool retval = true;
	for (long i = 0; retval && i < script.GetSize(); i++) {
		String slotname(script.SlotName(i));
		ParameterMapper *m;
		if (slotname.Length() <= 0) {
			Trace("Anonymous slot, call myself again with script[" << i << "]");
			retval = DoGetStream(key, os, ctx, script[i]);
		} else if ((m = ParameterMapper::FindParameterMapper(slotname))) {
			Trace("Slotname equals mapper: " << slotname);
			if (script[i].IsNull()) {
				// no more config in script, use original mappers config
				// fallback to orignal mapping with m's config
				Trace("Slotval is null. Calling " << slotname << " with it's default config...");
				retval = m->DoGetStream(key, os, ctx, m->DoSelectScript(key, m->fConfig));
			} else {
				Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
				retval = m->DoGetStream(key, os, ctx, m->DoSelectScript(key, script[i]));
			}
		} else {
			Trace("Slotname " << slotname << " is not a mapper (not found).");
			Trace("Using slot as new key and call myself again with script[" << i << "]");
			retval = DoGetStream(slotname, os, ctx, script[i]);
		}
	}
	return retval;
}

bool ParameterMapper::DoFinalGetAny(const char *key, Anything &value, Context &ctx)
{
	StartTrace1(ParameterMapper.DoFinalGetAny, NotNull(key));

	ROAnything ctxValue;
	String sourceSlot = DoGetSourceSlot(ctx);
	sourceSlot = sourceSlot.IsEqual("") ? String(key) : (sourceSlot << "." << key);

	if (ctx.Lookup(sourceSlot, ctxValue) && !ctxValue.IsNull() ) {
		Trace("Found key [" << sourceSlot << "] in context...");
		PlaceIntoAnyOrAppendIfNotEmpty(value, ctxValue);
		return true;
	}
	Trace("Key [" << sourceSlot << "] not found in context. Failing.");
	return false;
}

static void PlaceAnyOnStream(ostream &os, ROAnything value)
{
	if (value.GetType() == Anything::eArray) {
		os << value;
	} else {
		os << value.AsCharPtr("");
	}
}

bool ParameterMapper::DoFinalGetStream(const char *key, ostream &os, Context &ctx)
{
	StartTrace1(ParameterMapper.DoFinalGetStream, NotNull(key));

	ROAnything ctxValue;
	String sourceSlot = DoGetSourceSlot(ctx);
	sourceSlot = sourceSlot.IsEqual("") ? String(key) : (sourceSlot << "." << key);

	if (ctx.Lookup(sourceSlot, ctxValue) && !ctxValue.IsNull() ) {
		Trace("Found key [" << sourceSlot << "] in context with value [" << ctxValue.AsString() << "]");
		Trace("Putting value onto return stream...");
		PlaceAnyOnStream(os, ctxValue);
		return true;
	}
	Trace("Key [" << sourceSlot << "] not found in context. Failing.");
	return false;
}

String ParameterMapper::DoGetSourceSlot(Context &ctx)
{
	ROAnything slotname;

	String slotnamename(fName, Storage::Current());
	slotnamename.Append(".SourceSlot");
	return (Lookup("SourceSlot", slotname) || ctx.Lookup(slotnamename, slotname)) ? slotname.AsCharPtr() : "";
}

//---- EagerParameterMapper ------------------------------------------------
RegisterParameterMapper(EagerParameterMapper);

EagerParameterMapper::EagerParameterMapper(const char *name, ROAnything config) : ParameterMapper(name)
{
	fConfig = config;
}

//---- ResultMapper ----------------------------------------------------------------
RegisterResultMapper(ResultMapper);
RegisterOutputMapperAlias(Mapper, ResultMapper);
RegCacheImpl(ResultMapper);	// FindResultMapper()

ROAnything ResultMapper::DoSelectScript(const char *key, ROAnything script)
{
	StartTrace1(ResultMapper.DoSelectScript, "getting key [" << NotNull(key) << "]");
	return script[key];
}

bool ResultMapper::DoGetConfigName(const char *category, const char *objName, String &configFileName)
{
	StartTrace(ResultMapper.DoGetConfigName);

	if (String("ResultMapper").IsEqual(category)) {
		configFileName = "OutputMapperMeta";    // keep legacy name to avoid config problems
	} else {
		configFileName = (String(category) << "Meta");
	}
	return true;
}

bool ResultMapper::DoLoadConfig(const char *category)
{
	StartTrace(ResultMapper.DoLoadConfig);
	Trace("category: " << category << " fName: " << fName);

	if ( ConfNamedObject::DoLoadConfig(category) && fConfig.IsDefined(fName) ) {
		TraceAny(fConfig, "fConfig before: ");
		// trx impls use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		TraceAny(fConfig, "new fConfig:");
		return true;
	}
	fConfig = Anything();
	Trace("returning false");
	return false;
}

#if !defined(BOOL_NOT_SUPPORTED)
bool ResultMapper::Put(const char *key, bool value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}
#endif

bool ResultMapper::Put(const char *key, int value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}
bool ResultMapper::Put(const char *key, long value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, float value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, double value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, String value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, Anything value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	DAAccessTimer(ResultMapper.Put, key, ctx);
	return DoPutAny(key, value, ctx, DoSelectScript(key, fConfig));
}

bool ResultMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script)
{
	StartTrace1(ResultMapper.DoPutAny, NotNull(key));

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		return DoFinalPutAny(key, value, ctx);
	}

	if (script.GetType() != Anything::eArray) {
		// we found a simple value in script use it as a new key in final put
		Trace("Script is a simple value:" << script.AsString());
		return DoFinalPutAny(script.AsCharPtr(key), value, ctx);
	}

	// now for the scripting case, similar to Renderers
	TraceAny(script, "Got a script. Starting interpretation foreach slot...");
	// force setting a possibly given DestinationSlot from config into the context
	// -> this allows sub-mappers, eg executed in script mode, to put into the same slot if not overridden again
	String path = DoGetDestinationSlot(ctx);

	bool retval = true;
	for (long i = 0; retval && i < script.GetSize(); i++) {
		String slotname(script.SlotName(i));
		ResultMapper *m;
		if (slotname.Length() <= 0) {
			Trace("Anonymous slot, call myself again with script[" << i << "]");
			retval = DoPutAny(key, value, ctx, script[i]);
		} else if ((m = ResultMapper::FindResultMapper(slotname))) {
			Trace("Slotname equals mapper: " << slotname);
			if (script[i].IsNull()) {
				// fallback to mappers original config
				Trace("Slotval is null. Calling " << slotname << " with it's default config...");
				retval = m->DoPutAny(key, value, ctx, m->DoSelectScript(key, m->fConfig));
			} else {
				Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
				retval = m->DoPutAny(key, value, ctx, m->DoSelectScript(key, script[i]));
			}
		} else {
			Trace("Slotname " << slotname << " is not a mapper (not found).");
			Trace("Using slot as new key and call myself again with script[" << i << "]");
			retval = DoPutAny(slotname, value, ctx, script[i]);
		}
	}
	return retval;
}

bool ResultMapper::Put(const char *key, istream &is, Context &ctx)
{
	StartTrace1(ResultMapper.Put, NotNull(key));
	return DoPutStream(key, is, ctx, DoSelectScript(key, fConfig));
}

bool ResultMapper::DoPutStream(const char *key, istream &is, Context &ctx, ROAnything script)
{
	StartTrace1(ResultMapper.DoPutStream, NotNull(key));

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		return DoFinalPutStream(key, is, ctx);
	}

	if (script.GetType() != Anything::eArray) {
		// we found a simple value in script use it as a new key in final put
		Trace("Script is a simple value:" << script.AsString());
		return DoFinalPutStream(script.AsCharPtr(key), is, ctx);
	}
	// now for the scripting case, similar to Renderers
	TraceAny(script, "Got a script. Starting interpretation foreach slot...");
	bool retval = true;
	for (long i = 0; retval && i < script.GetSize(); i++) {
		String slotname(script.SlotName(i));
		ResultMapper *m;
		if (slotname.Length() <= 0) {
			Trace("Anonymous slot, call myself again with script[" << i << "]");
			retval = DoPutStream(key, is, ctx, script[i]);
		} else if ((m = ResultMapper::FindResultMapper(slotname))) {
			Trace("Slotname equals mapper: " << slotname);
			if (script[i].IsNull()) {
				// fallback to mappers original config
				Trace("Slotval is null. Calling " << slotname << " with it's default config...");
				retval = m->DoPutStream(key, is, ctx, m->DoSelectScript(key, m->fConfig));
			} else {
				Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]");
				retval = m->DoPutStream(key, is, ctx, m->DoSelectScript(key, script[i]));
			}
		} else {
			Trace("Slotname " << slotname << " is not a mapper (not found).");
			Trace("Using slot as new key and call myself again with script[" << i << "]");
			retval = DoPutStream(slotname, is, ctx, script[i]);
		}
	}
	return retval;
}

void ResultMapper::DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx)
{
	StartTrace1(ResultMapper.DoGetDestinationAny, NotNull(key));

	String path = DoGetDestinationSlot(ctx), kPrefix(key);
	if (path.Length() > 0 && kPrefix.Length()) {
		path << "." << kPrefix;
	} else {
		path << kPrefix;
	}
	Anything conf;
	conf["Slot"] = path;
	Trace("Path for slotfinder: " << path);

	if (path.Length() > 0) {
		SlotFinder::Operate(ctx.GetTmpStore(), targetAny, conf);
	} else {
		targetAny = ctx.GetTmpStore();
	}
}

bool ResultMapper::DoFinalPutAny(const char *key, Anything value, Context &ctx)
{
	StartTrace1(ResultMapper.DoFinalPutAny, NotNull(key));

	String kStr(key);
	if (kStr.Length() <= 0) {
		return false;    // empty key not allowed
	}

	// check if key is a path, if so, split it into prefix and key
	long idx = kStr.StrRChr('.');
	String kPrefix, kKey(key);
	if ( idx > -1 ) {
		Trace("Key is a path: " << kKey);
		kPrefix << kStr.SubString(0, idx);
		kKey = kStr.SubString(idx + 1);
		Trace("Prefix: " << kPrefix);
		Trace("Key: " << kKey);
	}
	Trace("Key for put: " << kKey);
	Trace("Initial key: " << key);

	Anything anyTarget;
	DoGetDestinationAny(kPrefix, anyTarget, ctx);
	if ( ( Lookup("AppendAnyAlways", 0L) != 0L ) || anyTarget.IsDefined(kKey) ) {
		anyTarget[kKey].Append(value);
	} else {
		anyTarget[kKey] = value;
	}
	return true;
}

bool ResultMapper::DoFinalPutStream(const char *key, istream &is, Context &ctx)
{
	StartTrace1(ResultMapper.DoFinalPutStream, NotNull(key));

	OStringStream input;
	input << is.rdbuf();
	Trace(input.str());
	return DoFinalPutAny(key, input.str(), ctx);
}

String ResultMapper::DoGetDestinationSlot(Context &ctx)
{
	StartTrace1(ResultMapper.DoGetDestinationSlot, "fName [" << fName << "]");
	ROAnything roaDest;
	String slotnamename(fName, Storage::Current());
	slotnamename.Append(".DestinationSlot");
	TraceAny(fConfig, "my configuration for Lookup");
	if ( !Lookup("DestinationSlot", roaDest) ) {
		Trace("doing ctx.Lookup on [" << slotnamename << "]");
		ctx.Lookup(slotnamename, roaDest);
	}
	String strRet(!roaDest.IsNull() ? roaDest.AsCharPtr() : "Mapper", Storage::Current());
	Trace("destination slot is [" << strRet << "]");
	ctx.GetTmpStore()["ResultMapper"]["DestinationSlot"] = strRet;
	return strRet;
}

// -------------------------- EagerResultMapper -------------------------

RegisterResultMapper(EagerResultMapper);

EagerResultMapper::EagerResultMapper(const char *name, ROAnything config) : ResultMapper(name)
{
	fConfig = config;
}

// ========================== Other Mappers =================================

// -------------------------- RootMapper -------------------------------
RegisterResultMapper(RootMapper);

// -------------------------- ConfigMapper -----------------------------
RegisterParameterMapper(ConfigMapper);

bool ConfigMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything config)
{
	StartTrace(ConfigMapper.DoGetAny);

	// step recursively through config and check, if we need to restart scripting
	EvaluateConfig(config, value, ctx);
	return true;
}

void ConfigMapper::EvaluateConfig(ROAnything config, Anything &value, Context &ctx)
{
	if (config.GetType() == Anything::eArray) {
		for (int i = 0; i < config.GetSize(); i++) {
			if ( String(config.SlotName(i)).IsEqual("MapperScript") ) {
				// must start scripting again (no key given)
				EagerParameterMapper epm("temp");
				epm.DoGetAny("", value, ctx, config[i]);
			} else {
				EvaluateConfig(config[i], value[config.SlotName(i)], ctx);
			}
		}
	} else {
		value = config.DeepClone();
	}
}
