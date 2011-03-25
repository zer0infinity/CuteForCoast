#ifndef StringIteratorTest_H
#define StringIteratorTest_H

#include "TestCase.h"//lint !e537
#include "ITOString.h"//lint !e537

class StringIteratorTest: public TestFramework::TestCase {
protected:
	String fStr5; // string with 5 elements set-up in setUp
public:
	StringIteratorTest(TString tname) :
		TestCaseType(tname) {
	}

	virtual void setUp();
	static Test *suite();

	void testEmptyStringBegin();
	void testSimpleStringBegin();
	void testSimpleStringDeref();
	void testSimpleStringIncrement();
	void testSimpleStringDecrement();
	void testSimpleStringIncDec();
	void testSimpleStringAssignment();
	void testStringIteration();
	void testStringIndex();
	void testIteratorSubstract();
	void testIteratorIntAdd();
	void testIteratorIntSubstract();
	void testIteratorCompare();
};

#endif
