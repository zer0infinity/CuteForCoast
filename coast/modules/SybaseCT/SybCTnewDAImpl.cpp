/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SybCTnewDAImpl.h"
#include "SybCTnewDA.h"
#include "Timers.h"
#include "TimeStamp.h"
#include "StringStream.h"
#include "Action.h"
#include "PeriodicAction.h"

SimpleMutex SybCTnewDAImpl::fgStructureMutex("StructureMutex", coast::storage::Global());
Anything SybCTnewDAImpl::fgListOfSybCT;
Anything SybCTnewDAImpl::fgContextMessages;
CS_CONTEXT *SybCTnewDAImpl::fg_cs_context;
bool SybCTnewDAImpl::fgInitialized = false;
bool SybCTnewDAImpl::fbUseDelayedCommit = false;
PeriodicAction *SybCTnewDAImpl::fgpPeriodicAction = NULL;
Semaphore *SybCTnewDAImpl::fgpResourcesSema = NULL;

//---- SybCTnewDAImpl ----------------------------------------------------------------
RegisterDataAccessImpl(SybCTnewDAImpl);

SybCTnewDAImpl::SybCTnewDAImpl(const char *name)
	: DataAccessImpl(name)
{
	StartTrace(SybCTnewDAImpl.SybCTnewDAImpl);
}

SybCTnewDAImpl::~SybCTnewDAImpl()
{
	StartTrace(SybCTnewDAImpl.~SybCTnewDAImpl);
}

IFAObject *SybCTnewDAImpl::Clone(Allocator *a) const
{
	StartTrace(SybCTnewDAImpl.Clone);
	return new (a) SybCTnewDAImpl(fName);
}

bool SybCTnewDAImpl::Init(ROAnything config)
{
	StartTrace(SybCTnewDAImpl.Init);
	if ( !fgInitialized ) {
		ROAnything myCfg;
		String strInterfacesPathName;
		long nrOfSybCTs = 5L;
		long lCloseConnectionTimeout = 60L;
		// check if the number of SybCTs is specified in Config
		if (config.LookupPath(myCfg, "SybaseModule.SybCTnewDAImpl")) {
			nrOfSybCTs = myCfg["ParallelQueries"].AsLong(nrOfSybCTs);
			lCloseConnectionTimeout = myCfg["CloseConnectionTimeout"].AsLong(lCloseConnectionTimeout);
			strInterfacesPathName = myCfg["InterfacesPathName"].AsString();
			fbUseDelayedCommit = ( myCfg["DelayedCommit"].AsLong(0L) != 0 );
		}

		LockUnlockEntry me(fgStructureMutex);
		fgContextMessages.SetAllocator(coast::storage::Global());
		fgContextMessages = Anything();
		fgListOfSybCT.SetAllocator(coast::storage::Global());
		fgListOfSybCT = Anything();
		// SybCTnewDA::Init initializes the cs_context.  It must be done only once
		if ( SybCTnewDA::Init(&fg_cs_context, &fgContextMessages, strInterfacesPathName, nrOfSybCTs) == CS_SUCCEED ) {
			// use the semaphore to block when no more resources are available
			fgListOfSybCT["Size"] = nrOfSybCTs;
			fgpResourcesSema = new Semaphore(nrOfSybCTs);
			String server, user;
			for ( long i = 0; i < nrOfSybCTs; ++i ) {
				SybCTnewDA *pCT = new (coast::storage::Global()) SybCTnewDA(fg_cs_context);
				IntDoPutbackConnection(pCT, false, server, user);
			}
			if ( !fgpPeriodicAction ) {
				fgpPeriodicAction = new (coast::storage::Global()) PeriodicAction("SybCheckCloseOpenedConnectionsAction", lCloseConnectionTimeout);
				fgpPeriodicAction->Start();
			}
			fgInitialized = true;
		}
	}
	return fgInitialized;
}

bool SybCTnewDAImpl::Finis()
{
	StartTrace(SybCTnewDAImpl.Finis);
	if ( fgpPeriodicAction ) {
		fgpPeriodicAction->Terminate();
		delete fgpPeriodicAction;
		fgpPeriodicAction = NULL;
	}
	bool bInitialized;
	{
		LockUnlockEntry me(fgStructureMutex);
		bInitialized = fgInitialized;
		// force pending/upcoming Exec calls to fail
		fgInitialized = false;
	}
	if ( bInitialized ) {
		for (long lCount = 0L; lCount < fgListOfSybCT["Size"].AsLong(0L) && fgpResourcesSema->Acquire(); ++lCount) {
			SybCTnewDA *pSyb = NULL;
			bool bIsOpen = false;
			String strServer, strUser;
			if ( DoGetConnection(pSyb, bIsOpen, strServer, strUser) ) {
				if ( bIsOpen ) {
					pSyb->Close(true);
				}
				delete pSyb;
			}
		}
		delete fgpResourcesSema;
		fgpResourcesSema = NULL;
		SybCTnewDA::Finis(fg_cs_context);
		// trace messages which occurred without a connection
		while ( fgContextMessages.GetSize() ) {
			SystemLog::Warning(fgContextMessages[0L].AsString());
			fgContextMessages.Remove(0L);
		}
	}
	return !fgInitialized;
}

