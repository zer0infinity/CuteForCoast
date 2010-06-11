/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Role.h"

//--- standard modules used ----------------------------------------------------
#include "Session.h"
#include "Registry.h"
#include "Renderer.h"
#include "SystemLog.h"
#include "Dbg.h"
#include "AnyIterators.h"
#include <cstring>

//---- RolesModule -----------------------------------------------------------
RegisterModule(RolesModule);

RolesModule::RolesModule(const char *name)
	: WDModule(name)
{
}

RolesModule::~RolesModule()
{
}

bool RolesModule::Init(const ROAnything config)
{
	if (config.IsDefined("Roles")) {
		HierarchyInstaller hi("Role");
		return RegisterableObject::Install(config["Roles"], "Role", &hi);
	}
	return false;
}

bool RolesModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at("Role");
	return RegisterableObject::ResetTerminate("Role", &at);
}

bool RolesModule::Finis()
{
	return StdFinis("Role", "Roles");
}

//---- Role --------------------------------------------------------------------------
RegisterRole(Role);

Role::Role(const char *name) : HierarchConfNamed(name)
{
}

Role::~Role()
{
	StatTrace(Role.Misc, "~Role: <" << fName << ">", Storage::Current());
}

IFAObject *Role::Clone() const
{
	return new Role(fName);
}

bool Role::Init(Context &)
{
	StatTrace(Role.Init, fName << ": abstract - nothing to init, returning true", Storage::Current());
	return true;
}

void Role::Finis(Session &, Role *)
{
	StatTrace(Role.Finis, fName << ": abstract - nothing to do", Storage::Current());
}

bool Role::Synchronize(Context &) const
{
	StatTrace(Role.Synchronize, fName << ": abstract returning true", Storage::Current());
	return true;
}

bool Role::CheckLevel(const String &queryRoleName) const
{
	StartTrace1(Role.CheckLevel, "my role name [" << fName << "], query-role to check for [" << queryRoleName << "]");
	bool bLevelOk = false;
	if ( !( bLevelOk = fName.IsEqual(queryRoleName) ) ) {
#if defined(COAST_TRACE)
// the following code would be nice to see role relations
// -> but we need to re-authenticate as soon as the role names are different
		// names are not equal, check for their relation
		long lThisLevel = GetRoleLevel( this );
		Role *pQRole = Role::FindRole(queryRoleName);
		long lQRoleLevel = GetRoleLevel( pQRole );
		Trace("my role level:" << lThisLevel << " query-role level:" << lQRoleLevel);
		// if the roles are on the same level, they cannot be related
		if ( lThisLevel != lQRoleLevel ) {
			if ( lQRoleLevel > lThisLevel ) {
				// check if current role is a parent of the query-role
				Role *pRole = pQRole;
				String strRoleName;
				while ( !bLevelOk && pRole && ( pRole = (Role *)pRole->GetSuper() ) && pRole ) {
					pRole->GetName(strRoleName);
					bLevelOk = strRoleName.IsEqual(fName);
					Trace("role [" << strRoleName << "]" << (bLevelOk ? " is parent" : ""));
				}
			}
		}
		bLevelOk = false;
#endif
	}
	return bLevelOk;
}

long Role::GetRoleLevel(const Role *pRole) const
{
	StartTrace(Role.GetRoleLevel);
	long lLevel = 0;
	String strRoleName;
	if ( pRole ) {
		pRole->GetName(strRoleName);
	}
	while ( pRole && ( pRole = (Role *)pRole->GetSuper() ) ) {
		++lLevel;
	}
	Trace("Role <" << strRoleName << "> has Level " << lLevel);
	return lLevel;
}

long Role::GetTimeout() const {
	return Lookup("SessionTimeout", 60L);
}

void Role::PrepareTmpStore(Context &c)
{
	// now extract state from URL into TmpStore
	StartTrace1(Role.PrepareTmpStore, fName << ":");
	ROAnything stateFullList;

	if (Lookup("StateFull", stateFullList)) {
		TraceAny(stateFullList, "State full list");

		Anything query = c.GetQuery();
		Anything fields = query["fields"];
		Anything tmpStore = c.GetTmpStore();

		for (int i = 0, szf = stateFullList.GetSize(); i < szf; ++i) {
			const char *stateName = stateFullList[i].AsCharPtr(0);

			if (stateName) {
				//--- don't overwrite entries already there
				bool stateAlreadyDefined = tmpStore.IsDefined(stateName);
				stateAlreadyDefined = stateAlreadyDefined && (strlen(tmpStore[stateName].AsCharPtr("")) > 0);

				if ( !stateAlreadyDefined ) {
					if (query.IsDefined(stateName) && !query[stateName].IsNull()) {
						tmpStore[stateName] = query[stateName];
					}
					//Implicit value overriding
					if (fields.IsDefined(stateName) && !fields[stateName].IsNull()) {
						tmpStore[stateName] = fields[stateName];
					}
				}
			}
		}
		TraceAny(tmpStore, "Tempstore after");
	}
}

bool Role::GetNewPageName(Context &c, String &transition, String &pagename) const {
	// this method implements the default page resolving mechanism
	// it searches a new page through a lookup in the action/page map
	// table, defined in the role's *.any file

	StartTrace1(Role.GetNewPageName, "Rolename <" << fName << "> currentpage= <" << pagename << ">, transition= <" << transition << ">");

	if (IsStayOnSamePageToken(transition)) {
		return true;
	}

	ROAnything entry;
	if (GetNextActionConfig(entry, transition, pagename)) {
		// now look for new map entries consisting of page and action
		const char *newpagename = 0;
		if (entry.IsDefined("Page")) {
			newpagename = entry["Page"].AsCharPtr(0);
		} else {
			newpagename = entry[0L].AsCharPtr(0);
		}
		Trace("returning newPageName: <" << NotNull(newpagename) << ">");
		if (newpagename) {
			pagename = newpagename;
			if (entry.GetSize() > 1) {
				if (entry.IsDefined("Action")) {
					transition = entry["Action"].AsCharPtr(0);
				} else {
					transition = entry[1L].AsCharPtr(0);
				}
			} else {
				transition = "PreprocessAction";
			}
			return true;
		}
	}
	return false;
}

