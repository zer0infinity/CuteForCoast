/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringTest.h"
#include "SystemFile.h"
#include "StringStream.h"
#include "Tracer.h"

using namespace coast;

#include <limits>

StringTest::StringTest() : fShort("short"), fLong("a somewhat longer string") {
	StartTrace(StringTest.setUp);
}

void StringTest::runAllTests(cute::suite &s) {
	StartTrace(StringTest.suite);
	s.push_back(CUTE_SMEMFUN(StringTest, constructors));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsChar));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsCharChain));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsNullPtr));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsString));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsLong));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsDouble));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsAsHex));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsTwoHexAsChar));
	s.push_back(CUTE_SMEMFUN(StringTest, appendsFile));       // ??????
	s.push_back(CUTE_SMEMFUN(StringTest, appendsWithDelimiter));
	s.push_back(CUTE_SMEMFUN(StringTest, methods));
	s.push_back(CUTE_SMEMFUN(StringTest, TestCapacity));
	s.push_back(CUTE_SMEMFUN(StringTest, GetLine));
	s.push_back(CUTE_SMEMFUN(StringTest, OptimizedConstructorOrAssignment));
	s.push_back(CUTE_SMEMFUN(StringTest, trimFrontEmpty));
	s.push_back(CUTE_SMEMFUN(StringTest, EmptyAllocatorTest));
	s.push_back(CUTE_SMEMFUN(StringTest, DumpAsHexTest));
	s.push_back(CUTE_SMEMFUN(StringTest, ReplaceTest));
	s.push_back(CUTE_SMEMFUN(StringTest, TestFirstCharOf));
	s.push_back(CUTE_SMEMFUN(StringTest, TestLastCharOf));
	s.push_back(CUTE_SMEMFUN(StringTest, TestContainsCharAbove));
	s.push_back(CUTE_SMEMFUN(StringTest, prependWith));
	s.push_back(CUTE_SMEMFUN(StringTest, DoubleToStringTest));
}

void StringTest::constructors()
{
	StartTrace(StringTest.constructors);
	// Init a string without parameters
	String empty;
	ASSERT_EQUAL("", empty);
	ASSERT_EQUAL(empty.Length(), 0L);
	ASSERT_GREATER_EQUAL(empty.Capacity(), 0L);

	// Init a string only with the Capacity
	String emptyCap(50);
	ASSERT_EQUAL("", emptyCap);
	ASSERT ( strcmp((const char *)emptyCap, "") == 0 );
	ASSERT ( memcmp((const char *)emptyCap, "",
					  strlen((const char *)emptyCap) ) == 0 );
	ASSERT_EQUAL(emptyCap.Length(), 0L);
	ASSERT_GREATER_EQUAL(emptyCap.Capacity(), 50L);

	String emptyCap0(-1);
	ASSERT_EQUAL("", emptyCap0);
	ASSERT_EQUAL(emptyCap0.Length(), 0L);
	ASSERT_LESS_EQUAL( 0L, emptyCap0.Capacity() );

	// Init a string only with a 'charChain' and the length
	String stringStrLen1( "string test 1 ", strlen("string test 1 ") );
	ASSERT (stringStrLen1 == "string test 1 ");
	ASSERT ( memcmp( (const char *)stringStrLen1, "string test 1 ", stringStrLen1.Length() ) == 0 );
	ASSERT (stringStrLen1.Length() == (long)strlen("string test 1 "));
	ASSERT (stringStrLen1.Capacity() >= stringStrLen1.Length());

	String stringStrLen2( "string test 2 ", strlen("string test 2 ") + 1 );
	ASSERT (stringStrLen2 == "string test 2 ");
	ASSERT ( memcmp( (const char *)stringStrLen2, "string test 2 ", stringStrLen2.Length() ) == 0 );
	ASSERT ( memcmp( (const char *)stringStrLen2, "string test 2 ", strlen("string test 2 ") ) == 0 );
	ASSERT (stringStrLen2.Length() == (long)strlen("string test 2 "));
	// const char * constructor, only read until the first \000
	ASSERT (stringStrLen2.Capacity() >= stringStrLen2.Length());

	String stringStrLen3( "string test 3 ", strlen("string test 3 ") - 1 );
	ASSERT (stringStrLen3 != "string test 3 ");
	ASSERT (stringStrLen3 == "string test 3");
	ASSERT (stringStrLen3.Length() <  (long)strlen("string test 3 "));
	ASSERT (stringStrLen3.Length() == (long)strlen("string test 3"));
	ASSERT (stringStrLen3.Capacity() >= stringStrLen3.Length());
	ASSERT_EQUAL(stringStrLen3.Length() + 1, stringStrLen3.Capacity());

	String stringStrLen4( "string test 4 ", -1 );
	ASSERT (stringStrLen4 == "string test 4 ");
	ASSERT (stringStrLen4.Length() == (long)strlen("string test 4 "));
	ASSERT (stringStrLen4.Capacity() >= stringStrLen4.Length());
	ASSERT_EQUAL(stringStrLen4.Length() + 1, stringStrLen4.Capacity());

	// Init a string only with a 'charChain'
	String stringCharChain( "CharChain" );
	ASSERT_EQUAL("CharChain", stringCharChain);
	ASSERT ( memcmp( (const char *)stringCharChain, "CharChain", stringCharChain.Length() ) == 0 );
	ASSERT_EQUAL(stringCharChain.Length(), (long)strlen("CharChain"));
	ASSERT (stringCharChain.Capacity() >= stringCharChain.Length());

	// Init a string with another string
	String s("string test 4");
	String stringStr0(s);
	ASSERT (stringStr0 == s);
	ASSERT_EQUAL(stringStr0.Length(), s.Length());
	//ASSERT_EQUAL(stringStr0.Capacity(), s.Capacity());
	ASSERT (stringStr0.Capacity() >= stringStr0.Length() );
	ASSERT (s.Capacity() >= s.Length() );

	ASSERT_EQUAL(stringStr0.Length() + 1, stringStr0.Capacity());

	s = "string test 4+";		// this is an operator!!
	String stringStr01(s);
	ASSERT (stringStr01 == s);
	ASSERT_EQUAL(stringStr01.Length(), s.Length());
	// ASSERT_EQUAL(stringStr01.Capacity(), s.Capacity());			// Op = does not copy the capacity
	ASSERT (stringStr01.Capacity() >= stringStr01.Length() );
	ASSERT (s.Capacity() >= s.Length() );

	String stringStr1(empty);
	ASSERT (stringStr1 == empty);
	ASSERT_EQUAL("", stringStr1);
	ASSERT_EQUAL(stringStr1.Length(), empty.Length());
	//PS? ASSERT_EQUAL(stringStr1.Capacity(), empty.Capacity());
	ASSERT (stringStr1.Capacity() >= stringStr1.Length() );
	ASSERT (empty.Capacity() >= empty.Length() );

	String stringStr2(emptyCap);
	ASSERT (stringStr2 == emptyCap);
	ASSERT_EQUAL("", stringStr2);
	ASSERT_EQUAL(stringStr2.Length(), emptyCap.Length());
	// ASSERT_EQUAL(stringStr2.Capacity(), emptyCap.Capacity());		// ???? capacity not copied
	ASSERT_EQUAL(stringStr2.Length(), 0L);
	ASSERT (stringStr2.Capacity() >= stringStr2.Length());
	//PS? ASSERT_EQUAL(stringStr2.Capacity(), 0L);  // Why is capacity 0 ????

	String one(1);
	ASSERT (empty == one);

	String oneString("1");
	ASSERT_EQUAL("1", oneString);
	ASSERT_EQUAL(oneString.Length(), 1L);
	ASSERT_EQUAL("short", fShort);
	ASSERT (fShort != "Short");  // == fails

	String inputString("test for InputIterator constructor");
	IStringStream inputStream(inputString);
	std::istreambuf_iterator<char> end;
	String outputString(std::istreambuf_iterator<char>(inputStream), end);
	ASSERT (inputString == outputString);
	ASSERT_EQUAL(inputString.Length(), outputString.Length());
}

