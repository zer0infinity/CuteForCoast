/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Queue_H
#define _Queue_H

#include "STLStorage.h"
#include "Threads.h"
#include "DiffTimer.h"
#include "MT_Storage.h"
#include <limits>
#include "ITOTypeTraits.h"	// for demangle

//---- Queue ----------------------------------------------------------
//! Base class for simple, thread-safe, container based queue
/*!
Queue elements are represented using either by objects of their type or Anythings. The internal queue itself is either a std::container or an Anything which allows
simple handling.
Statistics can be made by evaluating the returned Anything from GetStatistics(). The following slots are defined:
<pre>
{
	/QueueSize		long		size of the queue
	/CurrentSize	long		current element count in queue
	/MaxLoad		long		maximum number of entries
	/CreateTime {				time when Q was created
		/sec		long		seconds since 1.1.1970
		/usec		long		microseconds of current second
	}
	/StatisticTime {			time when statistics were queried
		/sec		long		seconds since 1.1.1970
		/usec		long		microseconds of current second
	}
	/LifeTime {					life time of the queue
		/sec		long		seconds difference
		/usec		long		microseconds difference
	}
	/PutCount		long		how many elements were put since queue was instantiated
	/GetCount		long		how many elements were get since queue was instantiated
} </pre>
*/
template <
class TElementType,
	  class TListStorageType
	  >
class QueueBase : public IFAObject, public coast::AllocatorNewDelete
{
	friend class QueueTest;
public:
	typedef TElementType ElementType;
	typedef ElementType &ElementTypeRef;
	typedef ElementType const& ConstElementTypeRef;
	typedef TListStorageType ListStorageType;
	typedef ListStorageType &ListStorageTypeRef;
	typedef QueueBase<ElementType, ListStorageType> ThisType;
	typedef long size_type;

	QueueBase(const char *name, size_type lQueueSize = std::numeric_limits<long>::max(), Allocator *pAlloc = coast::storage::Global())
		: fName(name, -1, coast::storage::Global())
		, fAllocator(pAlloc)
		, fQueueSize(lQueueSize)
		, fSemaFullSlots(0L)
		, fSemaEmptySlots(fQueueSize)
		, fPutCount(0L)
		, fGetCount(0L)
		, fMaxLoad(0L)
		, fBlockingPutCount(0L)
		, fBlockingGetCount(0L)
		, fAlive(0xf007f007)
		, feBlocked(eBothSides)
		, fQueueLock("QueueMutex", coast::storage::Global())
		, fBlockedLock("BlockedLock", coast::storage::Global())
		, fBlockingPutLock("BlockingPutLock", coast::storage::Global())
		, fBlockingGetLock("BlockingGetLock", coast::storage::Global())
		, fContainer()
		, fQueueStartTime(DiffTimer::eMicroseconds) {
		StartTrace1(Queue.Queue, "queue size:" << lQueueSize);
		UnBlock();
	}

	~QueueBase() {
		StartTrace(Queue.~Queue);
		// block any pending interface accesses
		Block();

		LockUnlockEntry me(fQueueLock);
		// mark that we are no longer alive
		fAlive = 0x00dead00;

		long lSize(IntGetSize());
		if ( lSize > 0L ) {
			SYSWARNING("Destruction of non-empty queue [" << fName << "]! still " << lSize << " Elements in Queue!");
			ListStorageType anyElements;
			IntEmptyQueue(anyElements);
		}
	}

	/*! Status code of queue Put and Get operations. As we support blocking of either read and/or write side of the queue, we must return appropriate codes telling the caller what happened. A simple boolean value is far not enough to find out why an operation failed. */
	enum StatusCode {
		eSuccess = 0,								//!< put or get was successful
		eEmpty = 1,									//!< the queue did not contain any elements when accessing it
		eFull = eEmpty << 1,						//!< the queue was already full when trying to put an element
		eBlocked = eFull << 1,						//!< the requested queue side is not accessible
		eError = eBlocked << 1,						//!< internal error occurred
		eAcquireFailed = eError << 1,				//!< internal error about acquiring a semaphore
		eTryAcquireFailed = eAcquireFailed << 1,	//!< internal error about trying to acquire a semaphore
		eDead = eTryAcquireFailed << 1,				//!< queue already destructed
	};