bool Role::GetNextActionConfig(ROAnything &entry, String &transition, String &pagename) const
{
	StartTrace(Role.GetNextActionConfig);
	// get the action page map from the role's configuration file
	// map inheritance is not needed explicitly but realized using hierarchic configured objects
	String index("Map.");
	index << pagename << "." << transition;
	Trace("trying [" << index << "]");
	if (!Lookup(index, entry)) {
		String defaultindex("Map.Default.");
		defaultindex  << transition;
		Trace("trying [" << defaultindex << "]");
		Lookup(defaultindex, entry);
	}
	return !entry.IsNull();
}

bool Role::IsStayOnSamePageToken(String &transition) const
{
	StartTrace1(Role.IsStayOnSamePageToken, "checking token <" << transition << ">");
	ROAnything tokens;
	bool bIsStayToken(false);
	if ( transition == "StayOnSamePage" || ( Lookup("StayOnSamePageTokens", tokens) && tokens.Contains(transition) ) ) {
		transition = "PreprocessAction";
		bIsStayToken=true;
	}
	Trace("resulting token <" << transition << "> is " << (bIsStayToken?"":"not ") << "to StayOnSamePage");
	return bIsStayToken;
}

void Role::CollectLinkState(Anything &stateIn, Context &c)
{
	StartTrace(Role.CollectLinkState);

	// copy selected fields from TmpStore into Link
	// the symmetric operation is in GetNewPageName
	ROAnything stateFullList, roaStatefulEntry;
	if (Lookup("StateFull", stateFullList)) {
		Anything tmpStore = c.GetTmpStore();
		AnyExtensions::Iterator<ROAnything> statefulIter(stateFullList);
		String strStateName(32L);
		while (statefulIter.Next(roaStatefulEntry)) {
			strStateName = roaStatefulEntry.AsString();
			if (strStateName.Length()) {
				if (!stateIn.IsDefined(strStateName) && tmpStore.IsDefined(strStateName)) {
					Trace("copying content of TmpStore[\"" << strStateName << "\"]");
					stateIn[strStateName] = tmpStore[strStateName];
				}
			}
		}
	}
	stateIn["role"] = fName;
	Session *s = c.GetSession();
	if (s) {
		s->CollectLinkState(stateIn, c);
	}
}

bool Role::TransitionAlwaysOK(const String &transition) const
{
	return (transition == "Logout");
}

String Role::GetRequestRoleName(Context &ctx) const
{
	String name;
	Anything query = ctx.GetQuery();
	if ( query.IsDefined("role") ) {
		name = query["role"].AsString(name);
		StatTrace(Role.GetRequestRoleName, "got query role <" << name << ">", Storage::Current());
	} else {
		name = GetDefaultRoleName(ctx);
		StatTrace(Role.GetRequestRoleName, "got default role <" << name << ">", Storage::Current());
	}
	return name;
}

// this method verifies the authentication level of role
// if everything is ok it let's the subclass verify the
// detailed parameters of the query in DoVerify
bool Role::Verify(Context &c, String &transition, String &pagename) const
{
	StartTrace1(Role.Verify, "Rolename <" << fName << "> currentpage= <" << pagename << ">, transition= <" << transition << ">");
	// if the action is always possible (e.g. logout) no further checking
	// is necessary
	if (TransitionAlwaysOK(transition)) {
		return true;
	}

	// we check the role level by role name
	// if no role is defined in the query we use the default defined in slot DefaultRole or as last resort, use the Role base class
	String name = GetRequestRoleName(c);

	// check the level of the role it is defined in the config
	// assuming some levels of roles (e.g. Guest < Customer < PaymentCustomer)
	if ( CheckLevel(name) ) {
		// We have the right role level
		// let's check the query parameters
		Trace("role level is OK");
		return DoVerify(c, transition, pagename);
	}

	// We have a level which is too low for this
	// query, we can't proceed without authentication
	Trace("role level not equal");
	return false;
}

bool Role::DoVerify(Context &, String &, String &) const
{
	StatTrace(Role.DoVerify, "of role <" << fName << "> is not defined, returning true", Storage::Current());
	return true;
}

RegCacheImpl(Role);	// FindRole()

String Role::GetDefaultRoleName(Context &ctx) {
	String ret;
	ROAnything rspec;
	if (ctx.Lookup("DefaultRole", rspec)) {
		Renderer::RenderOnString(ret, ctx, rspec);
	} else {
		ret = "Role";
	}
	return ret;
}

Role *Role::FindRoleWithDefault(const char *role_name, Context &ctx, const char *dflt) {
	Role *ret = Role::FindRole(role_name);
	if (ret == 0) {
		String msg;
		msg << "<" << ctx.GetSessionId() << "> " << "no valid role <" << role_name << "> found; using <" << dflt << ">";
		SystemLog::Info(msg);
		ret = Role::FindRole(dflt);
		if (ret == 0) {
			msg.Trim(0);
			msg << "<" << ctx.GetSessionId() << "> " << "Role <" << role_name << "> and Role <" << dflt << "> not registered -- Danger";
			SYSERROR(msg);
			// fake a new role...as a last resort before crash
			Assert(ret != 0); // fail in Debug mode
			::abort();
		}
	}
	return ret;
}
