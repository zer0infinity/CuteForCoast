/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROAnyConfNamedObjectLookupAdapterTest_H
#define _ROAnyConfNamedObjectLookupAdapterTest_H

#include "TestCase.h"

#include "Registry.h"

//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ROAnyConfNamedObjectLookupAdapterTest : public testframework::TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	ROAnyConfNamedObjectLookupAdapterTest(TString tstrName);

	//! destroys the test case
	~ROAnyConfNamedObjectLookupAdapterTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	void LookupTest();
	void NoConfNamedObjectTest();
	void NothingAtAllTest();
};

class TestConfNamedObj : public ConfNamedObject
{
public:
	//--- constructors
	TestConfNamedObj(const char *name);
	~TestConfNamedObj() {} ;

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const ;
	RegCacheDef(TestConfNamedObj);	// FindTestConfNamedObj()

protected:
	bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;
	bool DoLoadConfig(const char *category);

private:
	// Only set at class initialization, will never be changed, so it is safe in singleton context.
};

#define RegisterTestConfNamedObj(name) RegisterObject(name, TestConfNamedObj)
#endif
