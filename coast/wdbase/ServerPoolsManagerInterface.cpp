/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServerPoolsManagerInterface.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "Threads.h"
#include "Registry.h"
#include "Dbg.h"

//---- ServerPoolsManagerInterfacesModule -----------------------------------------------------------
RegisterModule(ServerPoolsManagerInterfacesModule);

ServerPoolsManagerInterfacesModule::ServerPoolsManagerInterfacesModule(const char *name)
	: WDModule(name)
{
}

ServerPoolsManagerInterfacesModule::~ServerPoolsManagerInterfacesModule()
{
}

bool ServerPoolsManagerInterfacesModule::Init(const ROAnything config)
{
	if (config.IsDefined("ServerPoolsManagerInterfaces")) {
		AliasInstaller ai("ServerPoolsManagerInterface");
		return RegisterableObject::Install(config["ServerPoolsManagerInterfaces"], "ServerPoolsManagerInterface", &ai);
	}
	return false;
}

bool ServerPoolsManagerInterfacesModule::ResetFinis(const ROAnything )
{
	AliasTerminator at("ServerPoolsManagerInterface");
	return RegisterableObject::ResetTerminate("ServerPoolsManagerInterface", &at);
}

bool ServerPoolsManagerInterfacesModule::Finis()
{
	return StdFinis("ServerPoolsManagerInterface", "ServerPoolsManagerInterfaces");
}

class CountEntry
{
public:
	CountEntry(Mutex &aMutex, Condition &aCondition, long &lCount)
		: fMutex(aMutex), fCondition(aCondition), fCount(lCount) {
		StartTrace(CountEntry.CountEntry);
		MutexEntry sme(fMutex);
		sme.Use();
		++fCount;
		Trace("count:" << fCount);
	}
	~CountEntry() {
		StartTrace(CountEntry.~CountEntry);
		MutexEntry sme(fMutex);
		sme.Use();
		if (fCount > 0L) {
			--fCount;
		}
		Trace("count:" << fCount);
		Trace("signalling condition");
		fCondition.Signal();
	}
	void Use() const {};

private:
	Mutex	&fMutex;
	Condition &fCondition;
	long		&fCount;
};

//---- ServerPoolsManagerInterface ------------------------------------------------------------------
ServerPoolsManagerInterface::ServerPoolsManagerInterface(const char *ServerThreadPoolsManagerName)
	: ConfNamedObject(ServerThreadPoolsManagerName)
	, fReady(false)
	, fbInTermination(false)
	, fMutex("ServerPoolsManagerInterface")
	, fCount(0L)
{
	StartTrace(ServerPoolsManagerInterface.Ctor);
}

ServerPoolsManagerInterface::~ServerPoolsManagerInterface()
{
	StartTrace(ServerPoolsManagerInterface.Dtor);
	fbInTermination = true;
	// wake up potential blockers in IsReady and let them go out
	// wait until we get the fMutex back
	{
		MutexEntry me(fMutex);
		Trace("count:" << fCount);
		while ( fCount > 0 ) {
			fCond.Signal();
			fCond.Wait(fMutex);
			Trace("count:" << fCount);
		}
	}
}

void ServerPoolsManagerInterface::SetReady(bool ready)
{
	MutexEntry me(fMutex);
	fReady = ready;
	fCond.BroadCast();
}

bool ServerPoolsManagerInterface::IsReady(bool ready, long timeout)
{
	StartTrace(ServerPoolsManagerInterface.IsReady);
	bool bRet = false;
	if ( !fbInTermination ) {
		CountEntry ce(fMutex, fCond, fCount);
		ce.Use();
		MutexEntry me(fMutex);
		Trace("waiting on ready=" << (ready ? "true" : "false" ) << " fReady=" << (fReady ? "true" : "false" ));
		while ( !fbInTermination && ( fReady != ready ) ) {
			long ret = fCond.TimedWait(fMutex, timeout);
			if ( ret == TIMEOUTCODE ) {
				break;
			}
			Trace("ready=" << (ready ? "true" : "false" ) << " fReady=" << (fReady ? "true" : "false" ));
		}
		bRet = (fReady == ready);
	}
	return bRet;
}

//---- registry interface
RegCacheImpl(ServerPoolsManagerInterface);	// FindServerPoolsManagerInterface()
