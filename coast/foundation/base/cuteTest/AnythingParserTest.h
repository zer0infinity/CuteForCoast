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

#ifndef _AnythingParserTest_h_
#define _AnythingParserTest_h_

#include "cute.h"
#include "Anything.h"//lint !e537
class AnythingParserTest {
protected:
	Anything	emptyAny, anyTemp0, anyTemp1,
				anyTemp2, anyTemp3, anyTemp4;

	Anything	storeAndReload( Anything );
	void		writeResult( String *input , long nrOfElt, const char *path, const char *ext );
	void		checkImportExport( Anything any, String fileName );
	void		scanAnything( Anything );
	Anything	anyOutput;
	long		lineCounter;
	void 		assertParsedAsDouble(const char *in, double val, int id);
	void 		assertParsedAsLong(const char *in, long val, int id);
	void 		assertParsedAsString(const char *in, int id);

public:
	AnythingParserTest();
	static void runAllTests(cute::suite &s);

	void		parseSimpleTypeLong();
	void 		IntParseSimpleTypeLong(const String &inp, long exp);
	void		parseSimpleTypeDouble();
	void		parseSimpleTypeNull();
	void		parseSimpleTypeCharPtr();
	void		parseMixedAnything ();
	void		parseNumber();
	void		parseOctal();
	void		parseBinary();
	void		importEmptyStream();
	void		readWriteAnything();
	void		parseTestFiles();
	void		testObjectParsing();
	void		testNumberParsing();
};
#endif