bool SybCTnewDAImpl::IntGetOpen(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user)
{
	StartTrace1(SybCTnewDAImpl.IntGetOpen, "server [" << server << "] user [" << user << "]");
	pSyb = NULL;
	bIsOpen = false;
	Anything anyTimeStamp(coast::storage::Global()), anyEntry(coast::storage::Global());
	TraceAny(fgListOfSybCT, "current list of connections");
	if ( fgListOfSybCT.LookupPath(anyTimeStamp, "Open") && anyTimeStamp.GetSize() ) {
		String strToLookup(server);
		if ( strToLookup.Length() && user.Length() ) {
			strToLookup << '.' << user;
		}
		Trace("Lookup name [" << strToLookup << "]");
		for (long lIdx = 0; lIdx < anyTimeStamp.GetSize(); ++lIdx) {
			Anything anyTS(coast::storage::Global());
			anyTS = anyTimeStamp[lIdx];
			for (long lTimeSub = 0L; lTimeSub < anyTS.GetSize(); ++lTimeSub) {
				if ( ( strToLookup.Length() <= 0 ) || strToLookup == anyTS[lTimeSub][1L].AsString() ) {
					Trace("removing subentry :" << lIdx << ":" << lTimeSub);
					anyEntry = anyTS[lTimeSub];
					anyTS.Remove(lTimeSub);
					// we do not have to close the connection before using because the SybCTnewDA is for the same server and user
					bIsOpen = ( strToLookup.Length() > 0 );
					break;
				}
			}
			if ( !anyEntry.IsNull() ) {
				pSyb = SafeCast(anyEntry[0L].AsIFAObject(), SybCTnewDA);
				if ( anyTS.GetSize() == 0L ) {
					anyTimeStamp.Remove(lIdx);
				}
				break;
			}
		}
	}
	return !anyEntry.IsNull();
}

bool SybCTnewDAImpl::DoGetConnection(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user)
{
	StartTrace(SybCTnewDAImpl.DoGetConnection);
	LockUnlockEntry me(fgStructureMutex);
	return IntDoGetConnection(pSyb, bIsOpen, server, user);
}

bool SybCTnewDAImpl::IntDoGetConnection(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user)
{
	StartTrace(SybCTnewDAImpl.IntDoGetConnection);
	pSyb = NULL;
	bIsOpen = false;
	if ( !server.Length() || !user.Length() || !IntGetOpen(pSyb, bIsOpen, server, user) ) {
		// favour unused connection against open connection of different server/user
		if ( fgListOfSybCT["Unused"].GetSize() ) {
			Trace("removing first unused element");
			pSyb = SafeCast(fgListOfSybCT["Unused"][0L].AsIFAObject(), SybCTnewDA);
			fgListOfSybCT["Unused"].Remove(0L);
		} else {
			String strEmpty;
			if ( IntGetOpen(pSyb, bIsOpen, strEmpty, strEmpty) ) {
				Trace("connection of different server or user");
				// if this call would return false we could possibly delete and recreate an object
				pSyb->Close();
			}
		}
	}
	Trace("returning &" << (long)(IFAObject *)pSyb);
	if ( pSyb == NULL ) {
		SYSERROR("could not get a valid SybCTnewDA");
	}
	return (pSyb != NULL);
}

void SybCTnewDAImpl::DoPutbackConnection(SybCTnewDA *&pSyb, bool bIsOpen, const String &server, const String &user)
{
	StartTrace(SybCTnewDAImpl.DoPutbackConnection);
	LockUnlockEntry me(fgStructureMutex);
	IntDoPutbackConnection(pSyb, bIsOpen, server, user);
}