void StringTest::appendsChar()
{
	StartTrace(StringTest.appendsChar);
	//Mechanismen zur Festlegung der Capacity sind mir unklar ????

	// Append von Buchstaben
	//----------------------
	String str0;
	str0.Append('a');
	//PS? ASSERT_EQUAL(str0.Capacity(), 4L);
	ASSERT_EQUAL(str0.Length(), 1L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 1 );
	ASSERT ( memcmp((const char *)str0, "a", strlen( (const char *)str0)) == 0 );

	str0.Append('b');
	//PS? ASSERT_EQUAL(str0.Capacity(), 4L);
	ASSERT_EQUAL(str0.Length(), 2L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 2 );
	ASSERT ( memcmp((const char *)str0, "ab", strlen( (const char *)str0)) == 0 );

	str0.Append('c');
	//PS? ASSERT_EQUAL(str0.Capacity(), 4L);
	ASSERT_EQUAL(str0.Length(), 3L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 3 );
	ASSERT ( memcmp((const char *)str0, "abc", strlen( (const char *)str0)) == 0 );

	str0.Append('d');
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 4L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 4 );
	ASSERT ( memcmp((const char *)str0, "abcd", strlen( (const char *)str0)) == 0 );

	str0.Append('e');
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 5L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 5 );
	ASSERT ( memcmp((const char *)str0, "abcde", strlen( (const char *)str0)) == 0 );

	str0.Append('f');
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 6L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 6 );
	ASSERT ( memcmp((const char *)str0, "abcdef", strlen( (const char *)str0)) == 0 );

	str0.Append('g');
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 7L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 7 );
	ASSERT ( memcmp((const char *)str0, "abcdefg", strlen( (const char *)str0)) == 0 );

	str0.Append('h');
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 8L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 8 );
	ASSERT ( memcmp((const char *)str0, "abcdefgh", strlen( (const char *)str0)) == 0 );

	str0.Append('i');
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 9L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 9 );
	ASSERT ( memcmp((const char *)str0, "abcdefghi", strlen( (const char *)str0)) == 0 );

	str0.Append('l');
	//PS? ASSERT_EQUAL(str0.Capacity(), 22L);
	ASSERT_EQUAL(str0.Length(), 10L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 10 );
	ASSERT ( memcmp((const char *)str0, "abcdefghil", strlen( (const char *)str0)) == 0 );

	str0.Append('m');
	//PS? ASSERT_EQUAL(str0.Capacity(), 22L);
	ASSERT_EQUAL(str0.Length(), 11L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 11 );
	ASSERT ( memcmp((const char *)str0, "abcdefghilm", strlen( (const char *)str0)) == 0 );

	str0.Append('n');
	//PS? ASSERT_EQUAL(str0.Capacity(), 22L);
	ASSERT_EQUAL(str0.Length(), 12L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 12 );
	ASSERT ( memcmp((const char *)str0, "abcdefghilmn", strlen( (const char *)str0)) == 0 );

	// Wenn im Content der String ein '\0' gibt, gibt String.Length() einen inkonsistenten Wert zurueck.
	// 'String.Length' und 'strlen(Content)' stimmen nicht ueberein.  ????
	String str1;
	str1.Append( (char) 0);
	//PS? ASSERT_EQUAL(str1.Capacity(), 4L);
	ASSERT_EQUAL(str1.Length(), 1L);
	ASSERT ( str1.Capacity() >= str1.Length() );
	ASSERT ( (long)strlen( (const char *)str1 ) != str1.Length() );

	str1.Append('A');
	//PS? ASSERT_EQUAL(str1.Capacity(), 4L);
	ASSERT_EQUAL(str1.Length(), 2L);
	ASSERT ( str1.Capacity() >= str1.Length() );
	ASSERT ( (long)strlen( (const char *)str1 ) != str1.Length() );

	// Append mit Zahlen
	String str2;
	str2.Append((char)97); // 97 = 'a'
	//PS? ASSERT_EQUAL(str2.Capacity(), 4L);
	ASSERT_EQUAL(str2.Length(), 1L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( strlen( (const char *)str2 ) == 1 );
	ASSERT ( memcmp((const char *)str2, "a", strlen( (const char *)str2)) == 0 );

	str2.Append((char)98); // 98 = 'b'
	//PS? ASSERT_EQUAL(str2.Capacity(), 4L);
	ASSERT_EQUAL(str2.Length(), 2L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( strlen( (const char *)str2 ) == 2 );
	ASSERT ( memcmp((const char *)str2, "ab", strlen( (const char *)str2)) == 0 );

	str2.Append((char)99); // 99 = 'c'
	//PS? ASSERT_EQUAL(str2.Capacity(), 4L);
	ASSERT_EQUAL(str2.Length(), 3L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( strlen( (const char *)str2 ) == 3 );
	ASSERT ( memcmp((const char *)str2, "abc", strlen( (const char *)str2)) == 0 );

	// Append mit unerlaubten Zahlen
	String str3;
	str3.Append( (char)(-5) );
	str3.Append( (char)(-10) );
	str3.Append( (char)(9999999L) );
	//PS? ASSERT_EQUAL(str3.Capacity(), 4L);
	ASSERT_EQUAL(str3.Length(), 3L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( strlen( (const char *)str3 ) == 3 );
}

void StringTest::appendsNullPtr ()
/* what: tries to append a 0-ptr to a String. The behavior seems somewhat inconsistent
*/
{
	String test("0123456789");
	long oldLength = test.Length();
	long oldCapacity = test.Capacity();
	test.Append((const char *)0, 10);
	ASSERT_EQUAL(oldLength, test.Length());
	// seems pretty crazy to copy from zero or leave it unititialized
	ASSERT_LESS( test.Length(), test.Capacity() );
	ASSERT_EQUAL(oldCapacity, test.Capacity());
	// not a necessary assertion, just to make sure we know what we are doing

	test.Append((const char *)0, 5);
	ASSERT_EQUAL(oldLength, test.Length());
	// seems pretty crazy to copy from zero or leave it unititialized
	ASSERT_LESS( test.Length(), test.Capacity() );
	ASSERT_EQUAL(oldCapacity, test.Capacity());
	// capacity does not increase

	String test1("abcdefghijklmnopqrstuvwxyz");		// 26 characters
	test1.Trim(10);									// trimmed to 10
	String test2("abcdefghij");						// only 10 characters

	ASSERT_EQUAL(test1, test2);
	test1.Append((const char *)0, 15);				// does not expand capacity
	test2.Append((const char *)0, 15);				// expands capacity and copies

	ASSERT_EQUAL(test1, test2);						// compares only up to first 0
	ASSERT_EQUAL(test1.Length(), test2.Length());
	ASSERT_EQUAL(10, test1.Length());				// must stay 10

	ASSERT(memcmp((const char *)test1, (const char *)test2, test1.Length()) == 0);
	// not really consistent ...
}

void StringTest::appendsCharChain()
{
	StartTrace(StringTest.appendsCharChain);
	char   charChain[6] = { 'a', 'b', 'c', 'd', 'e', '\000' };
	// PT: if Append(char * is used, the buffer *must* be 0 terminated!

	// Capacity = 2*(Lenght+1)
	// charChain 100% gelesen
	String str0;
	str0.Append( charChain, 5 );
	//PS? ASSERT_EQUAL(str0.Capacity(), 12L);
	ASSERT_EQUAL(str0.Length(), 5L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( strlen( (const char *)str0 ) == 5 );
	ASSERT ( memcmp((const char *)str0, charChain, str0.Length() ) == 0 );

	// String.Length() != strlen((const char *)String) !!!
	// reading all of charChain plus something extra
	// (purify might generate a UMR, ABR or something the like
	String str1;
	str1.Append( (void *) charChain, 10 );
	//PS? ASSERT_EQUAL(str1.Capacity(), 22L);
	ASSERT_EQUAL(str1.Length(), 10L);
	ASSERT ( str1.Capacity() >= str1.Length() );
	ASSERT ( memcmp((const char *)str1, charChain, strlen(charChain) ) == 0 );
//	ASSERT ( memcmp((const char *)str1, charChain, max(str1.Length(), strlen(charChain)) ) == 0 );
	// PT: this is a highly dubious assertion, compare some random memory!
	// (purify generates a UMR)

	// 0 char von charChain gelesen
	String str2;
	str2.Append( charChain, 0 );
	//PS? ASSERT_EQUAL(str2.Capacity(), 0L);
	ASSERT_EQUAL(str2.Length(), 0L);
	ASSERT ( str2.Capacity() >= str2.Length() );

	// Lesen innerhalb von charChain
	String str3;
	str3.Append( charChain, 3 );
	//PS? ASSERT_EQUAL(str3.Capacity(), 8L);
	ASSERT_EQUAL(str3.Length(), 3L);
	ASSERT ( str3.Capacity() >= str3.Length() );
	ASSERT ( memcmp((const char *)str3, "abcde", str3.Length() ) == 0 );

	// Lesen innerhalb von charChain
	String str4;
	str4.Append( &charChain[3], 2 );
	//PS? ASSERT_EQUAL(str4.Capacity(), 6L);
	ASSERT_EQUAL(str4.Length(), 2L);
	ASSERT ( str4.Capacity() >= str4.Length() );
	ASSERT ( memcmp((const char *)str4, "de", str4.Length() ) == 0 );

	//Lesen innerhalb von charChain
	String str5;
	str5.Append( &charChain[4], 1 );
	//PS? ASSERT_EQUAL(str5.Capacity(), 4L);
	ASSERT_EQUAL(str5.Length(), 1L);
	ASSERT ( str5.Capacity() >= str5.Length() );
	ASSERT ( memcmp((const char *)str5, "e", str5.Length() ) == 0 );

	//  deliberately reading out of charChain
	String str6;
	str6.Append( (void *)&charChain[4], 2 );
	//PS? ASSERT_EQUAL ( 6, str6.Capacity() );
	ASSERT_EQUAL ( 2, str6.Length() );
	ASSERT ( str6.Capacity() >= str6.Length() );

	// Lesen mit komischer Laenge:  ????
	String str7;
	str7.Append( charChain, -4 );

	// 2 Mal Append
	String str8;
	str8.Append( charChain, 5 );
	str8.Append( charChain, 5 );
	//PS? ASSERT_EQUAL(str8.Capacity(), 12L);  // 2*(5+1)
	ASSERT_EQUAL(str8.Length(), 10L);
	ASSERT ( str8.Capacity() >= str8.Length() );
	ASSERT ( memcmp((const char *)str8, "abcdeabcde", str8.Length() ) == 0 );

	// Append mit anderen Stringen
	String str9, str10;
	str9.Append( "123", 3 );
	str10.Append( "456", 3 );
	str9.Append( (const char *)str10, strlen((const char *)str10) );
	//PS? ASSERT_EQUAL(str9.Capacity(), 8L);
	ASSERT_EQUAL(str9.Length(), 6L);
	ASSERT ( str9.Capacity() >= str9.Length() );
	str9.Append( (const char *)str10, strlen((const char *)str10) );
	//PS? ASSERT_EQUAL(str9.Capacity(), 20L);
	ASSERT_EQUAL(str9.Length(), 9L);
	ASSERT ( str9.Capacity() >= str9.Length() );

	// Append mit sich selbst
	String str11;
	str11.Append( "123", 3 );
	str11.Append( (const char *)str11, strlen((const char *)str11) );
	//PS? ASSERT_EQUAL(str11.Capacity(), 8L);
	ASSERT_EQUAL(str11.Length(), 6L);
	ASSERT ( str11.Capacity() >= str11.Length() );
	ASSERT ( memcmp( (const char *)str11, "123123", str11.Length() ) == 0 );

	// Append mit einem Teil von sich selbst
	String str12;
	str12.Append( "123", 3 );
	str12.Append( ((const char *)(str12) + 1), 2 );
	//PS? ASSERT_EQUAL(str12.Capacity(), 8L);
	ASSERT_EQUAL(str12.Length(), 5L);
	ASSERT ( str12.Capacity() >= str12.Length() );
	ASSERT ( memcmp( (const char *)str12, "12323", str12.Length() ) == 0 );

	// Im Content gibt es einen '\0'.  Geht's ?
	char buf[6] = { 'a', 'b', (char)0, 'c', 'd', '\000' };
	String str13;
	str13.Append( (void *)buf, 5 );
	// (void *) cast because otherwise Append does not read past 0
	//PS? ASSERT_EQUAL ( 12, str13.Capacity() );
	ASSERT_EQUAL ( 5, str13.Length() );
	ASSERT ( str13.Capacity() >= str13.Length() );
	ASSERT ( str13.Capacity() >= (long)strlen( (const char *)str13 ) );
	str13.Append( (void *)buf, 5 );
	// (void *) cast because otherwise Append does not read past 0
	//PS? ASSERT_EQUAL ( 12, str13.Capacity() );
	ASSERT_EQUAL ( 10, str13.Length() );
	ASSERT ( str13.Capacity() >= str13.Length() );
	ASSERT ( str13.Capacity() >= (long)strlen( (const char *)str13 ) );
	ASSERT_EQUAL ( strlen( buf ), strlen( (const char *)str13 ) );
	ASSERT_EQUAL ( 2, strlen( (const char *)str13 ) );
	ASSERT ( str13.Length() >= (long)strlen( (const char *)str13 ) );
}

void StringTest::appendsString()
{
	StartTrace(StringTest.appendsString);
	// Append von 2 leeren Stringen
	String str0, str1;
	str0.Append( str1 );
	ASSERT_GREATER_EQUAL(str0.Capacity(), 0L);
	ASSERT_EQUAL(str0.Length(), 0L);

	// Append von sich selbst
	String str2;
	str2.Append("123", 3);  // length = 3 --> capacity = 2*(3+1)
	str2.Append( str2 );
	//PS? ASSERT_EQUAL(str2.Capacity(), 8L);
	ASSERT_EQUAL(str2.Length(), 6L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( memcmp( (const char *)str2, "123123", str2.Length() ) == 0 );
	str2.Append( str2 );
	//PS? ASSERT_EQUAL(str2.Capacity(), 26L);
	ASSERT_EQUAL(str2.Length(), 12L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( memcmp( (const char *)str2, "123123123123", str2.Length() ) == 0 );
	str2.Append( str2 );
	//PS? ASSERT_EQUAL(str2.Capacity(), 26L);
	ASSERT_EQUAL(str2.Length(), 24L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( memcmp( (const char *)str2, "123123123123123123123123", str2.Length() ) == 0 );
	str2.Append( str2 );
	//PS? ASSERT_EQUAL(str2.Capacity(), 98L);
	ASSERT_EQUAL(str2.Length(), 48L);
	ASSERT ( str2.Capacity() >= str2.Length() );
	ASSERT ( memcmp( (const char *)str2, "123123123123123123123123123123123123123123123123", str2.Length() ) == 0 );

	// Append von 2 Stringen
	String str3, str4;
	str3.Append( "123", 3 );
	str4.Append( "456", 3 );
	str3.Append( str4 );
	//PS? ASSERT_EQUAL(str3.Capacity(), 8L);
	ASSERT_EQUAL(str3.Length(), 6L);
	ASSERT ( str3.Capacity() >= str3.Length() );
	ASSERT ( memcmp( (const char *)str3, "123456", str3.Length() ) == 0 );
	str4.Append( str3 );
	//PS? ASSERT_EQUAL(str4.Capacity(), 20L);
	ASSERT_EQUAL(str4.Length(), 9L);
	ASSERT ( str4.Capacity() >= str4.Length() );
	ASSERT ( memcmp( (const char *)str4, "456123456", str4.Length() ) == 0 );

	// Append mit einer leeren String
	String str5, str6;
	str5.Append( "123", 3);
	str5.Append( str6 );
	//PS? ASSERT_EQUAL(str5.Capacity(), 8L);
	ASSERT_EQUAL(str5.Length(), 3L);
	ASSERT ( str5.Capacity() >= str5.Length() );
	ASSERT ( memcmp( (const char *)str5, "123", str5.Length() ) == 0 );

	// Append mit einer leeren String
	String str7, str8;
	str8.Append( "123", 3 );
	str7.Append( str8 );
	//PS? ASSERT_EQUAL(str7.Capacity(), 8L);
	ASSERT_EQUAL(str7.Length(), 3L);
	ASSERT ( str7.Capacity() >= str7.Length() );
	ASSERT ( memcmp( (const char *)str7, "123", str7.Length() ) == 0 );

	// Funktionniert Append auch wenn innerhalb des Contents ein '\0' ist?
	// Alles funktionniert NUR muss man erinnern, dass 'Strlen' ungleich 'Length()' sein kann.  ????
	String str9, str10;
	char buf[5] = {'a', 'b', (char)0, 'c', 'd'};
	str9.Append( (void *) buf, 5 );
	// (void *) cast because otherwise Append stops at 0
	str10.Append( "123", 3 );
	str9.Append( str10 );
	//PS? ASSERT_EQUAL(str9.Capacity(), 12L);
	ASSERT_EQUAL(str9.Length(), 8L);
	ASSERT ( str9.Capacity() >= str9.Length() );
	ASSERT ( str9.Length() >= (long)strlen( (const char *)str9 ) );
	ASSERT ( strlen( (const char *)str9 ) == strlen(buf) );
	char bufHelp[10] = {0};
	memcpy( bufHelp, buf, sizeof(buf) );
	memcpy( &bufHelp[sizeof(buf)], "123", strlen("123") );
	ASSERT ( memcmp( (const char *)str9, bufHelp, str9.Length() ) == 0 );

	// Funktionniert Append auch wenn innerhalb des Contents ein '\0' ist?
	// char buf[5] = {'a', 'b', (char)0, 'c', 'd'};
	String str11, str12;
	str11.Append( (void *)buf, 5 );
	// (void *) cast because otherwise Append does not read past 0
	str12.Append( "123", 3 );
	str12.Append( str11 );
	//PS? ASSERT_EQUAL(str12.Capacity(), 18L);
	ASSERT ( str12.Capacity() >= str12.Length() );
	ASSERT ( strlen( (const char *)str12 ) == strlen("123") + strlen(buf) );
	memset( bufHelp, 0, sizeof(bufHelp) );
	memcpy( bufHelp, "123", strlen("123") );
	memcpy( &bufHelp[strlen("123")], buf, sizeof(buf) );
	ASSERT ( memcmp( (const char *)str12, bufHelp, str12.Length() ) == 0 );
}

void StringTest::appendsLong()
{
	StartTrace(StringTest.appendsLong);
	// Append with several long numbers
	String str0;
	str0.Append( 0L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 4L);
	ASSERT_EQUAL(str0.Length(), 1L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( memcmp( (const char *)str0, "0", str0.Length() ) == 0 );

	str0.Append( 1L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 4L);
	ASSERT_EQUAL(str0.Length(), 2L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT_EQUAL("01", str0);
	//PS? ASSERT ( memcmp( (const char *)str0, "01", str0.Length() )==0 );

	str0.Append( 2L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 4L);
	ASSERT_EQUAL(str0.Length(), 3L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "012", str0.Length() )==0 );

	str0.Append( 3L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 4L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "0123", str0.Length() )==0 );

	str0.Append( -1L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 6L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "0123-1", str0.Length() )==0 );

	str0.Append( -2L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 10L);
	ASSERT_EQUAL(str0.Length(), 8L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "0123-1-2", str0.Length() )==0 );

	str0.Append( -3L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 22L);
	//PS? ASSERT_EQUAL(str0.Length(), 10L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "0123-1-2-3", str0.Length() )==0 );

	str0.Append( 2147483647L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 22L);
	ASSERT_EQUAL(str0.Length(), 20L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "0123-1-2-32147483647", str0.Length() )==0 );

	str0.Append( -2147483647L );
	//PS? ASSERT_EQUAL(str0.Capacity(), 64L);
	//PS? ASSERT_EQUAL(str0.Length(), 31L);
	ASSERT ( str0.Capacity() >= str0.Length() );
	//PS? ASSERT ( memcmp( (const char *)str0, "0123-1-2-32147483647-2147483647", str0.Length() )==0 );
}

void StringTest::appendsDouble()
{
	StartTrace(StringTest.appendsDouble);
	// Rundungen sind sehr ungenau ???? //
	//----------------------------------//

	// === capacity 4 === Length 3 === Content 0 0
	String str0;
	str0.Append( (double)0 );
	str0.Append(' ');
	str0.Append( (double)(-0) );
	ASSERT ( str0.Capacity() >= str0.Length() );

	// === capacity 4 === Length 3 === Content 0 -0
	String str1;
	str1.Append(0.0);
	str1.Append(' ');
	str1.Append(-0.0);
	ASSERT ( str1.Capacity() >= str1.Length() );

	// === capacity 42 === Length 24 === Content 2.22045e-16 -2.22045e-16
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str2;
	str2.Append(std::numeric_limits<double>::epsilon());
	str2.Append(' ');
	str2.Append(-std::numeric_limits<double>::epsilon());
	ASSERT ( str2.Capacity() >= str2.Length() );

	// === capacity 34 === Length 26 === Content 1.79769e+308 -1.79769e+308
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str3;
	str3.Append(std::numeric_limits<double>::max());
	str3.Append(' ');
	str3.Append(-std::numeric_limits<double>::min());
	ASSERT ( str3.Capacity() >= str3.Length() );

	// === capacity 34 === Length 26 === Content 2.22507e-308 -2.22507e-308
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str4;
	str4.Append(std::numeric_limits<double>::min());
	str4.Append(' ');
	str4.Append(-std::numeric_limits<double>::min());
	ASSERT ( str4.Capacity() >= str4.Length() );

	// === capacity 42 === Length 24 === Content 1.19209e-07 -1.19209e-07
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str5;
	str5.Append(std::numeric_limits<float>::epsilon());
	str5.Append(' ');
	str5.Append(-std::numeric_limits<float>::epsilon());
	ASSERT ( str5.Capacity() >= str5.Length() );

	// === capacity 42 === Length 24 === Content 3.40282e+38 -3.40282e+38
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str6;
	str6.Append(std::numeric_limits<float>::max());
	str6.Append(' ');
	str6.Append(-std::numeric_limits<float>::max());
	ASSERT ( str6.Capacity() >= str6.Length() );

	// === capacity 42 === Length 24 === Content 1.17549e-38 -1.17549e-38
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str7;
	str7.Append(std::numeric_limits<float>::min());
	str7.Append(' ');
	str7.Append(-std::numeric_limits<float>::min());
	ASSERT ( str7.Capacity() >= str7.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e-08 -1.23457e-08
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str8;
	str8.Append(0.123456789e-7);
	str8.Append(' ');
	str8.Append(-0.123456789e-7);
	ASSERT ( str8.Capacity() >= str8.Length() );

	// === capacity 42 === Length 24 === Content 7.12346e+07 -7.12346e+07
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str9;
	str9.Append(7.123456789E+7);
	str9.Append(' ');
	str9.Append(-7.123456789E+7);
	ASSERT ( str9.Capacity() >= str9.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+15 -1.23457e+15
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str10;
	str10.Append(123456789.123456789E+7);
	str10.Append(' ');
	str10.Append(-123456789.123456789E+7);
	ASSERT ( str10.Capacity() >= str10.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+15 -1.23457e+15
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str11;
	str11.Append(123456789.123456789E+7);
	str11.Append(' ');
	str11.Append(-123456789.123456789E+7);
	ASSERT ( str11.Capacity() >= str11.Length() );

	// Die Nachkommastellen gehen nicht verloren sondern gerundet XXX.XXX
	// === capacity 18 === Length 16 === Content 123.012 -123.012
	String str12;
	str12.Append(123.0123456789);
	str12.Append(' ');
	str12.Append(-123.0123456789);
	ASSERT ( str12.Capacity() >= str12.Length() );

	// Die Nachkommastellen gehen nicht verloren sondern gerundet XXXX.XX
	// === capacity 28 === Length 16 === Content 1234.01 -1234.01
	String str13;
	str13.Append(1234.0123456789);
	str13.Append(' ');
	str13.Append(-1234.0123456789);
	ASSERT ( str13.Capacity() >= str13.Length() );

	// Die Nachkommastellen gehen verloren
	// === capacity 26 === Length 12 === Content 12345 -12345
	String str14;
	str14.Append(12345.0123456789);
	str14.Append(' ');
	str14.Append(-12345.0123456789);
	ASSERT ( str14.Capacity() >= str14.Length() );

	// Die Nachkommastellen gehen verloren
	// === capacity 30 === Length 14 === Content 123456 -123456
	String str15;
	str15.Append(123456.0123456789);
	str15.Append(' ');
	str15.Append(-123456.0123456789);
	ASSERT ( str15.Capacity() >= str15.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+06 -1.23457e+06
	String str16;
	str16.Append(1234567.0123456789);
	str16.Append(' ');
	str16.Append(-1234567.0123456789);
	ASSERT ( str16.Capacity() >= str16.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+09 -1.23457e+09
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str17;
	str17.Append(1234567890.0123456789);
	str17.Append(' ');
	str17.Append(-1234567890.0123456789);
	ASSERT ( str17.Capacity() >= str17.Length() );
}

void StringTest::appendsAsHex()
{
	StartTrace(StringTest.appendsAsHex);
	// Test Methode AppendAsHex(char)
	{
		int i;
		String str0;
		for ( i = 0; i < 256; i++ ) {
			str0.AppendAsHex( (unsigned char)i);
			str0.Append(' ');
		}

		const char exp[] = "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 "//lint !e578
				"1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 "
				"38 39 3A 3B 3C 3D 3E 3F 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 "
				"56 57 58 59 5A 5B 5C 5D 5E 5F 60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 "
				"74 75 76 77 78 79 7A 7B 7C 7D 7E 7F 80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 "
				"92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF "
				"B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD "
				"CE CF D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB "
				"EC ED EE EF F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF ";
		ASSERT ( str0.Capacity() >= str0.Length() );
		ASSERT ( str0.Length() == (256 * 3));  // '3E ':  3 Stellen per Zahl
		ASSERT ( memcmp((const char *)str0, exp, (256 * 3)) == 0);
	}

	// Test Methode AppendAsHex(const char*, long, char)
	{
		{
			String str0, str1, str2, str3;

			const unsigned char buf[3] = { 0x00, 0xA1, 0xFF };

			str0.AppendAsHex(buf, 0);
			ASSERT ( str0 == String("") );
			ASSERT_EQUAL(str0.Length(), 0L);

			str1.AppendAsHex(buf, 1);
			ASSERT ( str1 == String("00") );
			ASSERT_EQUAL(str1.Length(), 2L);

			str2.AppendAsHex(buf, 3);
			ASSERT ( str2 == String("00A1FF") );
			ASSERT_EQUAL(str2.Length(), 6L);

			str3.AppendAsHex(buf, -1);
			ASSERT ( str3 == String("") );
			ASSERT_EQUAL(str3.Length(), 0L);
		}
		{
			String str0, str1, str2, str3;

			const unsigned char buf[3] = { 0x00, 0xA1, 0xFF };

			str0.AppendAsHex(buf, 0, ' ');
			ASSERT ( str0 == String("") );
			ASSERT_EQUAL(str0.Length(), 0L);

			str1.AppendAsHex(buf, 1, ' ');
			ASSERT ( str1 == String("00") );
			ASSERT_EQUAL(str1.Length(), 2L);

			str2.AppendAsHex(buf, 3, ' ');
			ASSERT ( str2 == String("00 A1 FF") );
			ASSERT_EQUAL(str2.Length(), 8L);

			str3.AppendAsHex(buf, -1, ' ');
			ASSERT ( str3 == String("") );
			ASSERT_EQUAL(str3.Length(), 0L);
		}
	}
}

void StringTest::appendsTwoHexAsChar()
{
	StartTrace(StringTest.appendsTwoHexAsChar);
	// Test Methode AppendTwoHexAsChar(const char *)
	{
		char	a[2];
		int		i, j;
		String	str0;

		for ( i = 0; i < 16; i++ ) {
			if ( i > 9 ) {
				switch (i) {
					case 10:
						a[0] = 'a';
						break;
					case 11:
						a[0] = 'b';
						break;
					case 12:
						a[0] = 'c';
						break;
					case 13:
						a[0] = 'd';
						break;
					case 14:
						a[0] = 'e';
						break;
					case 15:
						a[0] = 'f';
						break;
				}
			} else {
				a[0] = (char)i + '0';
			}

			for ( j = 0; j < 16; j++ ) {
				if ( j > 9 ) {
					switch (j) {
						case 10:
							a[1] = 'a';
							break;
						case 11:
							a[1] = 'b';
							break;
						case 12:
							a[1] = 'c';
							break;
						case 13:
							a[1] = 'd';
							break;
						case 14:
							a[1] = 'e';
							break;
						case 15:
							a[1] = 'f';
							break;
					}
				} else {
					a[1] = (char)j + '0';
				}

				str0.AppendTwoHexAsChar(a);//lint !e645
				str0.Append(' ');
			}
		}

		String str1;
		int ch;
		for ( ch = (char)0; ch < 256; ch++ ) {
			str1.Append( (char)ch );
			str1.Append(' ');
		}

		//PS? ASSERT_EQUAL(str0.Capacity(), str1.Capacity());
		ASSERT_EQUAL(str0.Length(), str1.Length());
		ASSERT ( str0.Capacity() >= str0.Length() );
		ASSERT ( str1.Capacity() >= str1.Length() );
		ASSERT( memcmp( (const char *)str0, (const char *)str1, str0.Length() ) == 0 );

		String str2;
		for ( i = 0; i < 16; i++ ) {
			if ( i > 9 ) {
				switch (i) {
					case 10:
						a[0] = 'A';
						break;
					case 11:
						a[0] = 'B';
						break;
					case 12:
						a[0] = 'C';
						break;
					case 13:
						a[0] = 'D';
						break;
					case 14:
						a[0] = 'E';
						break;
					case 15:
						a[0] = 'F';
						break;
				}
			} else {
				a[0] = (char)i + '0';
			}

			for ( j = 0; j < 16; j++ ) {
				if ( j > 9 ) {
					switch (j) {
						case 10:
							a[1] = 'A';
							break;
						case 11:
							a[1] = 'B';
							break;
						case 12:
							a[1] = 'C';
							break;
						case 13:
							a[1] = 'D';
							break;
						case 14:
							a[1] = 'E';
							break;
						case 15:
							a[1] = 'F';
							break;
					}
				} else {
					a[1] = (char)j + '0';
				}

				str2.AppendTwoHexAsChar(a);
				str2.Append(' ');
			}
		}

		String str3;
		for ( ch = (char)0; ch < 256; ch++ ) {
			str3.Append( (char)ch );
			str3.Append(' ');
		}

		//PS? ASSERT_EQUAL(str2.Capacity(), str3.Capacity());
		ASSERT_EQUAL(str2.Length(), str3.Length());
		ASSERT ( str2.Capacity() >= str2.Length() );
		ASSERT ( str3.Capacity() >= str3.Length() );
		ASSERT( memcmp( (const char *)str2, (const char *)str3, str2.Length() ) == 0 );

		//PS? ASSERT_EQUAL(str2.Capacity(), str0.Capacity());
		ASSERT_EQUAL(str2.Length(), str0.Length());
		ASSERT( memcmp( (const char *)str2, (const char *)str0, str2.Length() ) == 0 );

		// unerlaubte Eingabeparameter
		String str4;
		for ( i = 0; i < 256; i++ ) {
			for ( j = 0; j < 256; j++ ) {
				if ( ((char)i < '0') && ((char)i > '9') &&
					 ((char)i < 'a') && ((char)i > 'f') &&
					 ((char)i < 'A') && ((char)i > 'F') &&
					 ((char)j < '0') && ((char)j > '9') &&
					 ((char)j < 'a') && ((char)j > 'f') &&
					 ((char)j < 'A') && ((char)j > 'F')   ) {
					a[0] = (char)i;
					a[1] = (char)j;
					str4.AppendTwoHexAsChar( a );
					str4.Append(' ');
				}
			}
		}

		String str5;
		//PS? ASSERT_EQUAL(str4.Capacity(), str5.Capacity());
		ASSERT_EQUAL(str4.Length(), str5.Length());
		ASSERT ( str4.Capacity() >= str4.Length() );
		ASSERT ( str5.Capacity() >= str5.Length() );
		ASSERT( memcmp( (const char *)str4, (const char *)str5, str4.Length() ) == 0 );
	}

	// Test Methode AppendTwoHexAsChar(const char *, long, bool)
	{
		{
			String str0, str1, str2, str3;

			const char buf[7] = "005AFF";

			str0.AppendTwoHexAsChar(buf, 0);
			ASSERT ( str0 == String("") );
			ASSERT_EQUAL(str0.Length(), 0L);

			const char exp1[1] = { 0x00 };
			str1.AppendTwoHexAsChar(buf, 1);
			ASSERT ( memcmp(exp1, (const char *)str1, 1) == 0 );
			ASSERT_EQUAL(str1.Length(), 1L);

			const unsigned char exp2[3] = { 0x00, 0x5A, 0xff};//lint !e578
			str2.AppendTwoHexAsChar(buf, 3);
			ASSERT ( memcmp(exp2, (const char *)str2, 3) == 0 );
			ASSERT_EQUAL(str2.Length(), 3L);

			str3.AppendTwoHexAsChar(buf, -1);
			ASSERT ( str3 == String("") );
			ASSERT_EQUAL(str3.Length(), 0L);
		}
		{
			String str0, str1, str2, str3;

			const char buf[9] = "00 5A FF";

			str0.AppendTwoHexAsChar(buf, 0, true);
			ASSERT ( str0 == String("") );
			ASSERT_EQUAL(str0.Length(), 0L);

			const char exp1[1] = { 0x00 };
			str1.AppendTwoHexAsChar(buf, 1, true);
			ASSERT ( memcmp(exp1, (const char *)str1, 1) == 0 );
			ASSERT_EQUAL(str1.Length(), 1L);

			const unsigned char exp2[3] = { 0x00, 0x5A, 0xff};//lint !e578
			str2.AppendTwoHexAsChar(buf, 3, true);
			ASSERT ( memcmp(exp2, (const char *)str2, 3) == 0 );
			ASSERT_EQUAL(str2.Length(), 3L);

			str3.AppendTwoHexAsChar(buf, -1, true);
			ASSERT ( str3 == String("") );
			ASSERT_EQUAL(str3.Length(), 0L);
		}
	}
}

void StringTest::appendsFile()
{
	StartTrace(StringTest.appendsFile);
	// Init a string with a stream and a length
	String input = "01234";

	{
		IStringStream is(input);

		String str0;
		str0.Append( is, 5 );
		ASSERT (str0.Capacity() >= str0.Length() );
		ASSERT_EQUAL ( 5, str0.Length() );
		ASSERT_EQUAL ( 5, strlen( (const char *)str0 ) );
		ASSERT_EQUAL ( "01234", str0 );
	}
	{
		IStringStream is(input);

		String  str1;
		str1.Append( is, 10 );
		ASSERT (str1.Capacity() >= str1.Length()  );
		ASSERT_EQUAL ( 5, str1.Length() );
		ASSERT_EQUAL ( 5, strlen( (const char *)str1) );
		ASSERT_EQUAL ( "01234", str1 );
	}

	{
		IStringStream is(input);

		String  str2;
		str2.Append( is, -5 );
		ASSERT ( str2.Capacity() >= str2.Length() );
	}

	std::istream *is3 = system::OpenStream("len5", "tst");
	ASSERT( is3 != NULL );
	if ( is3 ) {
		String  str3;
		str3.Append( *is3, 0 );
		ASSERT_EQUAL("", str3);
		ASSERT_EQUAL(str3.Length(), 0L);
		//ASSERT_EQUAL(str3.Capacity(), 1L);
		ASSERT ( str3.Capacity() >= str3.Length() );
		delete is3;
	} else {
		ASSERT_EQUAL("'read file len5.tst'", "'could not read len5.tst'");
	}

	std::istream *is4 = system::OpenStream("len5", "tst");
	ASSERT( is4 != NULL );
	if (is4) {
		String  str4;
		str4.Append( *is4, 'a' );  // 'a' = 97
		ASSERT_EQUAL("01234", str4);
		ASSERT_EQUAL(str4.Length(), 5L);
		ASSERT (str4.Capacity() >= 'a' + 1);
		ASSERT ( str4.Capacity() >= str4.Length() );
		delete is4;
	} else {
		ASSERT_EQUAL("'read file len5.tst'", "'could not read len5.tst'");
	}
}

//==============================================================================================
//        M E T H O D E N
//==============================================================================================
void StringTest::methods()
{
	StartTrace(StringTest.methods);
	// IsEqual
	isEquals();

	//	Compare    STRCMP
	compares();

	// CompareN    STRNCMP
	compareNs();

	// At
	ats();

	// PutAt
	putAts();

	// ReplaceAt
	replaceAts();

	// StrChr
	strChrs();

	// StrRChr
	strRChrs ();

	// Contain
	contains();

	// SubString
	subStrings();

	// TrimFront
	trimFronts();

	// Trim
	trims();

	reserve();

	// ToLower
	toLowers();

	// ToUpper
	toUppers();

	// Dump
	dumps();

	// CaselessCompare
	caselessCompares();

	intPrintOns();
//	intReadFroms();

	asLong();

	asDouble();

	startsWith();

	asLongLong();
}

void StringTest::startsWith()
{
	StartTrace(StringTest.startsWith);
	ASSERT(String("").StartsWith(""));
	ASSERT(!String("").StartsWith(0));
	ASSERT(!String("").StartsWith("a"));

	ASSERT(String("a").StartsWith(""));
	ASSERT(String("a").StartsWith("a"));
	ASSERT(!String("a").StartsWith("ab"));
	ASSERT(!String("a").StartsWith("ba"));

	ASSERT(String("/foo/bar").StartsWith(""));
	ASSERT(String("/foo/bar").StartsWith("/"));
	ASSERT(String("/foo/bar").StartsWith("/foo"));
	ASSERT(String("/foo/bar").StartsWith("/foo/bar"));
	ASSERT(!String("/foo/bar").StartsWith("/foo/bar/"));
	ASSERT(!String("/foo/bar").StartsWith("/fox"));
}

void StringTest::asLong()
{
	String str;

	// Test a null string -> default value
	ASSERT_EQUAL(str.AsLong(-1), -1L);

	// Test a positive number
	str = "123456789";
	ASSERT_EQUAL(str.AsLong(-1), 123456789L);

	// Test a negative number
	str = "-123456789";
	ASSERT_EQUAL(str.AsLong(-1), -123456789L);

	// Test a positive number with ending text
	str = "123456789Hello";
	ASSERT_EQUAL( str.AsLong(1), 123456789L );

	// Test a negative number with ending text
	str = "-123456789Hello";
	ASSERT_EQUAL ( str.AsLong(-1), -123456789L );

	// Test a negative number with ending text
	str = "   -123456789";
	ASSERT_EQUAL ( str.AsLong(-1), -123456789L );

	// Test a negative number with ending text
	str = "  Hello-123456789";
	ASSERT_EQUAL ( str.AsLong(-1), -1L );

	// Test a positive number with leading text
	str = "Hello123456789";
	ASSERT_EQUAL ( str.AsLong(-1), -1L);

	// Test a negative number with leading text -> default value
	str = "Hello-123456789";
	ASSERT_EQUAL ( str.AsLong(-1), -1L );
}

void StringTest::asLongLong()
{
	String str;

	// Test a null string -> default value
	ASSERT_EQUAL(str.AsLongLong(INT64_LITERAL(-1)), INT64_LITERAL(-1));

	// Test a positive number smaller than 9223372036854775807 but higher than a long
	str = "12345678912345678";
	ASSERT_EQUAL(str.AsLongLong(INT64_LITERAL(-1)), INT64_LITERAL(12345678912345678));

	// Test a negative number higher than -9223372036854775808
	str = "-12345678912345678";
	ASSERT_EQUAL(str.AsLongLong(INT64_LITERAL(-1)), INT64_LITERAL(-12345678912345678));

	// Test a positive number with ending text
	str = "12345678912345678Hello";
	ASSERT_EQUAL ( str.AsLongLong(INT64_LITERAL(-1)), INT64_LITERAL(12345678912345678) );

	// Test a negative number with ending text
	str = "-12345678912345678Hello";
	ASSERT_EQUAL ( str.AsLongLong(INT64_LITERAL(1)), INT64_LITERAL(-12345678912345678) );

	// Test a negative number with ending text
	str = "    -12345678912345678";
	ASSERT_EQUAL ( str.AsLongLong(INT64_LITERAL(1)), INT64_LITERAL(-12345678912345678) );

	// Test a positive number with leading text
	str = "Hello12345678912345678";
	ASSERT_EQUAL(str.AsLongLong(INT64_LITERAL(-1)), INT64_LITERAL(-1));

	// Test a positive number with leading text
	str = "     Hello12345678912345678";
	ASSERT ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(-1) );

	// Test a negative number with leading text -> default value
	str = "Hello-12345678912345678";
	ASSERT_EQUAL(str.AsLongLong(INT64_LITERAL(-1)), INT64_LITERAL(-1));
}

void StringTest::asDouble()
{
	String str;

	// Test a null string -> default value
	ASSERT_EQUAL ( str.AsDouble(-1.5), -1.5 );

	// Test a positive number
	str = "123456789.123";
	ASSERT_EQUAL ( str.AsDouble(-1.5), 123456789.123 );

	// Test a negative number
	str = "-123456789.123";
	ASSERT_EQUAL ( str.AsDouble(-1.5), -123456789.123 );

	// Test a positive number with ending text
	str = "123456789.123Hello";
	ASSERT_EQUAL ( str.AsDouble(-1.5), 123456789.123 );

	// Test a negative number with ending text
	str = "-123456789.123Hello";
	ASSERT_EQUAL ( str.AsDouble(1.5), -123456789.123 );

	// Test a positive number with leading text
	str = "Hello123456789.123";
	ASSERT_EQUAL ( str.AsDouble(-1.5), -1.5 );

	// Test a negative number with leading text -> default value
	str = "Hello-123456789.123";
	ASSERT_EQUAL ( str.AsDouble(-1.5), -1.5 );
}

//==============================================================================================
//        I S E Q U A L             Beginn
//===========================================================================
void StringTest::isEqual0 ()
{
	String str0;
	String str1;
	str0.Append("abc");
	str0.Append('d');
	str0.Append(-123456789L);
	str0.Append(1.123456789E+23);
	str1.Append("abc");
	str1.Append('d');
	str1.Append(-123456789L);
	str1.Append(1.123456789E+23);
	ASSERT ( str0.IsEqual( (const char *)str1 ) == 1 );
	ASSERT ( str1.IsEqual( (const char *)str0 ) == 1 );
	//PS? ASSERT_EQUAL(str0.Capacity(), str1.Capacity());
	ASSERT_EQUAL(str0.Length(), str1.Length());
	ASSERT ( str0.Capacity() >= str0.Length() );
	ASSERT ( str1.Capacity() >= str1.Length() );
	ASSERT ( memcmp( (const char *)str0, (const char *)str1, str0.Length() ) == 0 );

	String str2 = str0;
	ASSERT ( str0.IsEqual( (const char *)str2 ) == 1 );
	ASSERT ( str2.IsEqual( (const char *)str0 ) == 1 );
	// ASSERT_EQUAL(str0.Capacity(), str2.Capacity());
	ASSERT_EQUAL(str0.Length(), str2.Length());
}

void StringTest::isEqual1()
{
	StartTrace(StringTest.isEqual1);
	char buf[5] = { 'a', 'b', (char)0, 'c', 'd' };
	String str;
	str.Append( (void *) buf, 5 );
	// void * cast because otherwise Append stops at 0
	ASSERT ( str.IsEqual( buf ) == 1 );
	ASSERT_EQUAL(str.Length(), 5L);
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( memcmp( (const char *)str, buf, 5 ) == 0 );
	// IsEqual geht bis zum \0 oder bis str.Length() ? Bis \0 (ist das gewuenscht ????)
	buf[3] = 'z';		// buffer changed, c is now z
	String str1((void *)buf, 5);
	// be careful with IsEqual
	// semantics as expected with String argument
	ASSERT_EQUAL ( 0, str.IsEqual( str1 ) );
	// char * semantics (0 terminated) with const char * argument!
	ASSERT_EQUAL ( 1, str.IsEqual( buf ) );
}

void StringTest::isEqual2()
{
	StartTrace(StringTest.isEqual2);
	char buf[5] = { 'a', 'b', (char)0, 'c', 'd' };
	String str0, str1 = "qqq";
	str0.Append( buf, 5 );

	ASSERT( str1.IsEqual( buf ) == 0 );
	ASSERT( str1.IsEqual( (const char *)str0  ) == 0);
	ASSERT( str0.IsEqual( (const char *)str1  ) == 0);
}

void StringTest::isEquals()
{
	StartTrace(StringTest.isEquals);
	isEqual0();
	isEqual1();
	isEqual2();
}
//==============================================================================================
//        I S E Q U A L             Ende
//==============================================================================================

//==============================================================================================
//        C O M P A R E             Beginn
//==============================================================================================
void StringTest::compare0 ()
{
	String str0, str1, str2;
	str0.Append("0123456789", 10);
	str1.Append("0123456789", 10);
	str2.Append("01234567", 8);
	ASSERT( str0.Compare( (const char *)str0 ) == 0 );
	ASSERT( str0.Compare( (const char *)str1 ) == 0 );
	ASSERT( str1.Compare( (const char *)str0 ) == 0 );
	ASSERT( str1.Compare( (const char *)str2 ) >  0 );
	ASSERT( str2.Compare( (const char *)str1 ) <  0 );
}

void StringTest::compareWithNullBytes ()
{
	String str0((void *)"0123\00056789", 10);
	String str1((void *)"0123\00045678", 10);
	String str2((void *)"0123\000", 5);
	ASSERT( str0.Compare( (const char *)str0 ) == 0 );
	ASSERT( str0.Compare( str0 ) == 0 );
	ASSERT( str0.Compare( (const char *)str1 ) ==  0 );
	// caution: comparison with a const char * stops at the first 0 character
	ASSERT( str0.Compare( str1 ) >  0 );
	// comparison with a real string is safer
	ASSERT( str1.Compare( (const char *)str0 ) == 0 );
	ASSERT( str1.Compare( str0 ) <  0 );
	ASSERT( str1.Compare( (const char *)str2 ) == 0 );
	ASSERT( str1.Compare( str2 ) >  0 );
	ASSERT( str2.Compare( (const char *)str1 ) == 0 );
	ASSERT( str2.Compare( str1 ) <  0 );
}

void StringTest::compares()
{
	StartTrace(StringTest.compares);
	compare0();
	compareWithNullBytes();
}
//==============================================================================================
//        C O M P A R E             Ende
//==============================================================================================

//==============================================================================================
//        C O M P A R E N            Beginn
//==============================================================================================
void StringTest::compareN0 ()
{
	String str0, str1, str2;
	str0.Append("0123456789", 10);
	str1.Append("0123456789", 10);
	str2.Append("01234567", 8);

	ASSERT( str0.CompareN( (const char *)str0, str0.Length(), 0 ) == 0 );
	ASSERT( str0.CompareN( (const char *)str1, str1.Length(), 0 ) == 0 );
	ASSERT( str1.CompareN( (const char *)str0, str1.Length(), 0 ) == 0 );
	ASSERT( str1.CompareN( (const char *)str2, str1.Length(), 0 ) >  0 );
	ASSERT( str2.CompareN( (const char *)str1, str1.Length(), 0 ) <  0 );
	ASSERT( str2.CompareN( (const char *)str1, str2.Length(), 0 ) ==  0 );
	ASSERT( str1.CompareN( (const char *)str2, str2.Length(), 0 ) ==  0 );
}

void StringTest::compareN1()
{
	StartTrace(StringTest.compareN1);
	// Der Test geht bis zum \0
	String	str0, str1, str2, str3 = "XXXX456789", str4 = "456789";
	long	length;

	str0.Append("0123456789", 10);
	str1.Append("0123456789", 10);
	str2.Append("01234567", 8);

	for ( length = 1; length < 50; length++ ) {
		// strncmp( &str0[4], &str1[4], ...)
		ASSERT( str0.CompareN( (const char *)str1 + 4, length, 4 ) == 0 );
		// strncmp( &str1[4], &str0[4], ...)
		ASSERT( str1.CompareN( (const char *)str0 + 4, length, 4 ) == 0 );
		if ( (length + 4) <= str2.Length() ) {
			//  Da der Test innerhalb str1 und str2 ist, ist strncmp == 0
			// strncmp( &str2[4], &str1[4], ...)
			ASSERT( str2.CompareN( (const char *)str1 + 4, length, 4 ) ==  0 );
			// strncmp( &str1[4], &str2[4], ...)
			ASSERT( str1.CompareN( (const char *)str2 + 4, length, 4 ) ==  0 );
		} else {
			//  Da der Test auch ausserhalb str1 und str2 ist, ist strncmp != 0 (str1 ist laenger als str2)
			// strncmp( &str2[4], &str1[4], ...)
			ASSERT( str2.CompareN( (const char *)str1 + 4, length, 4 ) <  0 );
			// strncmp( &str1[4], &str2[4], ...)
			ASSERT( str1.CompareN( (const char *)str2 + 4, length, 4 ) >  0 );
		}

		// strncmp( &str1[4], &str3[4], ...)
		ASSERT( str1.CompareN( (const char *)str3 + 4, length, 4 ) == 0 );
		// strncmp( &str3[4], &str1[4], ...)
		ASSERT( str3.CompareN( (const char *)str1 + 4, length, 4 ) == 0 );

		// strncmp( &str1[4], &str4[0], ...)
		ASSERT ( str1.CompareN( (const char *)str4, length, 4 ) == 0 );
		// strncmp( &str4[0], &str1[4], ... );
		ASSERT ( str4.CompareN( (const char *)str1 + 4, length, 0 ) == 0 );
	}

	// strncmp( &str1[0], &str1[1], 2 );
	ASSERT ( str1.CompareN( (const char *)str1 + 1, 2, 0 ) < 0 );

	// strncmp( &str1[0], &str1[1], 0 );
	ASSERT ( str1.CompareN( (const char *)str1 + 1, 0, 0 ) == 0 );
}

void StringTest::compareNs()
{
	StartTrace(StringTest.compareNs);
	compareN0();
	compareN1();
}
//==============================================================================================
//        C O M P A R E N            Ende
//==============================================================================================


//==============================================================================================
//         T O L O W E R           Beginn
//==============================================================================================
void StringTest::toLower0()
{
	String	str, strHlp;
	long	i;

	for ( i = 1; i < 256; i++ ) {
		str.Append( (char)i );

		if ( ( i >= (long)'A' ) && ( i <= (long) 'Z' )  ) {
			strHlp.Append( (char)(i + (long)'a' - (long)'A' ) );
		} else {
			strHlp.Append( (char)i );
		}
	}

	str.ToLower();
	ASSERT ( str == strHlp );
	ASSERT_EQUAL(str.Length(), strHlp.Length());
	ASSERT_EQUAL(str.Capacity(), strHlp.Capacity());
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( strHlp.Capacity() >= strHlp.Length() );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );

	str.ToLower();
	ASSERT ( str == strHlp );
	ASSERT_EQUAL(str.Length(), strHlp.Length());
	ASSERT_EQUAL(str.Capacity(), strHlp.Capacity());
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( strHlp.Capacity() >= strHlp.Length() );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );

}

void StringTest::toLowers()
{
	StartTrace(StringTest.toLowers);
	toLower0();
}
//==============================================================================================
//         T O L O W E R          Ende
//==============================================================================================

//==============================================================================================
//         T O U P P E R          Beginn
//==============================================================================================
void StringTest::toUpper0()
{
	String	str, strHlp;
	long	i;

	for ( i = 1; i < 256; i++ ) {
		str.Append( (char)i );

		if ( ( i >= (long)'a' ) && ( i <= (long) 'z' )  ) {
			strHlp.Append( (char)(i - (long)'a' + (long)'A' ) );
		} else {
			strHlp.Append( (char)i );
		}
	}

	str.ToUpper();
	ASSERT ( str == strHlp );
	ASSERT_EQUAL(str.Length(), strHlp.Length());
	ASSERT_EQUAL(str.Capacity(), strHlp.Capacity());
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( strHlp.Capacity() >= strHlp.Length() );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );

	str.ToUpper();
	ASSERT ( str == strHlp );
	ASSERT_EQUAL(str.Length(), strHlp.Length());
	ASSERT_EQUAL(str.Capacity(), strHlp.Capacity());
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( strHlp.Capacity() >= strHlp.Length() );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::toUppers()
{
	StartTrace(StringTest.toUppers);
	toUpper0();
}
//==============================================================================================
//         T O P P E R         Ende
//==============================================================================================

//==============================================================================================
//         P R E P E N D W I T H           Beginn
//==============================================================================================
void StringTest::prependWith()
{
	// String is smaller then newLen
	for ( int i = 0; i < 256; i++ ) {
		String str, result;
		for ( int ii = 256; ii > i; ii-- ) {
			str.Append("z");
		}
		long oldLen = str.Length();
		ASSERT(str.PrependWith(str.Length() + i, 'x') == 1);
		ASSERT_EQUAL((oldLen + i), str.Length());
		for ( int ii = 0; ii < i; ii++ ) {
			result.Append("x");
		}
		for ( int ii = 0; ii < (256 - i); ii++ ) {
			result.Append("z");
		}
		ASSERT_EQUAL(result, str);
	}
	// String is equal/smaller then newLen, a no-op
	for ( int i = 0; i < 256; i++ ) {
		String str, result;
		for ( int ii = 0; ii < i; ii++ ) {
			str.Append("z");
		}
		result = str;
		long oldLen = str.Length();
		for ( int ii = 0; ii < i; ii++ ) {
			ASSERT(str.PrependWith(str.Length() - i, 'x') == 0);
			ASSERT_EQUAL((oldLen), str.Length());
			ASSERT_EQUAL(result, str);
		}
	}

	// Zero length string test; string must be expanded
	String str;
	str.PrependWith(2, 'x');
	ASSERT_EQUAL("xx", str);
	ASSERT_EQUAL(2L, str.Length());

	// memcopy would overlap
	String str1 = "123";
	str1.PrependWith(4, '0');
	ASSERT_EQUAL("0123", str1);
	String str2 = "123";
	str2.PrependWith(5, '0');
	ASSERT_EQUAL("00123", str2);

	// memcpy is ok
	String str3 = "123";
	str3.PrependWith(6, '0');
	ASSERT_EQUAL("000123", str3);

}
//==============================================================================================
//         P R E P E N D W I T H           Ende
//==============================================================================================

//==============================================================================================
//         D U M P            Beginn
//==============================================================================================
void StringTest::dump0()
{
	String str = "Test for methos DUMP ...";
	str.Dump();
}

void StringTest::dumps()
{
	StartTrace(StringTest.dumps);
	dump0();
}

void StringTest::IntReplaceTest(String orig, const String &pattern, const String &nStr, bool exp, const String expStr)
{//lint !e578
	StartTrace(StringTest.IntReplaceTest);
	ASSERT(exp == orig.Replace(pattern, nStr));
	ASSERT_EQUAL(expStr, orig);
}

void StringTest::ReplaceTest()
{
	StartTrace(StringTest.ReplaceTest);
	IntReplaceTest("/a/b/c", "a", "d", true, "/d/b/c");
	IntReplaceTest("/a/b/c", "/a/b/c", "d", true, "d");
	IntReplaceTest("/a/b/c", "/a/b/d", "d", false, "/a/b/c");
	IntReplaceTest("/a/b/c", "", "d", true, "d/a/b/c");
	IntReplaceTest("", "", "d", false, "");
	IntReplaceTest("", "b", "d", false, "");
}

void StringTest::DumpAsHexTest()
{
	String teststring("0123456789ABCDEF");
	{
		String expected("30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46  0123456789ABCDEF"), answer;
		answer = teststring.DumpAsHex();
		ASSERT_EQUAL(expected.Length(), answer.Length());
		ASSERT_EQUAL(expected, answer);
	}
	{
		String expected("30 31 32 33 34 35 36 37  01234567\n38 39 41 42 43 44 45 46  89ABCDEF"), answer;
		answer = teststring.DumpAsHex(8L);
		ASSERT_EQUAL(expected.Length(), answer.Length());
		ASSERT_EQUAL(expected, answer);
	}
	{
		String expected("30 31 32 33 34 35 36 37  01234567\r\n38 39 41 42 43 44 45 46  89ABCDEF"), answer;
		answer = teststring.DumpAsHex(8L, "\r\n");
		ASSERT_EQUAL(expected.Length(), answer.Length());
		ASSERT_EQUAL(expected, answer);
	}
	{
		String expected("30 31 32 33  0123\n34 35 36 37  4567\n38 39 41 42  89AB\n43 44 45 46  CDEF"), answer;
		answer = teststring.DumpAsHex(4L);
		ASSERT_EQUAL(expected.Length(), answer.Length());
		ASSERT_EQUAL(expected, answer);
	}
	{
		String expected("30 31  01\n32 33  23\n34 35  45\n36 37  67\n38 39  89\n41 42  AB\n43 44  CD\n45 46  EF"), answer;
		answer = teststring.DumpAsHex(2L);
		ASSERT_EQUAL(expected.Length(), answer.Length());
		ASSERT_EQUAL(expected, answer);
	}
	{
		String expected("30  0\n31  1\n32  2\n33  3\n34  4\n35  5\n36  6\n37  7\n38  8\n39  9\n41  A\n42  B\n43  C\n44  D\n45  E\n46  F"), answer;
		answer = teststring.DumpAsHex(1L);
		ASSERT_EQUAL(expected.Length(), answer.Length());
		ASSERT_EQUAL(expected, answer);
	}
	{
		teststring = "";
		for (long l = 0; l < 16; l++) {
			teststring.PutAt(l, char(l));
		}
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ................" << std::endl << std::flush;
		teststring.DumpAsHex(os);
		ASSERT_EQUAL(expected, answer);
	}
	{
		teststring = "ABCDEFGHIJKLMNOPQRSTUVQWXYZ";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50  ABCDEFGHIJKLMNOP" << std::endl;
		os2 << "51 52 53 54 55 56 51 57 58 59 5A                 QRSTUVQWXYZ     " << std::endl << std::flush;
		teststring.DumpAsHex(os);
		ASSERT_EQUAL(expected, answer);
	}
	{
		teststring = "";
		for (long l = 0; l < 256; l++) {
			teststring.PutAt(l, char(l));
		}
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ................" << std::endl;
		os2 << "10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F  ................" << std::endl;
		os2 << "20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F   !" << '"' << "#$%&'()*+,-./" << std::endl;
		os2 << "30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F  0123456789:;<=>?" << std::endl;
		os2 << "40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F  @ABCDEFGHIJKLMNO" << std::endl;
		os2 << "50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F  PQRSTUVWXYZ[\\]^_" << std::endl;
		os2 << "60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F  `abcdefghijklmno" << std::endl;
		os2 << "70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F  pqrstuvwxyz{|}~." << std::endl;
		os2 << "80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F  ................" << std::endl;
		os2 << "90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F  ................" << std::endl;
		os2 << "A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF  ................" << std::endl;
		os2 << "B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF  ................" << std::endl;
		os2 << "C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF  ................" << std::endl;
		os2 << "D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF  ................" << std::endl;
		os2 << "E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF  ................" << std::endl;
		os2 << "F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF  ................" << std::endl << std::flush;
		teststring.DumpAsHex(os);
		ASSERT_EQUAL(expected, answer);
	}
}

//==============================================================================================
//         D U M P           Ende
//==============================================================================================

//==============================================================================================
//         C A S E L E S S C O M P A R E          Beginn
//==============================================================================================
void StringTest::caselessCompare0()
{
	StartTrace(StringTest.caselessCompare0);
	// Fall OHNE case
	String str = "012345";
	ASSERT( str.CaselessCompare( "012345", "012345") == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "012345") == 0 );
	ASSERT( str.CaselessCompare( "012345", (const char *)str ) == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, (const char *)str ) == 0 );

	ASSERT( str.CaselessCompare( "012345", "01234567") < 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "01234567") < 0 );
	ASSERT( str.CaselessCompare( "01234567", (const char *)str ) > 0 );

	ASSERT( str.CaselessCompare( "012345", "0123") > 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "0123") > 0 );
	ASSERT( str.CaselessCompare( "0123", (const char *)str ) < 0 );
}

void StringTest::caselessCompare1()
{
	StartTrace(StringTest.caselessCompare1);
	// Fall OHNE case
	String str = "abcdef";
	ASSERT( str.CaselessCompare( "abcdef", "abcdef") == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "abcdef") == 0 );
	ASSERT( str.CaselessCompare( "abcdef", (const char *)str ) == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, (const char *)str ) == 0 );

	ASSERT( str.CaselessCompare( "abcdef", "abcdefgh") < 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "abcdefgh") < 0 );
	ASSERT( str.CaselessCompare( "abcdefgh", (const char *)str ) > 0 );

	ASSERT( str.CaselessCompare( "abcdef", "abcd") > 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "abcd") > 0 );
	ASSERT( str.CaselessCompare( "abcd", (const char *)str ) < 0 );
}

