/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConnectionPool.h"
#include "OraclePooledConnection.h"
#include "SystemLog.h"
#include "Timers.h"
#include "TimeStamp.h"
#include "StringStream.h"
#include "PeriodicAction.h"
#include "AnyIterators.h"
#include "Tracer.h"
#include "MT_Storage.h"
#include "InitFinisManager.h"
#include "singleton.hpp"

namespace coast
{
	namespace oracle
	{
		namespace {
			class OracleTlsKeyInitializer {
				THREADKEY fConnectionKey;
				bool fTLSUsable;
				StatEvtHandlerPtrType fpStatEvtHandlerTLS;
			public:
				OracleTlsKeyInitializer() : fConnectionKey(0), fTLSUsable(false)  {
					if (THRKEYCREATE(fConnectionKey, 0)) {
						SystemLog::Error("TlsAlloc of fCleanerKey failed");
					} else {
						fTLSUsable = true;
						fpStatEvtHandlerTLS = StatEvtHandlerPtrType( new WPMStatHandler( 0 ) );
					}
					InitFinisManager::IFMTrace("OracleTlsKeyInitializer::Initialized\n");
				}
				~OracleTlsKeyInitializer() {
					if (THRKEYDELETE(fConnectionKey) != 0) {
						SystemLog::Error("TlsFree of fCleanerKey failed" );
					}
					InitFinisManager::IFMTrace("OracleTlsKeyInitializer::Finalized\n");
				}
				THREADKEY getConnectionKey() const {
					return fConnectionKey;
				}
				bool TLSUsable() const {
					return fTLSUsable;
				}
				WPMStatHandler* getStatHandlerTLS() const {
					return fpStatEvtHandlerTLS.get();
				}
			};
		    typedef coast::utility::singleton_default<OracleTlsKeyInitializer> OracleTlsKeyInitializerSingleton;
		}

		class ThreadSpecificConnectionCleaner : public CleanupHandler {
		public:
			static ThreadSpecificConnectionCleaner fgCleaner;
		protected:
			//:close and destruct OraclePooledConnection
			virtual bool DoCleanup() {
				StatTrace(ThreadSpecificConnectionCleaner.DoCleanup, "ThrdId: " << Thread::MyId(), coast::storage::Global());
				OraclePooledConnection *pConnection = 0;
				if (GETTLSDATA(OracleTlsKeyInitializerSingleton::instance().getConnectionKey(), pConnection, OraclePooledConnection)) {
					if ( pConnection->isOpen() ) {
						pConnection->Close( true );
					}
					delete pConnection;
					pConnection = 0;
					if (SETTLSDATA(OracleTlsKeyInitializerSingleton::instance().getConnectionKey(), pConnection)) {
						return true;
					}
				}
				return false;
			}
		};
		ThreadSpecificConnectionCleaner ThreadSpecificConnectionCleaner::fgCleaner;

		ConnectionPool::ConnectionPool( const char *name ) :
			fStructureMutex( String( name ).Append( "StructureMutex" ), coast::storage::Global() ), fListOfConnections(
				coast::storage::Global() ), fInitialized( false ), fpPeriodicAction( NULL ), fpResourcesSema( NULL ), fName(name)
		{
			StartTrace(ConnectionPool.ConnectionPool);
		}

		ConnectionPool::~ConnectionPool()
		{
			StartTrace(ConnectionPool.~ConnectionPool);
			PrintStatisticsOnStderr( GetName() );
		}

		void ConnectionPool::DoGetStatistic(Anything &statistics)
		{
			if ( fpStatEvtHandlerPool.get() ) {
				Anything anyStats;
				fpStatEvtHandlerPool->Statistic( anyStats );
				statistics["Pool"] = anyStats;
			}
			if ( OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS() ) {
				Anything anyStats;
				OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS()->Statistic( anyStats );
				statistics["TLS"] = anyStats;
			}
		}

