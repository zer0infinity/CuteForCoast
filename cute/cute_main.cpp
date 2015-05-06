/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"
#include "cute.h"
#include "cute_case.h"

void setupSuite(cute::suite &s) {}

int main(int argc, char const *argv[]) {
	cute::suite s;
	setupSuite(s);
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<> > lis(xmlfile.out);
	cute::makeRunner(lis)(s, "AllTests");
	return 0;
}