void StringTest::caselessCompare2()
{
	StartTrace(StringTest.caselessCompare2);
	// Fall MIT case
	String str = "aBcdEf", strReverse = "AbCDeF";
	ASSERT( str.CaselessCompare( "AbcdeF", "abCDef") == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "ABCdef") == 0 );
	ASSERT( str.CaselessCompare( "abcDEf", (const char *)str ) == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, (const char *)strReverse ) == 0 );
	ASSERT( str.CaselessCompare( (const char *)str, (const char *)str ) == 0 );

	ASSERT( str.CaselessCompare( "abcdef", "ABDDEFGH") < 0 );
	ASSERT( str.CaselessCompare( "ABCDEF", "abcdefgh") < 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "abcdEfgh") < 0 );
	ASSERT( str.CaselessCompare( "abcdefgH", (const char *)str ) > 0 );

	ASSERT( str.CaselessCompare( "abcdef", "abcd") > 0 );
	ASSERT( str.CaselessCompare( "ABCDEF", "abcd") > 0 );
	ASSERT( str.CaselessCompare( "abcdef", "ABCD") > 0 );
	ASSERT( str.CaselessCompare( "ABCDEF", "ABCD") > 0 );
	ASSERT( str.CaselessCompare( (const char *)str, "abcA") > 0 );
	ASSERT( str.CaselessCompare( "aBCd", (const char *)str ) < 0 );
}