void SybCTnewDAImpl::IntDoPutbackConnection(SybCTnewDA *&pSyb, bool bIsOpen, const String &server, const String &user)
{
	StartTrace1(SybCTnewDAImpl.IntDoPutbackConnection, "putting &" << (long)(IFAObject *)pSyb);
	if ( bIsOpen ) {
		String strToStore(server);
		strToStore << '.' << user;
		TimeStamp aStamp;
		Anything anyTimeStamp(coast::storage::Global());
		if ( !fgListOfSybCT.LookupPath(anyTimeStamp, "Open") ) {
			anyTimeStamp = Anything(Anything::ArrayMarker(),coast::storage::Global());
			fgListOfSybCT["Open"] = anyTimeStamp;
		}
		Anything anyToStore(coast::storage::Global());
		anyToStore[0L] = (IFAObject *)pSyb;
		anyToStore[1L] = strToStore;
		anyTimeStamp[aStamp.AsString()].Append(anyToStore);
	} else {
		fgListOfSybCT["Unused"].Append((IFAObject *)pSyb);
	}
	TraceAny(fgListOfSybCT, "current list of connections");
}

bool SybCTnewDAImpl::Exec( Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(SybCTnewDAImpl.Exec);
	bool bRet = false;
	DAAccessTimer(SybCTnewDAImpl.Exec, fName, ctx);
	// check if we are initialized
	bool bInitialized = false;
	if ( fgInitialized ) {
		LockUnlockEntry me(fgStructureMutex);
		bInitialized = fgInitialized;
	}
	if ( bInitialized ) {
		SemaphoreEntry aEntry(*fgpResourcesSema);
		// we need the server and user to see if there is an existing and Open SybCTnewDA
		String user, server;
		in->Get( "SybDBUser", user, ctx);
		in->Get( "SybDBHost", server, ctx);
		Trace ("USER IS:" << user );
		Trace ("Host is:" << server );
		out->Put("QuerySource", server, ctx);

		SybCTnewDA *pSyb = NULL;
		bool bIsOpen = false, bDoRetry = true;
		long lTryCount(1L);
		in->Get( "SybDBTries", lTryCount, ctx);
		if ( lTryCount < 1 ) {
			lTryCount = 1L;
		}
		// we slipped through and are ready to get a SybCT and execute a query
		// find a free SybCTnewDA, we should always get a valid SybCTnewDA here!
		while ( bDoRetry && --lTryCount >= 0 ) {
			String command;
			if ( DoGetConnection(pSyb, bIsOpen, server, user) ) {
				SybCTnewDA::DaParams daParams(&ctx, in, out, &fName);
				// test if the connection is still valid, eg. we are able to get/set connection params
				if ( bIsOpen ) {
					SybCTnewDA::DaParams outParams;
					if ( pSyb->GetConProps(CS_USERDATA, (CS_VOID **)&outParams, CS_SIZEOF(SybCTnewDA::DaParams)) != CS_SUCCEED ) {
						// try to close and reopen connection
						pSyb->Close();
						bIsOpen = false;
					}
				}
				if ( !bIsOpen ) {
					String passwd, app;
					in->Get( "SybDBPW", passwd, ctx);
					in->Get( "SybDBApp", app, ctx);
					// open new connection
					if ( !( bIsOpen = pSyb->Open( daParams, user, passwd, server, app) ) ) {
						SYSWARNING("Could not open connection to server [" << server << "] with user [" << user << "]");
					}

					// http://infocenter.sybase.com/help/index.jsp?topic=/com.sybase.infocenter.dc31644.1502/html/sag2/sag2349.htm
					//You can enable delayed_commit for the session with the set command or the database with sp_dboption. The syntax for delayed_commit is:
					//set delayed_commit on | off | default
					//  where on enables the delayed_commit option, off disables it, and default means the database-level setting takes effect.
					//The syntax for sp_dboption is: sp_dboption database_name, 'delayed commit', [true | false]
					//  where true enables delayed_commit at the database level, and false disables the delayed_commit option. Only the DBO can set this parameter.

					// set the option delayed commit just once, after the DB connection was establisched. Further SybCTnewDAImpl::Exec() calls woun't fall here
					if (fbUseDelayedCommit) {
						// activate delayed commit
						String sParamDelayed = "set delayed_commit on";
						String sTmpResultformat;
						in->Get( "SybDBResultFormat", sTmpResultformat, ctx);
						//if ( !(bRet = pSyb->SqlExec(daParams, sParamDelayed, sTmpResultformat) ) )
						if ( !(bRet = pSyb->SqlExec(daParams, sParamDelayed, "", 0, 0) ) ) {
							SYSWARNING("could not execute the sql command " << sParamDelayed << " or it was aborted");
							// maybe a close is better here to reduce the risk of further failures
							pSyb->Close();
							bIsOpen = false;
						}
					}
				}
				if ( bIsOpen ) {
					if ( DoPrepareSQL(command, ctx, in) ) {
						String resultformat, resultsize, resultmaxrows;
						in->Get( "SybDBResultFormat", resultformat, ctx);
						in->Get( "SybDBMaxResultSize", resultsize, ctx);
						in->Get( "SybDBMaxRows", resultmaxrows, ctx);
						if ( !(bRet = pSyb->SqlExec( daParams, command, resultformat, resultsize.AsLong(0L), resultmaxrows.AsLong(-1L) ) ) ) {
							SYSWARNING("could not execute the sql command or it was aborted");
							// maybe a close is better here to reduce the risk of further failures
							pSyb->Close();
							bIsOpen = false;
						} else {
							bDoRetry = false;
						}
					} else {
						out->Put("Messages", "Rendered slot SQL resulted in an empty string", ctx);
						bDoRetry = false;
					}
				}
				DoPutbackConnection(pSyb, bIsOpen, server, user);
			} else {
				SYSERROR("unable to get SybCTnewDA");
			}
			if ( bDoRetry && lTryCount > 0 ) {
				SYSWARNING("Internally retrying to execute command [" << command << "]");
			}
		}
	} else {
		SystemLog::Error("Tried to access SybCTnewDAImpl when SybaseModule was not initialized!\n Try to add a slot SybaseModule to Modules slot and /SybaseModule { /SybCTnewDAImpl { <config> } } into Config.any");
	}
	return bRet;
}

