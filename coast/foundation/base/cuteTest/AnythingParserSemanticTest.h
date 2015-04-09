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

#ifndef _AnythingParserSemanticTest_h_
#define _AnythingParserSemanticTest_h_

#include "cute.h"
#include "Anything.h"//lint !e537
class AnythingParserSemanticTest {
protected:
	Anything emptyAny, anyTemp0, anyTemp1, anyTemp2, anyTemp3, anyTemp4;

	Anything storeAndReload(Anything);
	void writeResult(String *input, long nrOfElt, char *path, char *ext);
	void checkImportExport(Anything any, String fileName);
	void scanAnything(Anything);
	Anything anyOutput;
	long lineCounter;

public:
	AnythingParserSemanticTest();
	static void runAllTests(cute::suite &s);

	void Semantic0Test();
	void Semantic1Test();
	void Semantic2Test();
	void Semantic3Test();
	void Semantic4Test();
	void Semantic5Test();
	void Semantic6Test();
	void Semantic7Test();
	void Semantic8Test();
	void Semantic9Test();
	void Semantic10Test();
	void Semantic11Test();
	void Semantic12Test();
	void Semantic13Test();
	void Semantic14Test();
	void Semantic15Test();
	void Semantic16Test();
	void Semantic17Test();
	void Semantic18Test();
	void Semantic19Test();
	void Semantic20Test();
	void Semantic21Test();
	void Semantic22Test();
	void Semantic23Test();
	void Semantic24Test();
	void Semantic25Test();
	void Semantic26Test();
	void Semantic27Test();
	void testSemantic28Prep(Anything *any);
	void Semantic28Test();
	void testSemantic29Prep(Anything *any);
	void Semantic29Test();
	void testSemantic30Prep(Anything &any);
	void Semantic30Test();
	void Semantic31Test();
	void Semantic32Test();
	void Semantic33Test();
	void slashSlotnames();
	void QuotedSlotnameTest();
};
#endif