void StringTest::caselessCompare3()
{
	StartTrace(StringTest.caselessCompare3);
	// Fall MIT case
	String	str, strHlp, str1;
	long	i;

	for ( i = 0; i < 256; i++ ) {
		str.Append( (char)i );

		if ( ( i >= (long)'a' ) && ( i <= (long) 'z' )  ) {
			strHlp.Append( (char)(i - (long)'a' + (long)'A' ) );
		} else {
			strHlp.Append( (char)i );
		}
	}

	ASSERT( str.CaselessCompare( (const char *)str, (const char *)strHlp ) == 0 );

	ASSERT( strHlp.CaselessCompare( (const char *)str, (const char *)strHlp ) == 0 );

	ASSERT( str1.CaselessCompare( (const char *)str, (const char *)strHlp ) == 0 );
}

void StringTest::caselessCompareWithNullPtr()
{
	String test("test");

	ASSERT(String::CaselessCompare(test, "Test") == 0);
	ASSERT(String::CaselessCompare(test, 0) == 1);
}

void StringTest::caselessCompares()
{
	StartTrace(StringTest.caselessCompares);
	caselessCompare0();
	caselessCompare1();
	caselessCompare2();
	caselessCompare3();
	caselessCompareWithNullPtr();
}
//==============================================================================================
//         C A S E L E S S C O M P A R E          Ende
//==============================================================================================