		Allocator *getListAllocator( const ROAnything roaConfig )
		{
			Allocator *pAlloc = coast::storage::Global();
			if ( roaConfig["UsePoolStorage"].AsLong( 1 ) == 1 ) {
				// create unique allocator id based on a pointer value
				long lAllocatorId = 0x011e0FFF;
				pAlloc = MT_Storage::MakePoolAllocator( roaConfig["PoolStorageSize"].AsLong( 99 ),
														roaConfig["NumOfPoolBucketSizes"].AsLong( 26 ), lAllocatorId );
				if ( pAlloc == NULL ) {
					StatTrace(ConnectionPool.getListAllocator, "failed to create PoolAllocator, using coast::storage::Global()", coast::storage::Current());
					SYSWARNING("was not able to create PoolAllocator with Id:" << lAllocatorId);
					pAlloc = coast::storage::Global();
				} else {
					StatTrace(ConnectionPool.getListAllocator, "created PoolAllocator", coast::storage::Current());
					// store allocator pointer for later deletion
					MT_Storage::RefAllocator( pAlloc );
				}
			}
			return pAlloc;
		}

		void releaseListAllocator( Allocator *pAlloc )
		{
			if ( pAlloc != coast::storage::Global() ) {
				StatTrace(ConnectionPool.releaseListAllocator, "unreferencing Allocator", coast::storage::Current());
				MT_Storage::UnrefAllocator( pAlloc );
			}
		}

		bool ConnectionPool::Init( ROAnything myCfg )
		{
			StartTrace(ConnectionPool.Init);
			if ( !fInitialized ) {
				long nrOfConnections( myCfg["ParallelQueries"].AsLong( 5L ) );
				long lCloseConnectionTimeout( myCfg["CloseConnectionTimeout"].AsLong( 60L ) );

				LockUnlockEntry me( fStructureMutex );
				{
					// the following call is guaranteed to return a valid Allocator
					fListOfConnections.SetAllocator( getListAllocator( myCfg ) );
					// use the semaphore to block when no more resources are available
					fListOfConnections["Size"] = nrOfConnections;
					fpResourcesSema = new Semaphore( nrOfConnections );
					for ( long i = 0; i < nrOfConnections; ++i ) {
						OraclePooledConnection *pConnection = new ( coast::storage::Global() ) OraclePooledConnection( i,
								myCfg["MemPoolSize"].AsLong( 2048L ), myCfg["MemPoolBuckets"].AsLong( 16L ) );
						IntReleaseConnection( pConnection );
					}
					fpStatEvtHandlerPool = StatEvtHandlerPtrType( new WPMStatHandler( nrOfConnections ) );
					if ( !fpPeriodicAction ) {
						fpPeriodicAction = new (coast::storage::Global()) PeriodicAction( "OracleCheckCloseOpenedConnectionsAction",
															   lCloseConnectionTimeout );
						fpPeriodicAction->Start();
					}
					fInitialized = true;
				}
			}
			return fInitialized;
		}

		bool ConnectionPool::Finis()
		{
			StartTrace(ConnectionPool.Finis);
			if ( fpPeriodicAction ) {
				fpPeriodicAction->Terminate();
				delete fpPeriodicAction;
				fpPeriodicAction = NULL;
			}
			bool bInitialized;
			{
				LockUnlockEntry me( fStructureMutex );
				bInitialized = fInitialized;
				// force pending/upcoming Exec calls to fail
				fInitialized = false;
			}
			if ( bInitialized ) {
				for ( long lCount = 0L; lCount < fListOfConnections["Size"].AsLong( 0L ); ++lCount ) {
					OraclePooledConnection *pConnection( NULL );
					String strServer, strUser;
					fpResourcesSema->Acquire();
					LockUnlockEntry me( fStructureMutex );
					if ( IntBorrowConnection( pConnection, strServer, strUser ) ) {
						if ( pConnection->isOpen() ) {
							pConnection->Close( true );
						}
						delete pConnection;
					}
				}
				delete fpResourcesSema;
				fpResourcesSema = NULL;
				Allocator *pAlloc = fListOfConnections.GetAllocator();
				fListOfConnections.clear();
				fListOfConnections.SetAllocator(coast::storage::Global());
				releaseListAllocator(pAlloc);
			}
			return !fInitialized;
		}

