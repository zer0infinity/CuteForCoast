/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MT_Storage.h"

//--- project modules used -----------------------------------------------------
#include "InitFinisManagerMTFoundation.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "PoolAllocator.h"
#include "SystemLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <cstring>
#if defined(WIN32)
#include <cstdio>
#endif

#if 1
#define TrackLockerInit(lockvar) , lockvar(0)
#define TrackLockerDef(lockvar)	volatile long lockvar

class CurrLockerEntry
{
	volatile long &frLockerId;
public:
	CurrLockerEntry(volatile long &rLockerId, const char *pFile, long lLine)
		: frLockerId(rLockerId) {
		CheckException(pFile, lLine);
		frLockerId = Thread::MyId();
	}
	~CurrLockerEntry() {
		frLockerId = 0;
	}
	inline void CheckException(const char *pFile, long lLine) {
		if ( frLockerId != 0 ) {
			long lOther = frLockerId;
			char buf[256] = { 0 };
			snprintf(buf, sizeof(buf), "\n%s:%ld Another Locker entered already! otherThreadId:%ld currentThreadId:%ld\n", pFile, lLine, lOther, Thread::MyId());
			SystemLog::WriteToStderr(buf, strlen(buf));
		}
	}
};

#define TrackLocker(lockvar)	CurrLockerEntry aEntry(lockvar, __FILE__, __LINE__)

#else

#define TrackLockerInit(lockvar)
#define TrackLockerDef(lockvar)
#define TrackLocker(lockvar)

#endif

class MTPoolAllocator: public PoolAllocator
{
public:
	//! create and initialize a pool allocator
	/*! \param poolid use poolid to distinguish more than one pool
		\param poolSize size of pre-allocated pool in kBytes, default 1MByte
		\param maxKindOfBucket number of different allocation units within PoolAllocator, starts at 16 bytes and doubles the size for maxKindOfBucket times. So maxKindOfBucket=10 will give a max usable size of 8192 bytes. */
	MTPoolAllocator(long poolid, u_long poolSize = 1024, u_long maxKindOfBucket = 10)
		: PoolAllocator(poolid, poolSize, maxKindOfBucket) TrackLockerInit(fCurrLockerId) {}
	//! destroy a pool only if its empty, i.e. all allocated bytes are freed
	virtual ~MTPoolAllocator() { }
	//! implement hook for freeing memory
	virtual inline size_t Free(void *vp) {
		TrackLocker(fCurrLockerId);
		return PoolAllocator::Free(vp);
	}

protected:
	TrackLockerDef(fCurrLockerId);
	//!implement hook for allocating memory using bucketing
	virtual inline void *Alloc(u_long allocSize) {
		TrackLocker(fCurrLockerId);
		return PoolAllocator::Alloc(allocSize);
	}
};

//------------- Utilities for Memory Tracking (multi threaded) --------------

MT_MemTracker::MT_MemTracker(const char *name, long lId)
	: MemTracker(name)
{
	int iRet = 0;
	if ( !CREATEMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex create failed: " << SystemLog::SysErrorMsg(iRet));
	}
	SetId( lId );
}

MT_MemTracker::~MT_MemTracker()
{
	int iRet = 0;
	if ( !DELETEMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex delete failed: " << SystemLog::SysErrorMsg(iRet));
	}
}

void *MT_MemTracker::operator new(size_t size)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(MT_MemTracker));
	return vp;
}

void *MT_MemTracker::operator new(size_t size, Allocator *)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(MT_MemTracker));
	return vp;
}

void MT_MemTracker::operator delete(void *vp)
{
	if (vp) {
		::operator delete(vp);
	}
}

void MT_MemTracker::TrackAlloc(MemoryHeader *mh)
{
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	MemTracker::TrackAlloc(mh);
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex unlock failed" << SystemLog::SysErrorMsg(iRet));
	}
}

void MT_MemTracker::TrackFree(MemoryHeader *mh)
{
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	MemTracker::TrackFree(mh);
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex unlock failed: " << SystemLog::SysErrorMsg(iRet));
	}
}

l_long MT_MemTracker::CurrentlyAllocated()
{
	l_long l;
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	l = fAllocated;
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	return  l;
}

void MT_MemTracker::PrintStatistic(long lLevel)
{
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	MemTracker::PrintStatistic(lLevel);
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
}

class MTStorageHooks : public StorageHooks
{
public:
	MTStorageHooks();
	virtual ~MTStorageHooks();

	virtual void Initialize();
	virtual void Finalize();
	virtual Allocator *Global();

	virtual Allocator *Current();

	/*! allocate a memory tracker object
		\param name name of the tracker
		\param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
		\return poniter to a newly created MemTracker object */
	virtual MemTracker *MakeMemTracker(const char *name, bool bThreadSafe);

	virtual void Lock() { fMutex.Lock(); }
	virtual void Unlock() { fMutex.Unlock(); }

private:
	bool fgInitialized;
    SimpleMutex fMutex;
};