//==============================================================================================
//         I N T P R I N T O N               Beginn
//==============================================================================================

void StringTest::intPrintOn0()
{
	StartTrace(StringTest.intPrintOn0);
	int		i;
	String	str, strHlp;

	for ( i = 0; i <= 256; i++ ) {
		if ( isprint( (unsigned char) i ) ) {
			str.Append( (char)i );
		}
	}

	// With proper Read/Write-Methods
	std::ostream *os = system::OpenOStream("tmp/fileprnt1", "tst");
	if ( os ) {
		str.IntPrintOn( *os, '\"');
		delete os;
		std::istream *is = system::OpenIStream("tmp/fileprnt1", "tst");
		if ( is ) {
			String	str1;
			str1.IntReadFrom( *is, '\"');
			ASSERT_EQUAL(str.Length(), str1.Length());
			ASSERT( memcmp( (const char *)str, (const char *)str1, str.Length() ) == 0 );
			delete is;

			// With normal Read/Write-Methods
			std::ostream *os1 = system::OpenOStream("tmp/fileprnt", "tst");
			if ( os1 ) {
				*os1 << str;
				delete os1;
				std::istream *is1 = system::OpenIStream("tmp/fileprnt", "tst");
				if ( is1 ) {
					*is1 >> str1;
					delete is1;
					ASSERT_EQUAL(str.Length(), str1.Length() + 1L);
					// the leading space is skiped
					ASSERT( memcmp( (const char *)str, (const char *)str1, str.Length() ) < 0 );  // str1 does not have the leading SPACE ????
					// << and >> are not symmetric if spaces are involved!!
				} else {
					ASSERT_EQUAL("'read file tmp/fileprnt.tst'", "'could not read tmp/fileprnt.tst'");
				}
			} else {
				ASSERT_EQUAL("'write to file tmp/fileprnt.tst'", "'could not write tmp/fileprnt.tst'");
			}
		} else {
			ASSERT_EQUAL("'read file tmp/fileprnt1.tst'", "'could not read tmp/fileprnt1.tst'");
		}
	} else {
		ASSERT_EQUAL("'write to file tmp/fileprnt1.tst'", "'could not write tmp/fileprnt1.tst'");
	}
	// read from a stream with spaces
	String text = " This test ";
	String forStream;
	{
		OStringStream os2(&forStream);
		os2 << text;
	}

	IStringStream is2(forStream);

	is2 >> str;
	ASSERT_EQUAL("This", str);
	ASSERT(is2.good() != 0);
	is2 >> str;
	ASSERT_EQUAL("test", str);
	ASSERT(is2.good() != 0);
	ASSERT(!is2.eof());		// space at the end is not eof!
	is2 >> str;
	ASSERT_EQUAL("", str);
	ASSERT(is2.good() == 0);
	ASSERT(is2.eof() != 0);

}