	//! Put element of type ConstElementTypeRef into queue
	/*! Either blocking or non-blocking calls are possible by setting bTryLock appropriately. By default, blocking calls are made.
		When the queue will get shut down, blocking callers will get released and informed by StatusCode::eBlocked
		\param anyElement element to put into queue
		\param bTryLock specify non-/blocking call, when set to true and the queue is already full, the method will exit with an appropriate StatusCode
		\return depending on internal state, a corresponding code will be returned */
	StatusCode Put(ConstElementTypeRef anyElement, bool bTryLock=false) {
		StartTrace(Queue.Put);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(ePutSide) ) {
			if ( bTryLock ) {
				eRet = /* eTryAcquireFailed |*/ eFull;
				if ( fSemaEmptySlots.TryAcquire() ) {
					eRet = DoPut(anyElement);
				}
			} else {
				LockedValueIncrementDecrementEntry ce(fBlockingPutLock, fBlockingPutCond, fBlockingPutCount);
				// need double checking here because of possible race condition during destruction on fBlockingPutLock
				if ( IsAlive() && fSemaEmptySlots.Acquire() ) {
					eRet = eDead;
					if ( IsAlive() ) {
						eRet = DoPut(anyElement);
					} else {
						// must release semaphore again because we did not put an element
						fSemaEmptySlots.Release();
					}
				}
			}
		}
		return eRet;
	}

	//! Get element of type ElementTypeRef from queue
	/*! Either blocking or non-blocking calls are possible by setting bTryLock appropriately. By default, blocking calls are made.
		When the queue will get shut down, blocking callers will get released and informed by StatusCode::eBlocked
		\param anyElement element to get from queue
		\param bTryLock specify non-/blocking call, when set to true and the queue is empty, the method will exit with an appropriate StatusCode
		\return depending on internal state, a corresponding code will be returned */
	StatusCode Get(ElementTypeRef anyElement, bool bTryLock = false) {
		StartTrace(Queue.Get);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(eGetSide) ) {
			if ( bTryLock ) {
				eRet = /*eTryAcquireFailed |*/ eEmpty;
				if ( fSemaFullSlots.TryAcquire() ) {
					eRet = DoGet(anyElement);
				}
			} else {
				LockedValueIncrementDecrementEntry ce(fBlockingGetLock, fBlockingGetCond, fBlockingGetCount);
				// need double checking here because of possible race condition during destruction on fBlockingGetLock
				eRet = eAcquireFailed;
				if ( IsAlive() && fSemaFullSlots.Acquire() ) {
					eRet = eDead;
					if ( IsAlive() ) {
						eRet = DoGet(anyElement);
					} else {
						// must release semaphore again because we did not get an element
						fSemaFullSlots.Release();
					}
				}
			}
		}
		return eRet;
	}

	//! Remove all elements from queue and put them into the given container
	/*! \param anyElement destination container to put removed elements into */
	template < class DestListType >
	void EmptyQueue(DestListType &anyElements) {
		StartTrace(Queue.EmptyQueue);
		if ( !IsBlocked() ) {
			LockUnlockEntry me(fQueueLock);
			this->IntEmptyQueue(anyElements);
		}
	}

	//! Return current number of elements in queue
	/*! \return number of elements in queue
		\return 0 returned if queue is empty
		\return -1 in case the queue has shut down already */
	long GetSize() {
		StartTrace(Queue.GetSize);
		long lSize = -1;
		if ( !IsBlocked() ) {
			LockUnlockEntry me(fQueueLock);
			lSize = IntGetSize();
		}
		return lSize;
	}

	size_type capacity() const {
		return fQueueSize;
	}

	//! Return lifetime statistics of queue
	/*! \param anyStatistics container to contain statistic values */
	void GetStatistics(Anything &anyStatistics) {
		StartTrace(Queue.GetStatistics);
		if ( IsAlive() ) {
			{
				LockUnlockEntry me(fQueueLock);
				anyStatistics["QueueSize"] = fQueueSize;
				anyStatistics["MaxLoad"] = fMaxLoad;
				anyStatistics["PutCount"] = static_cast<long>(fPutCount);
				anyStatistics["GetCount"] = static_cast<long>(fGetCount);
				anyStatistics["CurrentSize"] = IntGetSize();
			}
			anyStatistics["CreateTime"]["sec"] = static_cast<long>(DiffTimer::Scale(fQueueStartTime.getRawStartTime(), DiffTimer::eSeconds));
			anyStatistics["StatisticTime"]["sec"] = static_cast<long>(DiffTimer::Scale(DiffTimer::getCurrentRawTime(), DiffTimer::eSeconds));
			DiffTimer::tTimeType elapsedTimeSinceStart = fQueueStartTime.RawDiff();
			long secondsSinceStart = static_cast<long>(DiffTimer::Scale(elapsedTimeSinceStart, DiffTimer::eSeconds));
			anyStatistics["LifeTime"]["sec"] = secondsSinceStart;
			anyStatistics["LifeTime"]["usec"] = static_cast<long>(DiffTimer::Scale(elapsedTimeSinceStart, DiffTimer::eMicroseconds)-(secondsSinceStart*DiffTimer::eMicroseconds));
			TraceAny(anyStatistics, "statistics");
		}
	}

	//! Check if queue has not been destructed yet
	/*! \return true in case the queue can still be used */
	inline bool IsAlive() {
		return fAlive == 0xf007f007;
	}

	/*! Code to specify queue side to modify */
	enum BlockingSide {
		eNone = 0,							//!< neither Put nor Get side will get modified
		ePutSide = 1,						//!< input side of queue should get modified
		eGetSide = ePutSide << 1,			//!< output side of queue should get modified
		eBothSides = ePutSide | eGetSide,	//!< both sides of queue should get modified
	};

	//! Check if the given queue side (put/get) is blocked for access
	/*! \param aSide which side has to be checked
		\return true in case the specified side is not available for access
		\return false when the specified side is ready for access */
	bool IsBlocked(BlockingSide aSide = eBothSides) {
		StartTrace(Queue.IsBlocked);
		if ( IsAlive() ) {
			// need only a read lock here
			LockUnlockEntry rwe(fBlockedLock, RWLock::eReading);
			return ( ( feBlocked & aSide ) == aSide );
		}
		return true;
	}

	//! Block the specified queue side (put/get) for access. If a call is made to a blocked queue side, the method will return immediately and set the return code to StatusCode::eBlocked.
	/*! \param aSide which side should be blocked */
	void Block(BlockingSide aSide = eBothSides) {
		StartTrace1(Queue.Block, "side:" << aSide);
		if ( IsAlive() ) {
			{
				// need a write lock here
				LockUnlockEntry rwe(fBlockedLock, RWLock::eWriting);
				feBlocked = ( BlockingSide )( feBlocked | ( aSide & eBothSides ) );
			}
			// release potentially waiting putters/getters
			LockUnlockEntry me(fQueueLock);
			if ( aSide & ePutSide ) {
				IntReleaseBlockedPutters();
			}
			if ( aSide & eGetSide ) {
				IntReleaseBlockedGetters();
			}
		}
	}

	//! Unblock the specified queue side (put/get) for access.
	/*! \param aSide which side should be unblocked */
	void UnBlock(BlockingSide aSide = eBothSides) {
		StartTrace1(Queue.UnBlock, "side:" << aSide);
		if ( IsAlive() ) {
			// need a write lock here
			LockUnlockEntry rwe(fBlockedLock, RWLock::eWriting);
			feBlocked = ( BlockingSide )( feBlocked & ( ~aSide & eBothSides ) );
		}
	}

	//! Cloning of a queue is not allowed.
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return NULL;
	}

	String typeName() const {
		return coast::utility::demangle<String>(typeid(ListStorageType).name()).Append('[').Append(coast::utility::demangle<String>(typeid(ElementType).name())).Append(']');
	}