bool SybCTnewDAImpl::DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in)
{
	StartTrace(SybCTnewDAImpl.DoPrepareSQL);
	DAAccessTimer(SybCTnewDAImpl.DoPrepareSQL, fName, ctx);
	OStringStream os(command);
	in->Get("SQL", os, ctx);
	os.flush();
	SubTrace (Query, "QUERY IS [" << command << "]");
	return (command.Length() > 0L);
}

bool SybCTnewDAImpl::CheckCloseOpenedConnections(long lTimeout)
{
	StartTrace(SybCTnewDAImpl.CheckCloseOpenedConnections);
	bool bRet = false;
	Anything anyTimeStamp(coast::storage::Global());
	TimeStamp aStamp;
	aStamp -= lTimeout;
	Trace("current timeout " << lTimeout << "s, resulting time [" << aStamp.AsString() << "]");
	LockUnlockEntry me(fgStructureMutex);
	if ( fgInitialized ) {
		TraceAny(fgListOfSybCT, "current list of connections");
		if ( fgListOfSybCT.LookupPath(anyTimeStamp, "Open") && anyTimeStamp.GetSize() ) {
			SybCTnewDA *pSyb = NULL;
			long lTS = 0L;
			// if we still have open connections and the last access is older than lTimeout seconds
			while ( anyTimeStamp.GetSize() && ( aStamp > TimeStamp(anyTimeStamp.SlotName(lTS)) ) ) {
				Anything anyTS(coast::storage::Global());
				anyTS = anyTimeStamp[lTS];
				TraceAny(anyTS, "stamp of connections to close [" << anyTimeStamp.SlotName(0L) << "]");
				while ( anyTS.GetSize() ) {
					pSyb = SafeCast(anyTS[0L][0L].AsIFAObject(), SybCTnewDA);
					anyTS.Remove(0L);
					if ( pSyb != NULL ) {
						Trace("closing timeouted connection");
						if ( pSyb->Close() ) {
							bRet = true;
						}
					} else {
						SYSWARNING("Sybase connection with address " << (long)pSyb << " not valid anymore!");
					}
					fgListOfSybCT["Unused"].Append((IFAObject *)pSyb);
				}
				anyTimeStamp.Remove(lTS);
			}
		}
	} else {
		SYSERROR("SybCTnewDAImpl not initialized!");
	}
	return bRet;
}

//---- SybCheckCloseOpenedConnectionsAction ----------------------------------------------------------
//: triggers cleanup of open but unused sybase connections
class SybCheckCloseOpenedConnectionsAction : public Action
{
public:
	//--- constructors
	SybCheckCloseOpenedConnectionsAction(const char *name) : Action(name) { }
	~SybCheckCloseOpenedConnectionsAction() {}

	//:cleans the session list from timeouted sessions
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
		StartTrace(SybCheckCloseOpenedConnectionsAction.DoExecAction);
		return SybCTnewDAImpl::CheckCloseOpenedConnections(ctx.Lookup("PeriodicActionTimeout", 60L));
	}
};

//--- SybCheckCloseOpenedConnectionsAction ---
RegisterAction(SybCheckCloseOpenedConnectionsAction);