void StringTest::intPrintOn1()
{
	StartTrace(StringTest.intPrintOn1);
	int		i;
	String	str, strHlp;

	for ( i = 0; i <= 256; i++ ) {
		if ( !isprint( (unsigned char) i ) ) {
			str.Append( (char)i );
		}
	}

	std::ostream *os = system::OpenOStream("tmp/filenoprnt", "tst");
	if ( os ) {
		str.IntPrintOn( *os, '\"');
		delete os;
		std::istream *is = system::OpenIStream("tmp/filenoprnt", "tst");
		if ( is ) {
			String str1;
			str1.IntReadFrom( *is, '\"');
			delete is;
			ASSERT_EQUAL(str.Length(), str1.Length());
			ASSERT( memcmp( (const char *)str, (const char *)str1, str.Length() ) == 0 );
		} else {
			ASSERT_EQUAL("'read file tmp/filenoprnt.tst'", "'could not read tmp/filenoprnt.tst'");
		}
	} else {
		ASSERT_EQUAL("'write to file tmp/filenoprnt.tst'", "'could not write tmp/filenoprnt.tst'");
	}
}

void StringTest::intPrintOns()
{
	StartTrace(StringTest.intPrintOns);
	intPrintOn0();
	intPrintOn1();
}
//==============================================================================================
//         I N T P R I N T O N               Ende
//==============================================================================================

//==============================================================================================
//         A T              Beginn
//==============================================================================================
void StringTest::at0 ()
{
	String		str;
	long		i;

	for ( i = 0; i < 256; i++ ) {
		str.Append( (char)i );
	}
	for ( i = 0; i < 256; i++ ) {
		ASSERT( str.At(i) == (char)i );
	}

	// At out of boundaries:  result is 0  (OK!)
	ASSERT_EQUAL(str.At(-2), '\0');
	ASSERT_EQUAL(str.At(300), '\0');
	ASSERT( str.At( str.Length() ) == (char)str.Length() );
	ASSERT( str.At( str.Capacity() ) == 0 );
	ASSERT( str.At( str.Capacity() + 1 ) == 0 );
}

void StringTest::ats()
{
	StartTrace(StringTest.ats);
	at0 ();
}
//==============================================================================================
//         A T              Ende
//==============================================================================================

//==============================================================================================
//        P U T A T         Beginn
//==============================================================================================
void StringTest::putAt0 ()
{
	String		str;
	long 		i;

	for ( i = 0; i < 256; i++ ) {
		str.Append( ' ' );
	}

	for ( i = 0; i < 256; i++ ) {
		ASSERT( str.At(i) == ' ' );
	}

	for ( i = 0; i < 256; i++ ) {
		ASSERT_EQUAL(str.Length(), 256L);

		str.PutAt(i, (char)i);
	}
}

void StringTest::putAts()
{
	StartTrace(StringTest.putAts);
	putAt0();
}

//==============================================================================================
//        P U T A T                  Ende
//==============================================================================================

//==============================================================================================
//        R E P L A C E A T          Beginn
//==============================================================================================
void StringTest::replaceAt0 ()
{
	String	str, strHlp;
	long	i;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos 0
	long strCapTmp, strLenTmp;
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( 0, buf, i );
		ASSERT_EQUAL(strCapTmp, str.Capacity());
		ASSERT_EQUAL(strLenTmp, str.Length());
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str before replacing
		memcpy( &bufHlp[0], buf, i );												// replacing
		ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt1 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos > 0
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( 10, buf, i );
		ASSERT_EQUAL(strCapTmp, str.Capacity());
		ASSERT_EQUAL(strLenTmp, str.Length());
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 before replacing
		memcpy( &bufHlp[10], buf, i );												// replacing
		ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt2 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos < 0 : no replacement.
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( -5, buf, i );
		ASSERT_EQUAL(strCapTmp, str.Capacity());
		ASSERT_EQUAL(strLenTmp, str.Length());
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
		ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAtBeyondLength ()
{
	String	str("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }, bufHlp[150];
	memset(bufHlp, '\0', 150);

	long length = str.Length();
	memcpy( bufHlp, (const char *)str, length );
	memcpy( bufHlp + length + 1, buf, 50);			// concat

	// Replace at pos > length :  Append
	// Replace at Length()+1
	str.ReplaceAt( str.Length() + 1, buf, 50 );
	// the length of the char* is not checked, no purify error here!

	ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	ASSERT_EQUAL(bufHlp, (const char *) str);
}

void StringTest::replaceAt5 ()
{
	String	str, strHlp;
	long	i;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos = length-5 ---> Overflow:  Content, Length
	//	and Capacity grow and there is no problem (OK!)
	// Do not let the string grow
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		str.ReplaceAt( str.Length() - 5, buf, i );
	}

	// let the string grow
	str = strHlp;
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str.ReplaceAt( str.Length() - 5, buf, i );
	}
}