		bool ConnectionPool::BorrowConnection( OraclePooledConnection *&pConnection, const String &server, const String &user, bool bUseTLS )
		{
			StartTrace(ConnectionPool.BorrowConnection);
			bool bRet(false);
			if ( OracleTlsKeyInitializerSingleton::instance().TLSUsable() && bUseTLS ) {
				if ( ( bRet = GETTLSDATA(OracleTlsKeyInitializerSingleton::instance().getConnectionKey(), pConnection, OraclePooledConnection) ) ) {
					// verify if current connection is for same server/user
					Trace("got connection from TLS, server [" << pConnection->getServer() << "], user [" << pConnection->getUser() << "]");
					if ( !pConnection->isOpenFor(server, user) ) {
						Trace("got connection for different server/user, closing");
						pConnection->Close();
					}
				} else {
					Trace("need to create new Connection")
					Thread::RegisterCleaner(&ThreadSpecificConnectionCleaner::fgCleaner);
					pConnection = new ( coast::storage::Global() ) OraclePooledConnection( Thread::MyId(), 2048L, 26L );
					if ( pConnection != NULL ) {
						bRet = SETTLSDATA(OracleTlsKeyInitializerSingleton::instance().getConnectionKey(), pConnection);
						Trace("connection stored in TLS:" << (bRet ? "true" : "false"));
						if ( OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS() ) {
							OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS()->incrementPoolSize();
						}
					}
				}
				if ( OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS() ) {
					OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS()->HandleStatEvt( WPMStatHandler::eEnter );
				}
			} else {
				if ( fpStatEvtHandlerPool.get() ) {
					fpStatEvtHandlerPool->HandleStatEvt( WPMStatHandler::eEnter );
				}
				fpResourcesSema->Acquire();
				LockUnlockEntry me( fStructureMutex );
				bRet = IntBorrowConnection( pConnection, server, user );
			}
			return bRet;
		}

		void ConnectionPool::ReleaseConnection( OraclePooledConnection *&pConnection, bool bUseTLS )
		{
			StartTrace(ConnectionPool.ReleaseConnection);
			if ( OracleTlsKeyInitializerSingleton::instance().TLSUsable() && bUseTLS ) {
				Trace("nothing to do, keep connection as long as we (Thread) are alive");
				if ( OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS() ) {
					OracleTlsKeyInitializerSingleton::instance().getStatHandlerTLS()->HandleStatEvt( WPMStatHandler::eLeave );
				}
			} else {
				{
					LockUnlockEntry me( fStructureMutex );
					IntReleaseConnection( pConnection );
					fpResourcesSema->Release();
				}
				if ( fpStatEvtHandlerPool.get() ) {
					fpStatEvtHandlerPool->HandleStatEvt( WPMStatHandler::eLeave );
				}
			}
		}

		bool ConnectionPool::IntGetOpen( OraclePooledConnection *&pConnection, const String &server, const String &user )
		{
			StartTrace1(ConnectionPool.IntGetOpen, "server [" << server << "] user [" << user << "]");
			pConnection = NULL;
			ROAnything roaTimeStamp, roaTSEntry, roaConn;
			TraceAny(fListOfConnections, "current list of connections");
			if ( ROAnything( fListOfConnections ).LookupPath( roaTimeStamp, "Open" ) && roaTimeStamp.GetSize() ) {
				String strToLookup( server );
				if ( strToLookup.Length() && user.Length() ) {
					strToLookup << '.' << user;
				}
				Trace("Lookup name [" << strToLookup << "]");
				bool bDone( false );
				AnyExtensions::Iterator<ROAnything> aTSIter( roaTimeStamp );
				while ( !bDone && aTSIter.Next( roaTSEntry ) ) {
					AnyExtensions::Iterator<ROAnything> aConnIter( roaTSEntry );
					while ( !bDone && aConnIter.Next( roaConn ) ) {
						if ( ( strToLookup.Length() <= 0 ) || strToLookup.IsEqual( roaConn[1L].AsCharPtr("") ) ) {
							String strTSEntry( "Open" );
							strTSEntry << ':' << aTSIter.Index();
							String strConnEntry( strTSEntry );
							strConnEntry << ':' << aConnIter.Index();
							Trace("removing conn entry :" << strConnEntry );
							pConnection = SafeCast(roaConn[0L].AsIFAObject(), OraclePooledConnection);
							SlotCleaner::Operate( fListOfConnections, strConnEntry );
							if ( roaTSEntry.GetSize() <= 0 ) {
								Trace("removing timestamp entry :" << strTSEntry );
								SlotCleaner::Operate( fListOfConnections, strTSEntry );
							}
							TraceAny(fListOfConnections, "after cleaning");
							// we do not have to close the connection before using because the OraclePooledConnection is for the same server and user
							bDone = true;
						}
					}
				}
			}
			return pConnection;
		}

