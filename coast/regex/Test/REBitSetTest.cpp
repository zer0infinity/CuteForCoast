/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "REBitSet.h"
#include "REBitSetTest.h"
#include "SystemFile.h"
#include "Tracer.h"
#include <ctype.h>

//---- REBitSetTest ----------------------------------------------------------------
REBitSetTest::REBitSetTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(REBitSetTest.Ctor);
}

REBitSetTest::~REBitSetTest()
{
	StartTrace(REBitSetTest.Dtor);
}

void REBitSetTest::ManyTests()
{
	StartTrace(REBitSetTest.ManyTests);

	REBitSet empty;
	REBitSet full(true);
	t_assert(empty.IsSubSet(full));
	t_assert(empty.IsSubSet(empty));
	t_assert(full.IsSubSet(full));
	t_assert(!full.IsSubSet(empty));
	t_assert(full == full);
	t_assert(full.IsEqual(full));
	REBitSet one(empty);
	one.Set(1);
	t_assert(empty.IsSubSet(one));
	t_assert(!one.IsSubSet(empty));
	t_assert(one.IsSubSet(full));
	t_assert(one.IsMember(1));
	t_assert(!one.IsMember(0));
	REBitSet excepttwo(full);
	excepttwo.Unset(2);
	t_assert(!excepttwo.IsMember(2));
	t_assert(one.IsSubSet(excepttwo));
	t_assert(!excepttwo.IsSubSet(one));
	t_assert(excepttwo.IsMember(3));
	REBitSet two(full);
	t_assert(two == full);
	two ^= excepttwo;
	t_assert(two.IsMember(2));
	t_assert(!two.IsMember(3));
	t_assert(!two.IsSubSet(excepttwo));
	t_assert(!excepttwo.IsSubSet(two));
	two.Toggle(2);
	t_assert(two == empty);

	two |= REBitSet().Set(2);
	t_assert(two.IsMember(2));
	two ^= full;
	t_assert(!two.IsMember(2));
	t_assert(two == excepttwo);
}

void REBitSetTest::SetTo255()
{
	StartTrace(REBitSetTest.SetTo255);
	REBitSet s;

	s.Set('b', 255);
	t_assert(s.IsMember('b'));
	t_assert(!s.IsMember('a'));
	t_assert('a' < 'b');
	t_assert((unsigned char)'b' < (unsigned char)255);

}

void REBitSetTest::TestPredicateSet()
{
	StartTrace(REBitSetTest.TestPredicateSet);
	REBitSet s;

	s.Set(isalpha);
	for (unsigned i = 0; i < 256 ; ++i) {
		assertEqual((isalpha(i) != 0), s.IsMember(i));
	}
}

void REBitSetTest::GeneratePosixSet(std::ostream &cppfile, const char *pcName, REBitSet::Predicate predicate)
{
	REBitSet s;
	s.Set(predicate);

	cppfile << std::endl;
	cppfile << "const unsigned long _dummy_" << pcName << " [] = {" << std::endl;
	cppfile << s << "};" << std::endl;
	cppfile << "const REBitSet " << pcName << "(_dummy_" << pcName << ");" << std::endl;
}

// auxiliary predicates for posix and special escape handling
static int myIsBlank(int c)
{
	return (' ' == c || '\t' == c);
}

static int myIsNoDigit(int c)
{
	return ! isdigit(c);
}
static int myIsNoSpace(int c)
{
	return ! isspace(c);
}
static int myIsNoAlnum(int c)
{
	return ! isalnum(c);
}

// may insert special cases for Swiss lowercase/uppercase letters as well
// like myIsCHAlpha, myIsCHLower, myIsCHUpper

void REBitSetTest::GeneratePosixSets()
{
	StartTrace(REBitSetTest.GeneratePosixSets);

	// to be used by RECompiler to simplify charset handling for
	// POSIX character sets and special escapes

	std::ostream *os = coast::system::OpenOStream("REPosixBitSets", "h");
	if (t_assertm(os != NULL, "cannot write File REPosixBitSets.h")) {
		*os << "//(c) copyright ifs 2005, all rights reserved" << std::endl;
		*os << "//automatically generatest by REBitSetTest::GeneratePosixSets" << std::endl;

		GeneratePosixSet(*os, "gcSetIsAlnum", isalnum);
		GeneratePosixSet(*os, "gcSetIsAlpha", isalpha);
		GeneratePosixSet(*os, "gcSetIsAscii", isascii);
		GeneratePosixSet(*os, "gcSetIsBlank", myIsBlank);
		GeneratePosixSet(*os, "gcSetIsCntrl", iscntrl);
		GeneratePosixSet(*os, "gcSetIsDigit", isdigit);
		GeneratePosixSet(*os, "gcSetIsGraph", isgraph);
		GeneratePosixSet(*os, "gcSetIsLower", islower);
		GeneratePosixSet(*os, "gcSetIsPrint", isprint);
		GeneratePosixSet(*os, "gcSetIsPunct", ispunct);
		GeneratePosixSet(*os, "gcSetIsSpace", isspace);
		GeneratePosixSet(*os, "gcSetIsUpper", isupper);
		GeneratePosixSet(*os, "gcSetIsXDigit", isxdigit);
		// the following three are for special escapes \D \W \S
		GeneratePosixSet(*os, "gcSetIsNoDigit", myIsNoDigit);
		GeneratePosixSet(*os, "gcSetIsNoAlnum", myIsNoAlnum);
		GeneratePosixSet(*os, "gcSetIsNoSpace", myIsNoSpace);

		delete os;
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *REBitSetTest::suite ()
{
	StartTrace(REBitSetTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, REBitSetTest, ManyTests);
	ADD_CASE(testSuite, REBitSetTest, SetTo255);
	ADD_CASE(testSuite, REBitSetTest, TestPredicateSet);
	ADD_CASE(testSuite, REBitSetTest, GeneratePosixSets);

	return testSuite;

}