void StringTest::replaceAt6 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace 0 char:  no replacement
	for ( i = 0; i < str.Length() ; i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( i, buf, 0 );
		ASSERT_EQUAL(strCapTmp, str.Capacity());
		ASSERT_EQUAL(strLenTmp, str.Length());
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
		ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt7 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace -3 char: no replacement
	for ( i = 0; i < str.Length() ; i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( i, buf, -3 );
		ASSERT_EQUAL(strCapTmp, str.Capacity());
		ASSERT_EQUAL(strLenTmp, str.Length());
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
		ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt8()
{
	StartTrace(StringTest.replaceAt8);
	// code eliminated since it read past the valid contents of a const char *
	// and did not really assert the actual result
	// (which is basically impossible anyway)
}

void StringTest::replaceAt9 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// ReplaceAt on itsself
	str = strHlp;
	strCapTmp = str.Capacity();
	strLenTmp = str.Length();
	str.ReplaceAt( 0, (const char *)str, str.Length() );
	ASSERT_EQUAL(strCapTmp, str.Capacity());
	ASSERT_EQUAL(strLenTmp, str.Length());
	memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
	ASSERT ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison

	// ReplaceAt itsself does not work ?????
	str = strHlp;
	strCapTmp = str.Capacity();
	strLenTmp = str.Length();
	str.ReplaceAt( 1, (const char *)str, str.Length() );
	ASSERT ( strCapTmp <= str.Capacity() );
	ASSERT ( strLenTmp <  str.Length() );
	memcpy( &bufHlp[1], (const char *)strHlp, strHlp.Length() );					// bufHlp = the expected result
}

void StringTest::replaceAts()
{
	StartTrace(StringTest.replaceAts);
	replaceAt0 ();
	replaceAt1 ();
	replaceAt2 ();
	replaceAtBeyondLength ();
	replaceAt5 ();
	replaceAt6 ();
	replaceAt7 ();
	replaceAt8 ();
	replaceAt9 ();
}
//==============================================================================================
//         R E P L A C E A T         Ende
//==============================================================================================

//==============================================================================================
//         S T R C H R          Beginn
//==============================================================================================
void StringTest::strChr0 ()
{
	String	str = "0123456789";
	long	i, j;

	for ( i = 0 ; i < 10 ; i++ ) {
		for ( j = 0; j < 10; j++ ) {
			if ( i >= j ) { // (char)(i+'0') must be found at index i
				ASSERT( str.StrChr( (char)(i + '0'), j ) == i );
			} else { // (char)i cannot be found
				ASSERT( str.StrChr( (char)(i + '0'), j ) == -1 );
			}
		}
	}

	for ( i = 0 ; i < 256 ; i++ ) {
		if ( i == 0 ) {
			ASSERT( str.StrChr( (char)i, 0 ) == -1 );
			// PT: this seems to have changed ?! Should be correct now
		} else if ( ((char)i >= '0') && ((char)i <= '9') ) {
			ASSERT( str.StrChr( (char)i, 0 ) == (long)(i - '0') );
		} else {
			ASSERT( str.StrChr( (char)i, 0 ) == -1 );
		}
	}
}

void StringTest::strChr1 ()
{
	String	str = "0123456789";
	long	i;

	// Exceptions
	for ( i = 0; i < 10 ; i++ ) {
		ASSERT( str.StrChr( (char)(i + '0'), -5 ) == i );							// FOUND !!!
		ASSERT( str.StrChr( (char)(i + '0'), str.Length() + 1 ) == -1 );
		ASSERT( str.StrChr( (char)(i + '0'), str.Length() + 2 ) == -1 );
		ASSERT( str.StrChr( (char)(i + '0'), str.Capacity() ) == -1 );
		ASSERT( str.StrChr( (char)(i + '0'), str.Capacity() + 1 ) == -1 );
		ASSERT( str.StrChr( (char)(i + '0'), str.Capacity() + 2 ) == -1 );
	}
}

void StringTest::strChrs()
{
	StartTrace(StringTest.strChrs);
	strChr0();
	strChr1();
}
//==============================================================================================
//         S T R R C H R          Beginn
//==============================================================================================

//==============================================================================================
//         S T R R C H R          Beginn
//==============================================================================================
void StringTest::strRChr0 ()
{
	String	str = "0123456789";
	long	i;

	for ( i = 0 ; i < 10 ; i++ ) {
		ASSERT( str.StrRChr( (char)(i + '0') ) == i );
	}

	for ( i = 0 ; i < 256 ; i++ ) {
		if ( ((char)i >= '0') && ((char)i <= '9') ) {
			ASSERT_EQUAL( (long)(i - '0'), str.StrRChr( (char)i ) );
		} else {
			ASSERT_EQUAL( -1, str.StrRChr( (char)i ) );
		}
	}
}

void StringTest::strRChrs()
{
	StartTrace(StringTest.strRChrs);
	strRChr0();
}
//==============================================================================================
//         S T R R C H R          Beginn
//==============================================================================================

//==============================================================================================
//         C O N T A I N           Beginn
//==============================================================================================
void StringTest::contain0()
{
	StartTrace(StringTest.contain0);
	//	bool   Contains(const char *pattern) const;
	//----------------------------------------------
	String	str = "qwertzuiopasdfghjklyxcvbnm";
	char	buf[50];
	long	i, j, length = str.Length();

	for ( i = 0; i < length; i++ ) {
		for ( j = 1; j <= length - i; j++ ) {
			memcpy( buf, (const char *)str + i, j );
			buf[j] = 0;
			ASSERT( str.Contains( buf ) > -1 );
		}
	}

	ASSERT( str.Contains( "" ) > -1 );
	ASSERT( str.Contains( "0" ) == -1 );
	ASSERT( str.Contains( "01" ) == -1 );
	ASSERT( str.Contains( "012" ) == -1 );
	ASSERT( str.Contains( "0123" ) == -1 );
	ASSERT( str.Contains( "01234" ) == -1 );
	ASSERT( str.Contains( "012345" ) == -1 );
	ASSERT( str.Contains( "0123456" ) == -1 );
	ASSERT( str.Contains( "01234567" ) == -1 );
}

void StringTest::contains()
{
	StartTrace(StringTest.contains);
	contain0 ();
}
//==============================================================================================
//         C O N T A I N           Ende
//==============================================================================================

//==============================================================================================
//         S U B S T R I N G           Beginn
//==============================================================================================

//	String SubString(const char *pattern) const;	finds first occurence of pattern in String
//----------------------------------------------------------------------------------------------
void StringTest::subString0 ()
{
	String	str = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm", strHlp;
	char	buf[50];
	long	i, j, length = str.Length() / 2;

	// SubString is done:  is the result the expected one?  Yes!!!
	for ( i = 0; i < length; i++ ) {
		for ( j = 1; j <= length - i; j++ ) {
			memcpy( buf, (const char *)str + i, j );
			buf[j] = 0;
			ASSERT( memcmp( (const char *)str.SubString( buf ),
							  (const char *)str + i,							  // the pattern starts at pos 'i'
							  str.Length() - i
							) == 0
					);

			strHlp = str.SubString( buf );
			ASSERT( memcmp( (const char *)strHlp, (const char *)str + i, str.Length() - i) == 0 );
			ASSERT_LESS_EQUAL( strHlp.Length(), strHlp.Capacity() );
			ASSERT_EQUAL(strHlp.Length(), str.Length() - i );
		}
	}

	// After many str.SubString() is str unchanged? Yes!!!
	String	strBak = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm";
	ASSERT_EQUAL(str.Length(), strBak.Length());
	ASSERT_EQUAL(str.Capacity(), strBak.Capacity());
	ASSERT( memcmp( (const char *)str, (const char *)strBak, str.Length() ) == 0 );
}

void StringTest::subString1 ()
{
	String	str = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm";

	// SubString cannot be done: result is an empty string? Yes!!! Aber nicht 100%
	// PT: Something must have changed here ...
	ASSERT ( str.SubString( "" ).Length() == str.Length() );  // whole string
	ASSERT ( str.SubString( "" ).Capacity() >= str.SubString( "" ).Length() );
	ASSERT ( str.SubString( "" ) == str );
	ASSERT ( str.SubString( "" ).Length() <= str.SubString( "" ).Capacity() );

	ASSERT ( str.SubString( "0" ).Length() == 0 );
	ASSERT ( str.SubString( "0" ).Capacity() >= 0 );
	ASSERT ( str.SubString( "0" ).Length() <= str.SubString( "0" ).Capacity() );
}

void StringTest::subString2 ()
{
	String	str = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm";
	// SubString with itsself
	str = str.SubString( "rtzu" );
	ASSERT( memcmp( (const char *)str, "rtzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm", str.Length() ) == 0 );
	ASSERT_LESS_EQUAL( str.Length(), str.Capacity() );
	ASSERT_EQUAL(str.Length(), (long)strlen("rtzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm"));
}

void StringTest::subString3()
{
	StartTrace(StringTest.subString3);
	//	String SubString(long start, long length= -1) const;
	//----------------------------------------------------------
	String	str = "01234567890123456789", strHlp;
	long	i, j, length = str.Length();

	for ( i = 0; i < length; i++ ) {		// i=start
		for ( j = 0; j < length - i; j++ ) {	// j=nr of char to copy
			ASSERT( memcmp( (const char *)str.SubString( i, j ), (const char *)str + i, j ) == 0 );
			ASSERT( str.SubString( i, j ).Length() <= str.SubString( i, j ).Capacity() );
			ASSERT( str.SubString( i, j ).Length() == j);

			strHlp = str.SubString( i, j );
			ASSERT( memcmp( (const char *)strHlp, (const char *)str + i, j ) == 0 );
			ASSERT_LESS_EQUAL( strHlp.Length(), strHlp.Capacity() );
			ASSERT_EQUAL(strHlp.Length(), j);
		}
	}
}

void StringTest::subString4()
{
	StartTrace(StringTest.subString4);
	//	String SubString(long start, long length= -1) const;
	//----------------------------------------------------------
	String str = "01234567890123456789";
	// SubString starts at negative index: adjust start to 0 and return length chars
	ASSERT ( str.SubString( -5, 3 ).Length() == 3 );
	ASSERT ( str.SubString( -5, 3 ).Capacity() >= str.SubString( -5, 3 ).Length() );
	ASSERT ( str.SubString( -5, 3 ) == "012" );

	// Negative Länge:  bis zum Ende
	// Es ist OK (KEIN FEHLER)
	String subStr1 = str.SubString( 5, -3 );
	// von Stelle Nummer 5 bis zum Ende (d.h. ab der 6. Position!)
	ASSERT_EQUAL(subStr1.Length(), (long)strlen("567890123456789"));
	ASSERT ( memcmp( (const char *)subStr1, "567890123456789", subStr1.Length()) == 0 );
	ASSERT ( subStr1.Capacity() >= subStr1.Length() );

	// SubString out of boundary
	// PT: must stop at the end.
	ASSERT ( str.SubString( 5, 100 ).Length() == str.Length() - 5 );
	ASSERT ( str.SubString( 5, 100 ).Capacity() >= str.SubString( 5, 100 ).Length() );
	ASSERT ( str.SubString( 5, 100 ) == "567890123456789" );

	// After many str.SubString() is str unchanged?
	String	strHlp = "01234567890123456789";
	ASSERT_EQUAL(str.Length(), strHlp.Length());
	ASSERT_EQUAL(str.Capacity(), strHlp.Capacity());
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::subString5 ()
{
	String str = "01234567890123456789";
	// SubString with itsself
	str = str.SubString( 2, 5 );
	ASSERT( memcmp( (const char *)str, "23456", str.Length() ) == 0 );
	ASSERT_LESS_EQUAL( str.Length(), str.Capacity() );
	ASSERT_EQUAL(str.Length(), 5L);
}

void StringTest::subStringWithNull ()
{
	String source((void *)"abcdefg\000hijklmn", 14);

	ASSERT_EQUAL("bcdefg", source.SubString(1, 6));
	String str1 = source.SubString(2, 7);
	ASSERT_EQUAL("cdefg\000", str1);
	ASSERT(memcmp("cdefg\000", str1, 6) == 0);

	String str2 = source.SubString(3, 8);
	ASSERT_EQUAL("defg\000h", str2);
	ASSERT(memcmp("defg\000h", str2, 6) == 0);
} // subStringWithNull

void StringTest::subStrings()
{
	StartTrace(StringTest.subStrings);
	subString0();
	subString1();
	subString2();
	subString3();
	subString4();
	subString5();
	subStringWithNull();
}
//==============================================================================================
//         S U B S T R I N G           Ende
//==============================================================================================

//==============================================================================================
//         T R I M F R O N T           Beginn
//==============================================================================================
void StringTest::trimFront0 ()
{
	String	str, strHlp;
	long	i;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// Normal cases
	for ( i = 0; i < str.Length() ; i++ ) {
		str = strHlp;
		str.TrimFront(i);
		ASSERT_EQUAL(str.Length(), (long)strlen("0123456789") - i );
		ASSERT ( str.Length() <= str.Capacity() );
	}
}

void StringTest::trimFront1 ()
{
	String str;
	str.Append( "0123456789", strlen("0123456789") );
	// newStart < 0: nothing changes
	str.TrimFront( -5 );
	ASSERT_EQUAL(str.Length(), (long)strlen("0123456789"));
	ASSERT ( str.Length() <= str.Capacity() );
	ASSERT ( memcmp( (const char *)str, "0123456789", str.Length() ) == 0 );
}

void StringTest::trimFront2 ()
{
	String str;
	str.Append( "0123456789", strlen("0123456789") );
	// newStart > Length()
	str.TrimFront( str.Length() + 1 );
	ASSERT_EQUAL(str.Length(), 0L);
	ASSERT ( str.Capacity() >=  str.Length() );
}

void StringTest::trimFrontXXX ()
{
	String tst("X");

	ASSERT_EQUAL(1, tst.Length());
	tst.TrimFront(1);
	ASSERT_EQUAL("", tst);
	ASSERT_EQUAL(0L, tst.Length());

}

void StringTest::trimFrontEmpty ()
{
	String tst;
	ASSERT_EQUAL(0, tst.Length());
	tst.TrimFront(1);
	ASSERT_EQUAL("", tst);
	ASSERT_EQUAL(0L, tst.Length());
}

void StringTest::trimFrontOneByte ()
{
	String tst("longstringmorethanone");
	long length = tst.Length();
	ASSERT_GREATER_EQUAL(tst.Length(), 1L);
	tst.TrimFront(1);
	ASSERT_EQUAL(length - 1, tst.Length());
	ASSERT_EQUAL("ongstringmorethanone", tst);
}

void StringTest::trimFronts()
{
	StartTrace(StringTest.trimFronts);
	trimFront0();
	trimFront1();
	trimFront2();
	trimFrontOneByte();
	trimFrontXXX();
}
//==============================================================================================
//         T R I M F R O N T           Ende
//==============================================================================================

//==============================================================================================
//         T R I M                    Beginn
//==============================================================================================
void StringTest::trim0 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newlen < 0:  leeres String
	str.Trim( -1 );
	ASSERT_EQUAL(str.Length(), 0L);
	ASSERT ( str.Capacity() >=  str.Length() );
}

void StringTest::trim1 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newlen > Length()
	str = strHlp;
	str.Trim( str.Length() + 1 );		// Trim does *not* increase the length (anymore)
	ASSERT_EQUAL(str.Length(), strHlp.Length());
	ASSERT_EQUAL(str.Capacity(), strHlp.Capacity());
	ASSERT ( str == strHlp );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, strHlp.Length() ) == 0 );
}

