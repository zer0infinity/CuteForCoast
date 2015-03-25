/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STL_pool_allocator_H
#define _STL_pool_allocator_H

#include "STLStorage.h"//lint !e537

namespace stlstorage
{
	// use pool_allocator type for std::list container types which allocate elements one by one
	template <
	typename T,
			 typename UserAllocator = itostorage::BoostPoolUserAllocatorGlobal,
			 unsigned NextSize = 32 >
	class pool_allocator
	{
	public:
		typedef T value_type;
		typedef UserAllocator user_allocator;
		BOOST_STATIC_CONSTANT(unsigned, next_size = NextSize);

		typedef pool_refcounted<UserAllocator> pool_refcounted_type;
		typedef typename pool_refcounted_type::pool_refcount_storer int_pool_instance_type;

		typedef value_type *pointer;
		typedef const value_type *const_pointer;
		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef typename pool_refcounted_type::int_pool_type pool_type;
		typedef typename pool_type::size_type size_type;
		typedef typename pool_type::difference_type difference_type;
		typedef pool_allocator<T, UserAllocator, NextSize> ThisType;

		template <typename U>
		struct rebind {
			typedef pool_allocator<U, UserAllocator, NextSize> other;
		};

	private:
		ThisType &operator=(const ThisType &);

	public:
		/* constructors and destructor
		*/
		pool_allocator()
			: fpIntPool() {
			_StartTrace1(pool_allocator.pool_allocator, "this:" << static_cast<long>(this) << " sizeof(T):" << sizeof(T));
			void *pMem = (void *)UserAllocator::malloc(sizeof(pool_refcounted_type));
			fpIntPool = int_pool_instance_type( new (pMem) pool_refcounted_type(sizeof(T), NextSize) );
			_Trace("IntPool @" << (long)fpIntPool.get());
		}

		pool_allocator(const ThisType &aAllocator)
			: fpIntPool(aAllocator.fpIntPool) {
			_StartTrace1(pool_allocator.pool_allocator, "copy this:" << static_cast<long>(this) << " sizeof(T):" << sizeof(T) << " other:" << (long)&aAllocator << " sizeof(other):" << sizeof(typename ThisType::value_type));
			_Trace("IntPool @" << (long)fpIntPool.get());
		}

		template <typename U>
		pool_allocator(const pool_allocator<U, UserAllocator, NextSize>& aAllocator)
			: fpIntPool() {
			_StartTrace1(pool_allocator.pool_allocator, "copy other type this:" << static_cast<long>(this) << " sizeof(T):" << sizeof(T) << " other:" << (long)&aAllocator << " sizeof(U):" << sizeof(typename pool_allocator<U, UserAllocator, NextSize>::value_type));
			if ( aAllocator.fpIntPool.get() ) {
				fpIntPool = aAllocator.fpIntPool->Clone(sizeof(T), NextSize);
			} else {
				_Trace("no pool created for other type ??");
				void *pMem = (void *)UserAllocator::malloc(sizeof(pool_refcounted_type));
				fpIntPool = int_pool_instance_type( new (pMem) pool_refcounted_type(sizeof(T), NextSize) );
			}
			_Trace("IntPool @" << (long)fpIntPool.get());
		}

		~pool_allocator() {
			_StatTrace(pool_allocator.~pool_allocator, "this:" << static_cast<long>(this) << " IntPool:" << (long)fpIntPool.get(), coast::storage::Current());
		}

		static pointer address(reference r) {
			return &r;
		}
		static const_pointer address(const_reference s) {
			return &s;
		}
		static size_type max_size() {
			return (std::numeric_limits<size_type>::max)();
		}
		void construct(const pointer ptr, const value_type &t) {
			new (ptr) T(t);
		}
		void destroy(const pointer ptr) {
			ptr->~T();
			(void) ptr; // avoid unused variable warning
		}

		bool operator==(const pool_allocator &) const {
			return true;
		}
		bool operator!=(const pool_allocator &) const {
			return false;
		}

		pointer allocate(const size_type n) throw(std::bad_alloc) {
			const pointer ptr = static_cast<pointer>( fpIntPool->ordered_malloc(n) );
			_StatTrace(pool_allocator.allocate_n, "this:" << static_cast<long>(this) << " ptr:" << static_cast<long>(ptr) << " size:" << static_cast<long>(n) << '*' << sizeof(T) << " calling ordered_malloc " << " fpIntPool:" << (long)fpIntPool.operator->(), coast::storage::Current());
			if (ptr == 0) {
				throw std::bad_alloc();
			}
			return ptr;
		}
		pointer allocate(const size_type n, const void *const) throw(std::bad_alloc) {
			_StatTrace(pool_allocator.allocate_n_void, "this:" << static_cast<long>(this) << " size:" << static_cast<long>(n) << '*' << sizeof(T) << " void*", coast::storage::Current());
			return allocate(n);
		}
		void deallocate(const pointer ptr, const size_type n) {
			_StatTrace(pool_allocator.deallocate_ptr_n, "this:" << static_cast<long>(this) << " ptr:" << static_cast<long>(ptr) << " size:" << static_cast<long>(n) << '*' << sizeof(T) << " calling ordered_free " << " fpIntPool:" << (long)fpIntPool.operator->(), coast::storage::Current());
#ifdef BOOST_NO_PROPER_STL_DEALLOCATE
			if (ptr == 0 || n == 0) {
				return;
			}
#endif
			fpIntPool->ordered_free(ptr, n);
		}
		int_pool_instance_type fpIntPool;
	};
	template < typename T > struct pool_allocator_global : public stlstorage::pool_allocator<T, itostorage::BoostPoolUserAllocatorGlobal, 32 > {};
	template < typename T > struct pool_allocator_current : public stlstorage::pool_allocator<T, itostorage::BoostPoolUserAllocatorCurrent, 32 > {};
}

#endif