protected:
	//! Internal method to put an element of type ConstElementTypeRef into queue
	/*! At this level, it is guaranteed that we can put an element because the caller was able to acquire the semaphore.
		The element will get pushed into the underlying container.
		\param anyElement element to put into queue
		\return depending on internal state, a corresponding code will be returned */
	StatusCode DoPut(ConstElementTypeRef anyElement) {
		StartTrace(Queue.DoPut);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(ePutSide) ) {
			LockUnlockEntry me(fQueueLock);
			coast::threading::TLSEntry<Allocator> forceGlobalStorage(MT_Storage::getAllocatorKey(), fAllocator);
			fContainer.push_back(anyElement);
			++fPutCount;
			fMaxLoad = std::max( fMaxLoad, (long)fContainer.size() );
			fSemaFullSlots.Release();
			eRet = eSuccess;
		}
		return eRet;
	}

	//! Internal method to get an element of type ElementTypeRef from queue
	/*! At this level, it is guaranteed that we can put an element because the caller was able to acquire the semaphore.
		The element will get popped from the underlying container.
		\param anyElement element to get from queue
		\return depending on internal state, a corresponding code will be returned */
	StatusCode DoGet(ElementTypeRef anyElement) {
		StartTrace(Queue.DoGet);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(eGetSide) ) {
			LockUnlockEntry me(fQueueLock);
			coast::threading::TLSEntry<Allocator> forceGlobalStorage(MT_Storage::getAllocatorKey(), fAllocator);
			if ( fContainer.size() ) {
				anyElement = fContainer.front();	//! \todo change here so that it's not restricted only for Anything's - use any_cast
				fContainer.pop_front();
				++fGetCount;
				fSemaEmptySlots.Release();
				eRet = eSuccess;
			} else {
				// we can only get here if something curious happens with semaphore handling
				// because we were able to acquire a full-slot just before, something must have happened at this point
				// which should not be possible by its design
				// -> someone is destructing the queue without blocking first?
				SYSERROR("accessed empty Queue!?");
				eRet = ( StatusCode )( eEmpty | eError );
			}
		}
		return eRet;
	}

	//! Return current number of elements in queue
	/*! \return number of elements in underlying container */
	long IntGetSize() {
		StartTrace(Queue.IntGetSize);
		long lRet(fContainer.size());
		Trace("Anything  size:" << lRet);
		Trace("(Put-Get) size:" << (long)(fPutCount - fGetCount));
		return lRet;
	}

	//! Move all elements from queue into the given container
	/*! \param anyElements destination container to move elements into */
	template < class DestListType >
	void IntEmptyQueue(DestListType &anyElements) {
		StartTrace(Queue.IntEmptyQueue);
		long lSize(IntGetSize());
		Trace("current queue size:" << lSize);
		// try to optimize by using swap to exchange internal with external list
		// => NO, it would copy the allocator too, which is dangerous in case we use PoolAllocators belonging to threads
		while ( --lSize >= 0 ) {
			fSemaFullSlots.TryAcquire();
			moveElement(fContainer, anyElements);
			fSemaEmptySlots.Release();
		}
		Trace("elements removed:" << (long)anyElements.size());
	}

	//! internal method to move element from source to destination container
	/*! Depending on underlying container, this method can be optimized
		\param aFrom source container
		\param aTo destination container */
	template < class DestListType >
	void moveElement(ListStorageTypeRef aFrom, DestListType &aTo) {
		aTo.push_back(aFrom.front());
		aFrom.pop_front();
	}

	//! internal method to release all callers to the Put method
	/*! To keep track of Put method callers, a LockedValueIncrementDecrementEntry will be used.
		This is achieved by releasing the semaphore for any caller who currently entered the method which will then be able to acquire the semaphore but then
		needs to exit the method due to its blocked state */
	void IntReleaseBlockedPutters() {
		StartTrace(Queue.IntReleaseBlockedPutters);
		LockUnlockEntry sme(fBlockingPutLock);
		if ( fBlockingPutCount ) {
			SYSWARNING("waking up " << fBlockingPutCount << " blocked Put() callers");
		}
		while ( fBlockingPutCount ) {
			Trace("releasing one of " << fBlockingPutCount << " blocked Putters");
			fSemaEmptySlots.Release();
			// allow signalled thread to decrement the fBlockingPutCount after failed return from Put
			fBlockingPutCond.Wait(fBlockingPutLock);
		}
	}

	//! internal method to release all callers to the Get method
	/*! To keep track of Get method callers, a LockedValueIncrementDecrementEntry will be used.
		This is achieved by releasing the semaphore for any caller who currently entered the method which will then be able to acquire the semaphore but then
		needs to exit the method due to its blocked state */
	void IntReleaseBlockedGetters() {
		StartTrace(Queue.IntReleaseBlockedGetters);
		LockUnlockEntry sme(fBlockingGetLock);
		if ( fBlockingGetCount ) {
			SYSINFO("waking up " << fBlockingGetCount << " blocked Get() callers");
		}
		while ( fBlockingGetCount ) {
			Trace("releasing one of " << fBlockingGetCount << " blocked Getters");
			fSemaFullSlots.Release();
			// allow signalled thread to decrement the fBlockingGetCount after failed return from Get
			fBlockingGetCond.Wait(fBlockingGetLock);
		}
	}

	String		fName;
	Allocator	*fAllocator;
	size_type	fQueueSize;
	Semaphore	fSemaFullSlots, fSemaEmptySlots;
	ul_long		fPutCount, fGetCount;
	long		fMaxLoad, fBlockingPutCount, fBlockingGetCount;
	u_long		fAlive;
	BlockingSide feBlocked;
	SimpleMutex	fQueueLock;
	RWLock		fBlockedLock;
	SimpleMutex fBlockingPutLock, fBlockingGetLock;
	SimpleMutex::ConditionType fBlockingPutCond, fBlockingGetCond;
	ListStorageType fContainer;
	DiffTimer	fQueueStartTime;
};