void StringTest::trim2 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newLen > Capacity()
	str = strHlp;
	long oldLength = str.Length();				// old length
	long oldCapacity = str.Capacity();		// old capacity
	str.Trim( str.Capacity() + 5 );
	// semantic change: must not change capacity or length if capacity>length
	ASSERT_EQUAL( oldLength, str.Length() );
	ASSERT_EQUAL( oldCapacity, str.Capacity() );
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( str == strHlp );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, strHlp.Length() ) == 0 );
}

void StringTest::trim3 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newLen == 0
	str = strHlp;
	str.Trim( 0 );
	ASSERT_EQUAL(str.Length(), 0L);
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( str != strHlp );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::trim4 ()
{
	String str, strHlp;
	long	length;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newLen > 0  and  < Length()
	str = strHlp;
	length = str.Length();
	str.Trim( length - 5 );
	ASSERT ( str.Length()   == length - 5 );
	ASSERT ( str.Capacity() >= str.Length() );
	ASSERT ( str != strHlp );
	ASSERT ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::trims()
{
	StartTrace(StringTest.trims);
	trim0();
	trim1();
	trim2();
	trim3();

}
//==============================================================================================
//         T R I M                    Ende
//==============================================================================================

//==============================================================================================
//         T R I M R E S E R V E      Beginn
//==============================================================================================

void StringTest::reserve ()
{
	String test;

	ASSERT_EQUAL( test.Capacity(), 0);
	test.Reserve(9);					// you will get twice that much!
	ASSERT_GREATER_EQUAL(test.Capacity(), 9L);

	long oldCapacity = test.Capacity();
	test.Append("Something");
	ASSERT_EQUAL( test.Length(), 9);
	ASSERT( test.Capacity() > test.Length() ); 	// must be more
	ASSERT_EQUAL(oldCapacity, test.Capacity());
	long rest = test.Capacity() - test.Length();
	test.Reserve(rest - 1);
	// if you ask for little enough you won't get more
	ASSERT_EQUAL(oldCapacity, test.Capacity());
	ASSERT(test.Capacity() > test.Length() + rest - 1);
	test.Reserve(rest);
	// but ask for one more and get a lot more ...
	ASSERT(test.Capacity() > oldCapacity);
	ASSERT(test.Capacity() > test.Length() + rest);
}

//==============================================================================================
//         R E S E R V E      Ende
//==============================================================================================
void StringTest::TestCapacity()
{
	StartTrace(StringTest.TestCapacity);
	long i;
	String test;
	for ( i = 1; i < (3 * 4048); i++ ) {
		test.Append( 'a' );
		ASSERT_EQUAL(test.Length(), i);
		ASSERT( test.Capacity() > test.Length() );
		ASSERT( ( (test.Capacity() < 8192) || ( test.Capacity() <= test.Length() + 1024 )) );
	}
}

void StringTest::GetLine()
{
	String test;
	{
		String base("First line\nSecond line\n");
		IStringStream is(base);

		getline(is, test);
		ASSERT_EQUAL("First line", test);
		getline(is, test);
		ASSERT_EQUAL("Second line", test);
	}
	{
		String base("First line\r\nSecond line\r\n");
		IStringStream is(base);

		getline(is, test);
		ASSERT_EQUAL("First line\r", test);
		getline(is, test);
		ASSERT_EQUAL("Second line\r", test);
	}
} // GetLine

void StringTest::OptimizedConstructorOrAssignment()
{
	StartTrace(StringTest.OptimizedConstructorOrAssignment);
	const char *s = "Hello world";

	String ss(s);
	ASSERT_EQUAL(strlen(s) + 1, ss.Capacity());
	ss.Append(s);
	ASSERT(ss.Capacity() > long(2 * strlen(s)));
	ASSERT(ss.Capacity() > ss.Length());

	String t;
	ASSERT_EQUAL(0, t.Capacity());
	t = s;
	ASSERT_EQUAL(strlen(s) + 1, t.Capacity());
}

void StringTest::EmptyAllocatorTest()
{
	String str(0L);
	ASSERT(str.GetAllocator() == coast::storage::Current());
	str = "foo";
	ASSERT_EQUAL("foo", str);

	String str1((Allocator *)0);
	ASSERT(str1.GetAllocator() == coast::storage::Current());
	ASSERT(str1.GetAllocator() != 0);
	str1 = "bah";
	ASSERT_EQUAL("bah", str1);
}

void StringTest::appendsWithDelimiter()
{
	StartTrace(StringTest.appendsWithDelimiter);
	{
		String testinput =
			"-----------------------------210003122518197\n"
			"Content-Disposition: inline; name=\"kimdojo1\"; filename=\"C:\\karate\\pics\\hank.gif\"\n"
			"\n" /* this has been the problem of String::Append(std::istream&,long,char) */
			"A492993f39393939f393939393f393939r393\n"
			"-----------------------------210003122518197--\n";
		IStringStream is(testinput);
		String result;
		while ( !is.eof() ) {
			result.Append(is, 4096, '\n');
			ASSERT(is.eof() || is.good());
			if ('\n' == is.peek()) {
				result.Append((char)is.get());    // consume EOL
			}
		}
		ASSERT_EQUAL(testinput, result);
	}
	{
		char const eol = '\n';
		String result;
		String expected = "0123456789";
		String testinput= "0123456789\n";
		IStringStream iss(testinput);
		while ( iss.good() && iss.peek() != eol) {
			result.Append(iss, 3, '\n');
		}
		ASSERT_EQUAL(expected, result);
	}
	{
		String result;
		String expected = "012";
		String testinput= "0123456789\n";
		IStringStream iss(testinput);
		result.Append(iss, 3, '\n');
		ASSERT_EQUAL(expected, result);
	}
	{
		String result;
		String testinput5K;
		for (long i = 0; i < 5 * 1024; i++) {
			testinput5K.Append((char)('0' + (i % 10)));
		}
		IStringStream iss(testinput5K);
		while ( iss.good() ) {
			String tmp;
			tmp.Append(iss, 4096, '\n');
			result.Append(tmp);
		}
		ASSERT_EQUAL(testinput5K.Length(), 5 * 1024);
		ASSERT_EQUAL(testinput5K, result);
	}
}

void StringTest::TestFirstCharOf()
{
	String test("0123456789");

	long lIdx;
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		String containsOneOff;
		containsOneOff.Append(lIdx);
		ASSERT_EQUAL(lIdx, test.FirstCharOf(containsOneOff));
	}
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		String containsOneOff;
		char c(lIdx + 70); // 70 is ascii F
		containsOneOff.Append(c);
		ASSERT_EQUAL(-1, test.FirstCharOf(containsOneOff));
	}
	for ( lIdx = 0; lIdx < test.Length() - 1L; lIdx++ ) {
		String containsOneOff;
		containsOneOff.Append(lIdx);
		containsOneOff.Append(lIdx + 1L);
		ASSERT_EQUAL(lIdx, test.FirstCharOf(containsOneOff));
	}

	// no chars given
	ASSERT_EQUAL(-1L, test.FirstCharOf(""));

	// String on which method is executed is empty
	String empty;
	ASSERT_EQUAL(-1L, empty.FirstCharOf("0123456789"));
}

void StringTest::TestContainsCharAbove()
{
	String test;
	unsigned int ui;
	unsigned int lowbound = 0;
	unsigned int highbound = 256;

	// Tests without exclusion set
	// variation on water mark
	for ( ui = lowbound; ui < highbound; ui++ ) {
		test.Append((char) ui);
	}
	for (ui = lowbound; ui < highbound - 1; ui++) {
		ASSERT_EQUAL(ui - lowbound + 1, test.ContainsCharAbove(ui));
	}
	ASSERT_EQUAL(-1, test.ContainsCharAbove(highbound - 1));

	ASSERT_EQUAL(-1, test.ContainsCharAbove(255));
	ASSERT_EQUAL(0, test.ContainsCharAbove(256));

	// Tests with exclusion set
	for (ui = lowbound; ui < highbound; ui++) {
		ASSERT_EQUAL(-1, test.ContainsCharAbove(ui, test.SubString(ui - lowbound + 1)));
	}

	for (ui = lowbound; ui < highbound - 1; ui++) {
		ASSERT_EQUAL(ui - lowbound + 1, test.ContainsCharAbove(ui, test.SubString(ui - lowbound + 2)));
	}
	ASSERT_EQUAL(-1, test.ContainsCharAbove(ui, test.SubString(ui - lowbound + 2)));

	String empty;
	ASSERT_EQUAL(-1, empty.ContainsCharAbove(255));
	ASSERT_EQUAL(0, empty.ContainsCharAbove(256));
	ASSERT_EQUAL(-1, empty.ContainsCharAbove(255, "��"));
	ASSERT_EQUAL(0, empty.ContainsCharAbove(256, "��"));
}

void StringTest::TestLastCharOf()
{
	String test("0123456789");

	long lIdx;
	String containsOneOff;
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		containsOneOff.Append(lIdx);
		ASSERT_EQUAL(lIdx + 1, test.LastCharOf(containsOneOff));
	}
	containsOneOff.Trim(0);
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		// like above test, and adding char not in set
		for (long llIdx = 0; llIdx < lIdx; llIdx++) {
			containsOneOff.Append(llIdx);
		}
		char c(lIdx + 70);
		containsOneOff.Append(c);
		ASSERT_EQUAL((lIdx == 0 ? -1 : lIdx), test.LastCharOf(containsOneOff));
		containsOneOff.Trim(0);
	}
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		// reverse order
		for (long llIdx = lIdx; llIdx >= 0; llIdx--) {
			containsOneOff.Append(llIdx);
		}
		char c(lIdx + 70);
		containsOneOff.Append(c);
		ASSERT_EQUAL((lIdx + 1), test.LastCharOf(containsOneOff));
		containsOneOff.Trim(0);
	}

	// no chars given
	ASSERT_EQUAL(-1L, test.LastCharOf(""));

	// String on which method is executed is empty
	String empty;
	ASSERT_EQUAL(-1L, empty.LastCharOf("0123456789"));
}

void StringTest::DoubleToStringTest()
{
	StartTrace(StringTest.DoubleToStringTest);
	{
		String strBuf;
		double dValue = 1.0;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "1.0", strBuf);
	}
	{
		String strBuf;
		double dValue = 1000000;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "1000000.0", strBuf);
	}
	{
		String strBuf;
		double dValue = 1.23456789;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "1.23456789", strBuf);
	}
	{
		String strBuf;
		double dValue = 0.0;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "0.0", strBuf);
	}
	{
		String strBuf;
		double dValue = 0.056;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "0.056", strBuf);
	}
	{
		// string will be cut 15 decimals after decimal point
		String strBuf;
		double dValue = (1.0 / 3.0);
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "0.333333333333333", strBuf);
	}
	{
		// string will be cut 15 decimals after decimal point
		String strBuf;
		double dValue = (100.0 / 3.0);
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "33.333333333333336", strBuf);
	}
	{
		// string will be cut 15 decimals after decimal point
		String strBuf;
		double dValue = 27.16;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "27.16", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e15 + 0.123456789;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "1000000000000000.125", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e14 + 0.123456789;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "100000000000000.125", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e12 + 0.123456789;
		String::DoubleToString(dValue, strBuf);
		ASSERT_EQUAL( "1000000000000.1234130859375", strBuf);
	}
	{
		String strBuf;
		double dValue = (1e+20) + 0.22134;
		String::DoubleToString(dValue, strBuf);
#if defined(WIN32)
		ASSERT_EQUAL( "1e+020", strBuf);
#else
		ASSERT_EQUAL( "1e+20", strBuf);
#endif
	}
	{
		String strBuf;
		double dValue = 1e+30;
		String::DoubleToString(dValue, strBuf);
#if defined(WIN32)
		ASSERT_EQUAL( "1e+030", strBuf);
#else
		ASSERT_EQUAL( "1e+30", strBuf);
#endif
	}
}
