/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _QueueWorkingModule_H
#define _QueueWorkingModule_H

#include "Context.h"
#include "Server.h"
#include "Queue.h"
#include "AppLog.h"
#include "StringStream.h"
#include <memory>	// for auto_ptr

//---- QueueWorkingModule ----------------------------------------------------------
//! Queue based module for message passing systems
/*!
\par Configuration
\code
{
	/QueueSize				long		mandatory, number of elements to buffer in the Queue
	/UsePoolStorage			long		optional, [0|1], default 0, use preallocated memory pool for storage of Queue elements
	/PoolStorageSize		long		optional, [kB], default 10240, pool storage size in kbytes
	/NumOfPoolBucketSizes	long		optional, default 10, number of different allocation units within PoolStorage, starts at 16 bytes and always doubles the size so 16 << 10 will give a max usable size of 8192 bytes
	/Logging {
		/Servername			String		optional, default [Server], name of the Server which is used to access the correct AppLog-Channel configuration
		/QueueLogChannel	String		optional, default [QueueLog], name of the named AppLogChannel to create
		/ErrorChannel		String		optional, default [QueueLog], name of the named AppLogChannel to create, where error messages are written
		/WarningChannel		String		optional, default [QueueLog], name of the named AppLogChannel to create where warning messages are written
		/InfoChannel		String		optional, default [QueueLog], name of the named AppLogChannel to create, where info messages are written
	}
}
\endcode

*/
template
<
class ElementType,
	  class ListStorageType
	  >
class QueueWorkingModule : public WDModule
{
	friend class QueueWorkingModuleTest;
public:
	typedef ElementType &ElementTypeRef;
	typedef ElementType const& ConstElementTypeRef;
	typedef Queue<ElementType, ListStorageType> QueueType;
	typedef std::auto_ptr<QueueType> QueueTypePtr;
	typedef std::auto_ptr<Context> ContextPtr;
	typedef typename QueueType::StatusCode StatusCode;
	typedef typename QueueType::BlockingSide BlockingSide;

	//--- constructors
	QueueWorkingModule(const char *name)
		: WDModule(name)
		, fConfig(coast::storage::Global())
		, fQueue()
		, fFailedPutbackMessages()
		, fpQAllocator(0)
		, fContext()
		, fContextLock("QueueWorkingModuleContextLock", coast::storage::Global())
		, fAlive(0UL)
	{
		StartTrace(QueueWorkingModule.QueueWorkingModule);
	}

	//! implementers should initialize module using config
	virtual bool Init(const ROAnything config) {
		StartTrace(QueueWorkingModule.Init);
		SubTraceAny(FullConfig, config, "Config: ");
		ROAnything roaConfig;
		if ( config.LookupPath(roaConfig, fName) ) {
			fConfig = roaConfig.DeepClone();
			TraceAny(fConfig, "Module config");
			fLogName = GetNamedConfig("Logging")["QueueLogChannel"].AsCharPtr("QueueLog");
			fErrorLogName = GetNamedConfig("Logging")["ErrorChannel"].AsCharPtr(fLogName);
			fWarningLogName = GetNamedConfig("Logging")["WarningChannel"].AsCharPtr(fLogName);
			fInfoLogName = GetNamedConfig("Logging")["InfoChannel"].AsCharPtr(fLogName);
			const char *pServerName = GetNamedConfig("Logging")["Servername"].AsCharPtr("Server");
			Server *pServer = Server::FindServer(pServerName);
			Anything dummy;
			fContext = ContextPtr(new Context(fConfig, dummy, pServer, 0, 0));
			IntInitQueue(GetConfig());
			fAlive = 0xf007f007;
			return true;
		}
		return false;
	}

	//! implementers should terminate module expecting destruction
	virtual bool Finis() {
		StartTrace(QueueWorkingModule.Finis);

		SetDead();

		Anything anyStat;
		if ( GetQueueStatistics(anyStat) ) {
			StringStream aStream;
			aStream << "Statistics for [" << GetName() << "] (" << typeName() << ")\n";
			anyStat.PrintOn(aStream, true) << "\n" << std::flush;
			SystemLog::WriteToStderr(aStream.str());
		}

		// delete Queue, also wakes up blocked threads, threads MUST be in termination sequence!!
		IntCleanupQueue();

		{
			LockUnlockEntry me(fContextLock);
			fContext.reset();
		}

		return true;
	}

	//! initializes module after termination for reinitialization; default uses Init; check if this applies
	virtual bool ResetInit(const ROAnything config) {
		StartTrace(QueueWorkingModule.ResetInit);
		// calls Init
		return WDModule::ResetInit(config);
	}

