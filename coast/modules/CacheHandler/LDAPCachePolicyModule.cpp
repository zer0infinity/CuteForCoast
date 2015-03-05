/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "LDAPCachePolicyModule.h"
#include "DataAccess.h"
#include "Action.h"
#include "Context.h"

//---- LdapCachePolicyModule -----------------------------------------------------------
RegisterModule(LdapCachePolicyModule);

bool LdapCachePolicyModule::Init(const ROAnything config) {
	StartTrace(LdapCachePolicyModule.Init);
	ROAnything ldapCachePolicyModuleConfig;
	config.LookupPath(ldapCachePolicyModuleConfig, "LdapCachePolicyModule");
	TraceAny(ldapCachePolicyModuleConfig, "LdapCachePolicyModule:");

	ROAnything dataAccesses(ldapCachePolicyModuleConfig["LdapDataAccess"]);
	ROAnything dataAccessActions(ldapCachePolicyModuleConfig["LdapDataAccessAction"]);
	if (dataAccesses.GetSize() == 0 && dataAccessActions.GetSize() == 0) {
		SystemLog::WriteToStderr("\tLdapCachePolicyModule::Init can't read needed configuration data.\n");
		return false;
	}
	if (InitialLoad(dataAccesses, LdapCachePolicyModule::dataaccess) == false || InitialLoad(dataAccessActions,
			LdapCachePolicyModule::action) == false) {
		return false;
	}
	String failedDataAccesses;
	CheckContractIsFulfilled(failedDataAccesses, dataAccesses);
	CheckContractIsFulfilled(failedDataAccesses, dataAccessActions);
	if (failedDataAccesses.Length() != 0) {
		SystemLog::WriteToStderr(String("\tLdapCachePolicyModule::LDAP Query: ") << failedDataAccesses << String(" returned no data.\n"));
		return false;
	}
	SystemLog::WriteToStderr("\tLdapCachePolicyModule done\n");
	return true;
}

bool LdapCachePolicyModule::InitialLoad(const ROAnything dataAccesses, LdapCachePolicyModule::EDataAccessType daType) {
	StartTrace(LdapCachePolicyModule.InitialLoad);
	LdapDataAccessLoader ldl;
	LdapActionLoader lal;
	for (int i = 0; i < dataAccesses.GetSize(); ++i) {
		String toDo(dataAccesses[i].AsString());
		if (daType == dataaccess) {
			Trace("Loading ldl with: " << toDo);
			CacheHandler::instance().Load("LdapGetter", toDo, &ldl);
		}
		if (daType == action) {
			Trace("Loading lal with: " << toDo);
			CacheHandler::instance().Load("LdapGetter", toDo, &lal);
		}
	}
	return true;
}

bool LdapCachePolicyModule::CheckContractIsFulfilled(String &failedDataAccesses, const ROAnything dataAccesses) {
	StartTrace(LdapCachePolicyModule.CheckContractIsFulfilled);
	bool ret(true);
	for (int i = 0; i < dataAccesses.GetSize(); ++i) {
		String daToDo(dataAccesses[i].AsString());
		Trace("Checking with: " << daToDo);

		ROAnything result(LdapCacheGetter::GetAll(daToDo));
		// broke contract: specified LDAP-query must return data
		if (result.IsNull()) {
			failedDataAccesses.Append(daToDo);
			failedDataAccesses.Append(" ");
			ret = false;
		}
	}
	return ret;
}

bool LdapCachePolicyModule::Finis() {
	StartTrace(LdapCachePolicyModule.Finis);
	return true;
}

Anything LdapDataAccessLoader::Load(const char *ldapDa) {
	StartTrace(LdapDataAccessLoader.Load);
	Anything theResult;
	if (String(ldapDa).Length()) {
		Context ctx;
		if (DataAccess(ldapDa).StdExec(ctx)) {
			String strResultSlot = ctx.Lookup("ResultMapper.DestinationSlot", "Mapper");
			ROAnything roaResult;
			if (ctx.Lookup(strResultSlot, roaResult)) {
				TraceAny(roaResult, "Results for [" << ldapDa << "]");
				if (roaResult["Info"]["LdapSearchFoundEntryButNoData"].AsLong() == 0) {
					theResult = roaResult["LDAPResult"].DeepClone();
				}
			}
		} else {
			String msg;
			msg << "\tLdapCachePolicyModule::Load Unable to exec LDAP query for: " << ldapDa << "\n";
			SystemLog::WriteToStderr(msg);
		}
	}
	return theResult;
}

Anything LdapActionLoader::Load(const char *ldapDaAction) {
	StartTrace(LdapActionLoader.Load);
	Anything theResult;
	if (String(ldapDaAction).Length()) {
		Context ctx;
		String transition;
		Anything config;
		// Default constructs an action config containing the name of the LdapDataAccess to execute
		// This may be overridden by the action implementing the DataAccess(es).
		config[ldapDaAction]["DataAccess"] = ldapDaAction;
		if (Action::ExecAction(transition, ctx, config)) {
			String strResultSlot = ctx.Lookup("ResultMapper.DestinationSlot", "Mapper");
			ROAnything roaResult;
			if (ctx.Lookup(strResultSlot, roaResult)) {
				TraceAny(roaResult, "Results for [" << ldapDaAction << "]");
				if (roaResult["Info"]["LdapSearchFoundEntryButNoData"].AsLong() == 0) {
					theResult = roaResult["LDAPResult"].DeepClone();
				}
			}
		} else {
			String msg;
			msg << "\tLdapCachePolicyModule::Load Unable to exec LDAP query for: " << ldapDaAction << "\n";
			SystemLog::WriteToStderr(msg);
		}
	}
	return theResult;
}

bool LdapCacheGetter::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace(LdapCacheGetter.DoLookup);
	return Get(result, fDA, key, delim, indexdelim);
}

ROAnything LdapCacheGetter::GetAll(const String &dataAccess) {
	StartTrace1(LdapCacheGetter.GetAll, dataAccess);
	return CacheHandler::instance().Get("LdapGetter", dataAccess);
}

bool LdapCacheGetter::Get(ROAnything &result, const String &dataAccess, const String &key, char sepS, char sepI) {
	StartTrace1(LdapCacheGetter.Get, key);
	bool ret = GetAll(dataAccess).LookupPath(result, key, sepS, sepI);
	TraceAny(result, "Result:");
	return ret;
}