MemTracker *MT_Storage::fOldTracker = NULL;
static MTStorageHooks *sgpMTHooks = NULL;

//---- MT_Storage ------------------------------------------
THREADKEY MT_Storage::fgAllocatorKey = 0;
SimpleMutex *MT_Storage::fgpAllocatorInit = NULL;
bool MT_Storage::fgInitialized = false;

struct AllocList {
	Allocator *wdallocator;
	AllocList *next;
};

static AllocList *fgPoolAllocatorList = 0;

class MTStorageInitializer : public InitFinisManagerMTFoundation
{
public:
	MTStorageInitializer(unsigned int uiPriority)
		: InitFinisManagerMTFoundation(uiPriority) {
		IFMTrace("MTStorageInitializer created\n");
	}

	virtual void DoInit() {
		IFMTrace("MTStorageInitializer::DoInit\n");
		if (THRKEYCREATE(MT_Storage::fgAllocatorKey, 0)) {
			SystemLog::Error("TlsAlloc of MT_Storage::fgAllocatorKey failed");
		}
		MT_Storage::fgpAllocatorInit = new SimpleMutex("AllocatorInit", Coast::Storage::Global());
		MT_Storage::Initialize();
	}

	virtual void DoFinis() {
		IFMTrace("MTStorageInitializer::DoFinis\n");
		MT_Storage::Finalize();
		delete MT_Storage::fgpAllocatorInit;
		MT_Storage::fgpAllocatorInit = NULL;
		if (THRKEYDELETE(MT_Storage::fgAllocatorKey) != 0) {
			SystemLog::Error("TlsFree of MT_Storage::fgAllocatorKey failed" );
		}
	}
};

static MTStorageInitializer *psgMTStorageInitializer = new MTStorageInitializer(5);

void MT_Storage::Initialize()
{
	StatTrace(MT_Storage.Initialize, "entering", Coast::Storage::Global());
	// must be called before threading is activated
	if ( !fgInitialized ) {
		sgpMTHooks = new MTStorageHooks();
		Coast::Storage::SetHooks(sgpMTHooks);
		// switch to thread safe memory tracker if enabled through COAST_TRACE_STORAGE
		Allocator *a = Coast::Storage::Global();
		if ( a && Coast::Storage::GetStatisticLevel() >= 1 ) {
			fOldTracker = a->ReplaceMemTracker(Coast::Storage::MakeMemTracker("MTGlobalAllocator", true));
		}
		fgInitialized = true;
	}
	StatTrace(MT_Storage.Initialize, "leaving", Coast::Storage::Global());
}

void MT_Storage::Finalize()
{
	StatTrace(MT_Storage.Finalize, "entering", Coast::Storage::Global());
	if ( fgInitialized ) {
		// terminate pool allocators
		{
			LockUnlockEntry me(*fgpAllocatorInit);
			while (fgPoolAllocatorList) {
				AllocList *elmt = fgPoolAllocatorList;
				if ( elmt->wdallocator->GetId() != Coast::Storage::DoGlobal()->GetId() ) {
					SYSERROR("unfreed allocator found id: " << elmt->wdallocator->GetId());
					delete elmt->wdallocator;
				}
				fgPoolAllocatorList = elmt->next;
				::free(elmt);
			}
		}
		Allocator *a = Coast::Storage::Global();
		if ( a ) {
			if ( fOldTracker ) {
				MemTracker *pCurrTracker = a->ReplaceMemTracker(fOldTracker);
				StatTrace(MT_Storage.Finalize, "setting MemTracker back from [" << ( pCurrTracker ? pCurrTracker->GetName() : "NULL" ) << "] to [" << fOldTracker->GetName() << "]", Coast::Storage::Global());
				if ( pCurrTracker && Coast::Storage::GetStatisticLevel() >= 1 ) {
					pCurrTracker->PrintStatistic(2);
				}
				delete pCurrTracker;
				fOldTracker = NULL;
			}
		}
		StorageHooks *pOldHook = Coast::Storage::SetHooks(NULL);
		if ( pOldHook == sgpMTHooks ) {
			delete sgpMTHooks;
			sgpMTHooks = NULL;
		}
		fgInitialized = false;
	}
	StatTrace(MT_Storage.Finalize, "leaving", Coast::Storage::Global());
}

