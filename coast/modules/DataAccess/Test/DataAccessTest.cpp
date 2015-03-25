/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DataAccessTest.h"
#include "TestSuite.h"
#include "DataAccess.h"
#include "Session.h"
#include "Context.h"

//---- DataAccessTest ----------------------------------------------------------------
Test *DataAccessTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, DataAccessTest, GetImplTest);
	ADD_CASE(testSuite, DataAccessTest, ExecTest);
	ADD_CASE(testSuite, DataAccessTest, CopySessionStoreTest);
	return testSuite;
}

DataAccessTest::DataAccessTest(TString tname)
	: TestCaseType(tname)
{
}

DataAccessTest::~DataAccessTest()
{
}

class GetImplDataAccess : public DataAccess {
public:
	GetImplDataAccess(const char* name) : DataAccess(name) {}
	DataAccessImpl *MyGetImpl(const char *daName, Context &context) {
		return GetImpl(daName, context);
	}
};

void DataAccessTest::GetImplTest()
{
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());

	const char *daName = "daTest";
	GetImplDataAccess daTest(daName);
	DataAccessImpl *m = daTest.MyGetImpl(daName, ctx);

	t_assert( m != 0 );

	daName = "daTestNone";
	GetImplDataAccess daTestNone(daName);
	DataAccessImpl *none = daTestNone.MyGetImpl(daName, ctx);

	t_assert( !none );
	String str(tmpStore["DataAccess"][daName]["Error"][0L].AsCharPtr());
	t_assert(str.Contains("DataAccessImpl::FindDataAccessImpl returned 0 for daTestNone"));
}

void DataAccessTest::ExecTest()
{
	StartTrace(DataAccessTest.ExecTest);
	Anything dummy;
	Context ctx(GetTestCaseConfig().DeepClone(), dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());

	const char *daName = "LoopBackDATest";
	t_assert(DataAccess(daName).StdExec(ctx));

	TraceAny(tmpStore, "TempStore:");
	assertEqual("Hello", tmpStore["Mapper"]["mReply"].AsCharPtr());
	assertEqual("Loopback try", tmpStore["Mapper"]["hReply"].AsCharPtr());
	assertEqual("coast bsshht", tmpStore["Mapper"]["fReply"].AsCharPtr());
	assertEqual("XXX", tmpStore["Mapper"]["cReply"].AsCharPtr());
}

class SessionUnlockTestDAImpl : public DataAccessImpl
{
public:
	SessionUnlockTestDAImpl(const char *name): DataAccessImpl(name) {}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SessionUnlockTestDAImpl(fName);
	}
	virtual bool Exec(Context &c, ParameterMapper *input, ResultMapper *output) {
		c.GetTmpStore()["session"] = c.GetSession()->IsLockedByMe() ? "LOCKED" : "unlocked";
		return true;
	}
};

RegisterDataAccessImpl(SessionUnlockTestDAImpl);

void DataAccessTest::CopySessionStoreTest()
{
	StartTrace(DataAccessTest.SessionUnlockTest);
	Context sessionctx;
	Session s("SessionUnlockTest");
	{
		Context c;
		c.Push(&s);
		t_assert(s.IsLockedByMe());
		DataAccess da("SessionUnlockTestDAImpl");
		ParameterMapper p("SessionUnlockTestDAImpl");
		ResultMapper r("SessionUnlockTestDAImpl");
		t_assert(da.Exec(&p, &r, c));
		t_assert(c.GetTmpStore().IsDefined("session"));
		assertEqual("LOCKED", c.GetTmpStore()["session"].AsCharPtr(""));
	}
	{
		Context c;
		c.GetTmpStore()["Context"]["CopySessionStore"] = true;
		c.Push(&s);
		t_assert(!s.IsLockedByMe());
		s.fMutex.Lock();
		t_assert(s.IsLockedByMe());
		DataAccess da("SessionUnlockTestDAImpl");
		ParameterMapper p("SessionUnlockTestDAImpl");
		ResultMapper r("SessionUnlockTestDAImpl");
		t_assert(da.Exec(&p, &r, c));
		t_assert(c.GetTmpStore().IsDefined("session"));
		assertEqual("unlocked", c.GetTmpStore()["session"].AsCharPtr(""));
		t_assert(s.IsLockedByMe());
		s.fMutex.Unlock();
		t_assert(!s.IsLockedByMe());
	}
}
