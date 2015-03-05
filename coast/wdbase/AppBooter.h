/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AppBooter_H
#define _AppBooter_H

#include "Anything.h"
class Application;

//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class AppBooter
{
	// load the global config file with the information
	// set in the environment variable COAST_ROOT
	// or with a minimal relative path
public:
	AppBooter();
	~AppBooter();
	int Run(int argc, const char *argv[], bool doHalt = true);

	bool Boot(Anything &config); // access the intial config file
	Application *FindApplication(ROAnything config, String &applicationName);

	bool OpenLibs(const Anything &config);
	bool CloseLibs();
	bool ReadFromFile(Anything &config, const char *filename); // reading of configuration files
	void HandleArgs(int argc, const char *argv[], Anything &config);
	void SetSignalMask();

	//!sets COAST_ROOT and COAST_PATH if set in args; returns bootfilename
	String PrepareBootFileLoading(const ROAnything &roconfig);

	//!merges command line arguments into global configuration anything
	void MergeConfigWithArgs(Anything &config, const Anything &args);

	void Halt(const Anything &config);

private:
	Anything fLibArray;
	friend class AppBooterTest;
};

#endif