		bool ConnectionPool::IntBorrowConnection( OraclePooledConnection *&pConnection, const String &server,
				const String &user )
		{
			StartTrace(ConnectionPool.IntBorrowConnection);
			pConnection = NULL;
			if ( !server.Length() || !user.Length() || !IntGetOpen( pConnection, server, user ) ) {
				Trace("favour unused connection against open connection of different server/user");
				if ( fListOfConnections["Unused"].GetSize() ) {
					Trace("removing first unused element");
					pConnection = SafeCast(fListOfConnections["Unused"][0L].AsIFAObject(), OraclePooledConnection);
					fListOfConnections["Unused"].Remove( 0L );
				} else {
					Trace("no unused connection found, try to look for any open connection not in use");
					String strEmpty;
					if ( IntGetOpen( pConnection, strEmpty, strEmpty ) ) {
						Trace("need to close connection of different server or user before using connection");
						// if this call would return false we could possibly delete and recreate an object
						pConnection->Close();
					}
				}
			}
			Trace("returning &" << (long)(IFAObject *)pConnection);
			if ( pConnection == NULL ) {
				SYSERROR("could not get a valid OracleConnection");
			}
			return ( pConnection != NULL );
		}

		void ConnectionPool::IntReleaseConnection( OraclePooledConnection *&pConnection )
		{
			StartTrace1(ConnectionPool.IntReleaseConnection, "putting &" << (long)(IFAObject *)pConnection);
			if ( pConnection->isOpen() ) {
				String strToStore( pConnection->getServer() );
				strToStore << '.' << pConnection->getUser();
				TimeStamp aStamp;
				Anything anyTimeStamp( fListOfConnections.GetAllocator() );
				if ( !fListOfConnections.LookupPath( anyTimeStamp, "Open" ) ) {
					anyTimeStamp = Anything(Anything::ArrayMarker(), fListOfConnections.GetAllocator() );
					fListOfConnections["Open"] = anyTimeStamp;
				}
				Anything anyToStore( fListOfConnections.GetAllocator() );
				anyToStore[0L] = (IFAObject *) pConnection;
				anyToStore[1L] = strToStore;
				anyTimeStamp[aStamp.AsString()].Append( anyToStore );
			} else {
				fListOfConnections["Unused"].Append( (IFAObject *) pConnection );
			}
			TraceAny(fListOfConnections, "current list of connections");
		}

		bool ConnectionPool::CheckCloseOpenedConnections( long lTimeout )
		{
			StartTrace(ConnectionPool.CheckCloseOpenedConnections);
			bool bRet = false;
			TimeStamp aStamp;
			aStamp -= lTimeout;
			Trace("current timeout " << lTimeout << "s, resulting time [" << aStamp.AsString() << "]");
			LockUnlockEntry me( fStructureMutex );
			if ( fInitialized ) {
				Anything anyTimeStamp( fListOfConnections.GetAllocator() );
				TraceAny(fListOfConnections, "current list of connections");
				if ( fListOfConnections.LookupPath( anyTimeStamp, "Open" ) && anyTimeStamp.GetSize() ) {
					OraclePooledConnection *pConnection = NULL;
					long lTS = 0L;
					// if we still have open connections and the last access is older than lTimeout seconds
					while ( anyTimeStamp.GetSize() && ( aStamp > TimeStamp( anyTimeStamp.SlotName( lTS ) ) ) ) {
						Anything anyTS( fListOfConnections.GetAllocator() );
						anyTS = anyTimeStamp[lTS];
						TraceAny(anyTS, "stamp of connections to close [" << anyTimeStamp.SlotName(0L) << "]");
						while ( anyTS.GetSize() ) {
							pConnection = SafeCast(anyTS[0L][0L].AsIFAObject(), OraclePooledConnection);
							anyTS.Remove( 0L );
							if ( pConnection != NULL ) {
								Trace("closing timeouted connection");
								if ( pConnection->Close() ) {
									bRet = true;
								}
							} else {
								SYSWARNING("connection with address " << (long)pConnection << " not valid anymore!");
							}
							fListOfConnections["Unused"].Append( (IFAObject *) pConnection );
						}
						anyTimeStamp.Remove( lTS );
					}
				}
			} else {
				SYSERROR("ConnectionPool not initialized!");
			}
			return bRet;
		}

	} // end namespace oracle
} // end namespace coast