	//! terminates module for reinitialization; default uses Finis; check if this applies
	virtual bool ResetFinis(const ROAnything config) {
		StartTrace(QueueWorkingModule.ResetFinis);
		// calls Finis
		return WDModule::ResetFinis(config);
	}

	bool IsAlive() {
		return fAlive == 0xf007f007;
	}

	bool IsBlocked(BlockingSide aSide = QueueType::eBothSides) {
		StartTrace(QueueWorkingModule.IsBlocked);
		bool bRet = false;
		if ( fQueue.get() && fQueue->IsAlive() && IsAlive() ) {
			bRet = fQueue->IsBlocked(aSide);
		}
		return bRet;
	}

	void Block(BlockingSide aSide) {
		StartTrace(QueueWorkingModule.Block);
		if ( fQueue.get() && fQueue->IsAlive() && IsAlive() ) {
			fQueue->Block(aSide);
		}
	}

	void UnBlock(BlockingSide aSide) {
		StartTrace(QueueWorkingModule.UnBlock);
		if ( fQueue.get() && fQueue->IsAlive() && IsAlive() ) {
			fQueue->UnBlock(aSide);
		}
	}

	/*! main accessor functions to work with the queue */
	StatusCode PutElement(ConstElementTypeRef anyELement, bool bTryLock = false) {
		StartTrace(QueueWorkingModule.PutElement);
		StatusCode eRet = QueueType::eDead;
		if (fQueue.get() && fQueue->IsAlive() && IsAlive()) {
			eRet = fQueue->Put(anyELement, bTryLock);
			if (eRet != QueueType::eSuccess) {
				SYSWARNING("Queue->Put failed, QueueType::StatusCode:" << eRet << " !");
			}
		}
		return eRet;
	}

	StatusCode GetElement(ElementTypeRef anyValues, bool bTryLock = false) {
		StartTrace(QueueWorkingModule.GetElement);
		StatusCode eRet = QueueType::eDead;
		if (fQueue.get() && fQueue->IsAlive() && IsAlive()) {
			Trace("Queue still alive");
			// try to get a failed message first
			eRet = fFailedPutbackMessages->Get(anyValues, true);
			if (eRet == QueueType::eEmpty) {
				// Default is blocking get to save cpu time
				eRet = fQueue->Get(anyValues, bTryLock);
				if ((eRet != QueueType::eSuccess) && (eRet != QueueType::eEmpty)) {
					SYSWARNING("Queue->Get failed, QueueType::StatusCode:" << eRet << " !");
				}
			}
		}
		return eRet;
	}

	void PutBackElement(ConstElementTypeRef anyValues) {
		StartTrace(QueueWorkingModule.PutBackElement);
		// put message back to the queue (Appends!) if possible
		// take care not to lock ourselves up here, thus we MUST use a trylock here!
		StatusCode eRet = PutElement(anyValues, true);
		if (eRet != QueueType::eSuccess && (eRet & QueueType::eFull)) {
			// no more room in regular Queue, need to store this message internally for later put back
			fFailedPutbackMessages->Put(anyValues, true);
		}
	}

	/* exclusively consume all Elements from queue, threads which are blocked on the queue to get an element will be woken up because of the released semaphore. But instead of getting an Element it will get nothing back and should be able to handle this correctly.
		\param anyELements Anything to hold the elements removed from the queue
		\return number of elements removed from the queue */
	template < class DestListType >
	long FlushQueue(DestListType &anyElements) {
		StartTrace(QueueWorkingModule.FlushQueue);
		long lElements = 0L;
		if (fQueue.get() && fQueue->IsAlive() && IsAlive()) {
			// first we need to empty putback Q
			fFailedPutbackMessages->EmptyQueue(anyElements);
			fQueue->EmptyQueue(anyElements);
			lElements = anyElements.size();
			LogInfo(String(GetName()) << ": flushed " << lElements << " elements");
		}
		return lElements;
	}

	virtual bool DoGetQueueStatistics(Anything &anyStat) {
		StartTrace(QueueWorkingModule.DoGetQueueStatistics);
		if ( fQueue.get() ) {
			fQueue->GetStatistics(anyStat);
			return true;
		}
		return false;
	}

	bool GetQueueStatistics(Anything &anyStat) {
		StartTrace(QueueWorkingModule.GetQueueStatistics);
		return DoGetQueueStatistics(anyStat);
	}

	long GetCurrentSize() {
		StartTrace(QueueWorkingModule.GetCurrentSize);
		long lSize = 0L;
		if (fQueue.get()) {
			lSize = fQueue->GetSize();
		}
		return lSize;
	}

