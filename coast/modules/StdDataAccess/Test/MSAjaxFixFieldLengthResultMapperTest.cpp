/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#include "MSAjaxFixFieldLengthResultMapperTest.h"
#include "MSAjaxFixFieldLengthResultMapper.h"
#include "TestSuite.h"
#include "HierarchyInstallerWithConfig.h"
#include "CheckStores.h"
#include "Context.h"

namespace {
    const char *_Value = "Value";
    const char *_Stream = "Stream";
	bool setupMappers(ROAnything roaMapperConfigs) {
		StartTrace(MSAjaxFixFieldLengthResultMapperTest.setupMappers);
		Anything mappersToInitialize;
		ROAnything mapperConfig;
		AnyExtensions::Iterator<ROAnything> aMapperConfigIterator(roaMapperConfigs);
		String mapperName;
		while (aMapperConfigIterator.Next(mapperConfig)) {
			if (aMapperConfigIterator.SlotName(mapperName)) {
				mappersToInitialize[mapperName] = Anything();
			}
		}
		HierarchyInstallerWithConfig ip(ResultMapper::gpcCategory, roaMapperConfigs);
		return RegisterableObject::Install(mappersToInitialize, ResultMapper::gpcCategory, &ip);
	}
	void unregisterMappers() {
		StartTrace(MSAjaxFixFieldLengthResultMapperTest.unregisterMappers);
		AliasTerminator at(ResultMapper::gpcCategory);
		RegisterableObject::Terminate(ResultMapper::gpcCategory, &at);
	}
	void setupContext(ROAnything caseConfig, Context &ctx) {
		coast::testframework::PutInStore(caseConfig["SessionStore"], ctx.GetSessionStore());
		coast::testframework::PutInStore(caseConfig["RoleStore"], ctx.GetRoleStoreGlobal());
		coast::testframework::PutInStore(caseConfig["TmpStore"], ctx.GetTmpStore());
		coast::testframework::PutInStore(caseConfig["Query"], ctx.GetQuery());
		coast::testframework::PutInStore(caseConfig["Env"], ctx.GetEnvStore());
	}
}

void MSAjaxFixFieldLengthResultMapperTest::ConfiguredTests() {
	StartTrace(MSAjaxFixFieldLengthResultMapperTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if (!aEntryIterator.SlotName(caseName)) {
			t_assertm(false, "can not execute with unnamed case name, only named anything slots allowed");
			continue;
		}
		String putKeyName = caseConfig["Putkey"].AsString("HTTPHeader");
		if (t_assertm(setupMappers(caseConfig["MapperConfig"]), "ResultMapper setup must succeed to execute tests")) {
			String mapperName = caseConfig["MapperConfig"].SlotName(0L);
			ResultMapper *rm = ResultMapper::FindResultMapper(mapperName);
			if (t_assertm(rm != 0, TString("could not find mapper [") << mapperName.cstr() << "]")) {
				Context ctx;
				setupContext(caseConfig, ctx);
				ROAnything roaValue;
				if (caseConfig.LookupPath(roaValue, _Value)) {
					Anything value = roaValue.DeepClone();
					t_assertm(rm->Put(putKeyName, value, ctx), caseName);
				} else  if (caseConfig.LookupPath(roaValue, _Stream)) {
					String strValue = roaValue.AsString();
					IStringStream stream(strValue);
					t_assertm(rm->Put(putKeyName, stream, ctx), caseName);
				} else {
					t_assertm(false, TString("neither ").Append(_Value).Append(" nor ").Append(_Stream).Append(" is defined in configuration for ").Append(caseName));
					continue;
				}
				Anything anyFailureStrings;
				coast::testframework::CheckStores(anyFailureStrings, caseConfig["Result"], ctx, caseName, coast::testframework::exists);
				// non-existence tests
				coast::testframework::CheckStores(anyFailureStrings, caseConfig["NotResult"], ctx, caseName,
						coast::testframework::notExists);
				for (long sz = anyFailureStrings.GetSize(), i = 0; i < sz; ++i) {
					t_assertm(false, anyFailureStrings[i].AsString().cstr());
				}
			}
			unregisterMappers();
		}
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *MSAjaxFixFieldLengthResultMapperTest::suite() {
	StartTrace(MSAjaxFixFieldLengthResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MSAjaxFixFieldLengthResultMapperTest, ConfiguredTests);
	return testSuite;
}

