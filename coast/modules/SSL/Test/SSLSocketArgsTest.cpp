/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLSocketArgsTest.h"
#include "TestSuite.h"
#include "Tracer.h"

//---- SSLSocketArgsTest ----------------------------------------------------------------

SSLSocketArgsTest::SSLSocketArgsTest(TString tname) : TestCaseType(tname)
{
}

SSLSocketArgsTest::~SSLSocketArgsTest()
{
	StartTrace(SSLSocketArgsTest.Dtor);
}

void SSLSocketArgsTest::ArgsTest()
{
	StartTrace(SSLSocketArgsTest.ArgsTest);
	SSLSocketArgs sslsa(true, "gugus", true, true);
	assertEqual(sslsa.VerifyCertifiedEntity(), true);
	assertEqual(sslsa.CertVerifyString(), "gugus");
	assertEqual(sslsa.CertVerifyStringIsFilter(), true);
	assertEqual(sslsa.SessionResumption(), true);

	SSLSocketArgs sslsa1(0, "", 0, 0);
	assertEqual(sslsa1.VerifyCertifiedEntity(), false);
	assertEqual(sslsa1.CertVerifyString(), "");
	assertEqual(sslsa1.CertVerifyStringIsFilter(), false);
	assertEqual(sslsa1.SessionResumption(), false);
	// Default constructor
	SSLSocketArgs  sslsa2;
	assertEqual(sslsa2.VerifyCertifiedEntity(), false);
	assertEqual(sslsa2.CertVerifyString(), "");
	assertEqual(sslsa2.CertVerifyStringIsFilter(), false);
	assertEqual(sslsa2.SessionResumption(), false);

	// assignment operator
	SSLSocketArgs sslsa3;
	sslsa3 = sslsa;
	assertEqual(sslsa3.VerifyCertifiedEntity(), true);
	assertEqual(sslsa3.CertVerifyString(), "gugus");
	assertEqual(sslsa3.CertVerifyStringIsFilter(), true);
	assertEqual(sslsa3.SessionResumption(), true);

	// copy constructor
	SSLSocketArgs sslsa4(sslsa);
	assertEqual(sslsa4.VerifyCertifiedEntity(), true);
	assertEqual(sslsa4.CertVerifyString(), "gugus");
	assertEqual(sslsa4.CertVerifyStringIsFilter(), true);
	assertEqual(sslsa4.SessionResumption(), true);
}

// builds up a suite of tests, add a line for each testmethod
Test *SSLSocketArgsTest::suite ()
{
	StartTrace(SSLSocketArgsTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SSLSocketArgsTest, ArgsTest);
	return testSuite;
}