	void LogError(String strMessage) {
		StartTrace(QueueWorkingModule.LogError);
		SYSERROR(strMessage);
		Log(strMessage, fErrorLogName, AppLogModule::eERROR);
	}

	void LogWarning(String strMessage) {
		StartTrace(QueueWorkingModule.LogWarning);
		SYSWARNING(strMessage);
		Log(strMessage, fWarningLogName, AppLogModule::eWARNING);
	}

	void LogInfo(String strMessage) {
		StartTrace(QueueWorkingModule.LogInfo);
		Log(strMessage, fInfoLogName, AppLogModule::eINFO);
	}

	void LogWithSeverity(String strMessage, AppLogModule::eLogLevel iLevel) {
		StartTrace(QueueWorkingModule.LogInfo);
		Log(strMessage, fLogName, iLevel);
	}

	Context *GetContext() const { return fContext.get(); };
	QueueType *GetQueue() const { return fQueue.get(); };

	String typeName() const {
		if ( fQueue.get() ) return fQueue->typeName();
		return String("<queue not initialized>");
	}
protected:
	ROAnything GetNamedConfig(const char *name) {
		StartTrace(QueueWorkingModule.GetNamedConfig);
		return ((ROAnything)fConfig)[name];
	}

	ROAnything GetConfig() {
		StartTrace(QueueWorkingModule.GetConfig);
		return ((ROAnything)fConfig);
	}

	void IntCleanupQueue() {
		StartTrace(QueueWorkingModule.IntCleanupQueue);
		// we could do something here to persist the content of the queue and the putback message buffer
		fQueue.reset();
		fFailedPutbackMessages.reset();
		if (fpQAllocator) {
			MT_Storage::UnrefAllocator(fpQAllocator);
			fpQAllocator = 0;
		}
	}

	void SetDead() {
		fAlive = 0x00dead00;
	};

	void Log(String strMessage, const char *channel, AppLogModule::eLogLevel iLevel) {
		StartTrace(QueueWorkingModule.Log);
		Trace(strMessage);
		if (IsAlive() && fContext.get()) {
			LockUnlockEntry me(fContextLock);
			if (IsAlive() && fContext.get()) {
				fContext->GetTmpStore()["LogMessage"] = strMessage;
				AppLogModule::Log(*fContext, channel, iLevel);
			}
		}
	}

	void Log(Anything &anyStatus, const char *channel, AppLogModule::eLogLevel iLevel) {
		StartTrace(QueueWorkingModule.Log);
		TraceAny(anyStatus, "content to log");
		if (IsAlive() && fContext.get()) {
			LockUnlockEntry me(fContextLock);
			if (IsAlive() && fContext.get()) {
				Context::PushPopEntry<ROAnything> statusEntry(*fContext, "Status", anyStatus, "QueueWorkingStatus");
				AppLogModule::Log(*fContext, channel, iLevel);
			}
		}
	}

private:
	void IntInitQueue(const ROAnything roaConfig) {
		StartTrace(QueueWorkingModule.IntInitQueue);
		long lQueueSize = roaConfig["QueueSize"].AsLong(100L);
		Allocator *pAlloc = coast::storage::Global();

		if (roaConfig["UsePoolStorage"].AsLong(0) == 1) {
			// create unique allocator id based on a pointer value
			long lAllocatorId = (((long) this) & 0x00007FFF);
			pAlloc = MT_Storage::MakePoolAllocator(roaConfig["PoolStorageSize"].AsLong(10240), roaConfig["NumOfPoolBucketSizes"].AsLong(10),
					lAllocatorId);
			if (pAlloc == 0) {
				SYSERROR("was not able to create PoolAllocator with Id:" << lAllocatorId << " for [" << GetName() << "], check config!");
			} else {
				// store allocator pointer for later deletion
				MT_Storage::RefAllocator(pAlloc);
				fpQAllocator = pAlloc;
			}
		}
		fQueue = QueueTypePtr(new (pAlloc) QueueType(GetName(), lQueueSize, pAlloc));
		fFailedPutbackMessages = QueueTypePtr(new (pAlloc) QueueType(String(GetName()).Append("PutBack")));
	}

	Anything	fConfig;
	QueueTypePtr fQueue, fFailedPutbackMessages;
	Allocator	*fpQAllocator;
	ContextPtr fContext;
	SimpleMutex fContextLock;
	String		fErrorLogName, fWarningLogName, fInfoLogName, fLogName;
	u_long		fAlive;
};

typedef QueueWorkingModule<Anything, Anything> AnyQueueWorkingModule;

#endif