//! Stl-container based queue
template <
class TElementType,
	  class TListStorageType
	  >
class Queue : public QueueBase<TElementType, TListStorageType>
{
	friend class QueueTest;
public:
	typedef TElementType ElementType;
	typedef ElementType &ElementTypeRef;
	typedef TListStorageType ListStorageType;
	typedef ListStorageType &ListStorageTypeRef;
	typedef QueueBase<ElementType, ListStorageType> BaseType;
	typedef Queue<ElementType, ListStorageType> ThisType;
	typedef typename BaseType::size_type size_type;

	Queue(const char *name, size_type lQueueSize = std::numeric_limits<long>::max(), Allocator *pAlloc = coast::storage::Global())
		: BaseType(name, lQueueSize, pAlloc) {
		StatTrace(Queue.Queue, "generic", coast::storage::Current());
	}
};

//! Anything based queue, internal storage type still to be specified
template <
class TElementType
>
class Queue<TElementType, Anything> : public QueueBase<TElementType, Anything>
{
	friend class QueueTest;
public:
	typedef TElementType ElementType;
	typedef ElementType &ElementTypeRef;
	typedef Anything ListStorageType;
	typedef ListStorageType &ListStorageTypeRef;
	typedef QueueBase<ElementType, ListStorageType> BaseType;
	typedef Queue<ElementType, ListStorageType> ThisType;
	typedef typename BaseType::size_type size_type;

	Queue(const char *name, size_type lQueueSize = std::numeric_limits<long>::max(), Allocator *pAlloc = coast::storage::Global())
		: BaseType(name, lQueueSize, pAlloc) {
		StatTrace(Queue.Queue, "Anything", coast::storage::Current());
		this->fContainer.SetAllocator(pAlloc);
	}
};

//! Anything based queue using Anything as elements
typedef Queue<Anything, Anything> AnyQueueType;

#endif