void MT_Storage::RefAllocator(Allocator *wdallocator)
{
	StartTrace1(MT_Storage.RefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L));
	if ( fgInitialized ) {
		if ( wdallocator ) {
			// in mt case we need to control ref counting with mutexes
			LockUnlockEntry me(*fgpAllocatorInit);

			AllocList *elmt = (AllocList *)::calloc(1, sizeof(AllocList));
			// catch memory allocation problem
			if (!elmt) {
				static const char crashmsg[] = "FATAL: MT_Storage::RefAllocator calloc failed. I will crash :-(\n";
				SystemLog::WriteToStderr(crashmsg, sizeof(crashmsg));
				SystemLog::Error("allocation failed for RefAllocator");
				return;
			}

			// normal case everything ok
			wdallocator->Ref();
			Trace("refcount is now:" << wdallocator->RefCnt());
			// update linked list
			elmt->wdallocator = wdallocator;
			elmt->next = fgPoolAllocatorList;
			fgPoolAllocatorList = elmt;
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
}

void MT_Storage::UnrefAllocator(Allocator *wdallocator)
{
	long const allocId = wdallocator ? wdallocator->GetId() : -1L;
	StatTrace(MT_Storage.UnrefAllocator, "Id:" << allocId << " --- entering ---", Coast::Storage::Global());
	if ( fgInitialized ) {
		if (wdallocator) {	// just to be robust wdallocator == 0 should not happen
			LockUnlockEntry me(*fgpAllocatorInit);
			wdallocator->Unref();
			StatTrace(MT_Storage.UnrefAllocator, "refcount is now:" << wdallocator->RefCnt(), Coast::Storage::Global());

			if ( wdallocator->RefCnt() <= 0 ) {
				// remove pool allocator
				AllocList *elmt = fgPoolAllocatorList;
				AllocList *prev = fgPoolAllocatorList;

				while (elmt && (elmt->wdallocator != wdallocator)) {
					prev = elmt;
					elmt = elmt->next;
				}
				if ( elmt ) {
					if ( elmt == fgPoolAllocatorList ) {
						fgPoolAllocatorList = elmt->next;
					} else {
						prev->next = elmt->next;
					}

					delete elmt->wdallocator;
					::free(elmt);
				}
			}
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
	StatTrace(MT_Storage.UnrefAllocator, "Id:" << allocId << " --- leaving ---", Coast::Storage::Global());
}

bool MT_Storage::RegisterThread(Allocator *wdallocator)
{
	if ( fgInitialized ) {
		// can only be used once for the same thread
		Allocator *oldAllocator = 0;

		// check for old allocator, and if any, dont override it
		GETTLSDATA(MT_Storage::fgAllocatorKey, oldAllocator, Allocator);
		if ( oldAllocator == NULL ) {
			StatTrace(MT_Storage.RegisterThread, "setting Allocator:" << (long)wdallocator << " to ThreadLocalStorage", Coast::Storage::Global());
			return !SETTLSDATA(MT_Storage::fgAllocatorKey, wdallocator);
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
	return false;
}

bool MT_Storage::UnregisterThread()
{
	if ( fgInitialized ) {
		// check for current allocator
		Allocator *oldAllocator = 0;

		// check for old allocator, and if any delete it
		GETTLSDATA(MT_Storage::fgAllocatorKey, oldAllocator, Allocator);
		if ( oldAllocator ) {
			StatTrace(MT_Storage.UnregisterThread, "removing Allocator:" << (long)oldAllocator << " of ThreadLocalStorage", Coast::Storage::Global());
			return !SETTLSDATA(MT_Storage::fgAllocatorKey, 0);
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
	return false;
}

Allocator *MT_Storage::MakePoolAllocator(u_long poolStorageSize, u_long numOfPoolBucketSizes, long lPoolId)
{
	StartTrace(MT_Storage.MakePoolAllocator);
	Allocator *newPoolAllocator = new MTPoolAllocator(lPoolId, poolStorageSize, numOfPoolBucketSizes);
	if (!newPoolAllocator) {
		String msg("allocation of PoolStorage: ");
		msg << (long)poolStorageSize << ", " << (long)numOfPoolBucketSizes << " failed";
		SystemLog::Error(msg);
		return newPoolAllocator;
	}
	if (newPoolAllocator->RefCnt() <= -1) {
		// we managed to allocate a pool allocator
		// but the object failed to allocate the pool storage
		delete newPoolAllocator;
		newPoolAllocator = 0;
	}
	return newPoolAllocator;
}


//---- MTStorageHooks ------------------------------------------

MTStorageHooks::MTStorageHooks()
	: fgInitialized(false)
	, fMutex("MTStorageHooksGlobalAccessMutex", Coast::Storage::Global())
{
}

MTStorageHooks::~MTStorageHooks()
{
}

Allocator *MTStorageHooks::Global()
{
	return Coast::Storage::DoGlobal();
}

MemTracker *MTStorageHooks::MakeMemTracker(const char *name, bool bThreadSafe)
{
	MemTracker *pTracker = NULL;
	if ( bThreadSafe ) {
		pTracker = new MT_MemTracker(name, 55667788);
	} else {
		pTracker = Coast::Storage::DoMakeMemTracker(name);
	}
	return pTracker;
}

Allocator *MTStorageHooks::Current()
{
	if ( fgInitialized ) {
		Allocator *wdallocator = 0;
		// determine which allocator to use
		if ( GETTLSDATA(MT_Storage::fgAllocatorKey, wdallocator, Allocator) && wdallocator ) {
			return wdallocator;
		}
	}
	return Global();
}

void MTStorageHooks::Initialize()
{
	if ( !fgInitialized ) {
		fgInitialized = true;
	}
}

void MTStorageHooks::Finalize()
{
	if ( fgInitialized ) {
		fgInitialized = false;
	}
}
