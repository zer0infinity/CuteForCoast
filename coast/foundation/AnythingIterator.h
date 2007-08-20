/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef AnythingIterator_H
#define AnythingIterator_H

#include "config_foundation.h"
#include <iterator>

#if (__GNUC__ == 2 &&  __GNUC_MINOR__ <= 95)
typedef std::random_access_iterator<class Anything, ptrdiff_t> IteratorBase;
#else
typedef std::iterator<std::random_access_iterator_tag, class Anything> IteratorBase;
#endif

class EXPORTDECL_FOUNDATION Anything_iterator : public IteratorBase
{
	friend class Anything;
	friend class Anything_const_iterator;
protected:
	Anything &a;
	long	 position;
	Anything_iterator(Anything &any, long pos = 0)
		: a(any), position(pos) { } // should increase a's refcount here and in copy-ctor for robustness!
public:
	bool operator==(const Anything_iterator &r) const ;
	bool operator!=(const Anything_iterator &r) const {
		return !(*this == r);
	}
	bool operator<(const Anything_iterator &r) const ;
	bool operator>=(const Anything_iterator &r) const {
		return !(*this < r);
	}
	bool operator>(const Anything_iterator &r) const {
		return r < *this;
	}
	bool operator<=(const Anything_iterator &r) const {
		return !(r < *this);
	}
	reference operator*() const;
	reference operator[](difference_type index) const;
	difference_type operator-(const Anything_iterator &r) const;
	Anything_iterator operator+(difference_type index) const {
		return Anything_iterator(a, position + index);
	}
	Anything_iterator operator-(difference_type index) const {
		return Anything_iterator(a, position - index);
	}
	Anything_iterator &operator++() {
		++position;
		return *this;
	}
	Anything_iterator &operator--() {
		--position;
		return *this;
	}
	Anything_iterator &operator+=(difference_type index) {
		position += index;
		return *this;
	}
	Anything_iterator &operator-=(difference_type index) {
		position -= index;
		return *this;
	}
};

#if (__GNUC__ == 2 &&  __GNUC_MINOR__ <= 95)
typedef std::random_access_iterator<const class Anything, ptrdiff_t> ConstIteratorBase;
#else
typedef std::iterator<std::random_access_iterator_tag, class Anything, ptrdiff_t, const class Anything *, const class Anything &> ConstIteratorBase;
#endif

// no direct support for const_iterators, need to spell out std::iterator template parameters
class EXPORTDECL_FOUNDATION Anything_const_iterator : public ConstIteratorBase
{
	friend class Anything;
protected:
	const Anything &a;
	long	 position;
	Anything_const_iterator(const Anything &any, long pos = 0)
		: a(any), position(pos) { } // should increase a's refcount here and in copy-ctor for robustness!

public:
	Anything_const_iterator(const Anything_iterator &r)
		: a(r.a), position(r.position) { } // should increase a's refcount here and in copy-ctor for robustness!
	bool operator==(const Anything_const_iterator &r) const ;
	bool operator!=(const Anything_const_iterator &r) const {
		return !(*this == r);
	}
	bool operator<(const Anything_const_iterator &r) const ;
	bool operator>=(const Anything_const_iterator &r) const {
		return !(*this < r);
	}
	bool operator>(const Anything_const_iterator &r) const {
		return r < *this;
	}
	bool operator<=(const Anything_const_iterator &r) const {
		return !(r < *this);
	}
	const Anything &operator*() const;
	reference operator[](difference_type index) const;
	difference_type operator-(const Anything_const_iterator &r) const;
	Anything_const_iterator operator+(difference_type index) const {
		return Anything_const_iterator(a, position + index);
	}
	Anything_const_iterator operator-(difference_type index) const {
		return Anything_const_iterator(a, position - index);
	}
	Anything_const_iterator &operator++() {
		++position;
		return *this;
	}
	Anything_const_iterator &operator--() {
		--position;
		return *this;
	}
};
// do we need reverse-iterators, or can we rely on a std adapter for iterators?

#endif
