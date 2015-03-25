/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include <fstream>
#include "WDModule.h"
#include "Context.h"
#include "SecurityModule.h"
#include "Application.h"
#include "AppBooter.h"
#include <unistd.h>

class MyAppBooter : public AppBooter
{
	Application *FindApplication(const Anything &config, String &applicationName) {
		applicationName = "UnscrambleUrl";
		return Application::FindApplication(applicationName);
	}
};

class UnscrambleUrl : public Application
{
public:
	UnscrambleUrl(const char *n) : Application(n) {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) UnscrambleUrl(fName);
	}

	int GlobalInit(int argc, char *argv[], const Anything &config) {
		if (argc < 2) {
			cerr << "Unscramble Coast URL state." << endl;
			cerr << "To run, use same COAST_ROOT and COAST_PATH as the WD-App which generated the URLs" << endl;
			cerr << "Usage: " << argv[0] << " [ <url> ]+" << endl;
			exit(-1);
		}
		for (int i = 1; i < argc; i++) {
			fUrls.Append(argv[i]);
		}

	}
	int GlobalRun() {
		for (long i = 0; i < fUrls.GetSize(); i++) {
			Anything result;
			String url(fUrls[i].AsString(""));
			SecurityModule::UnscrambleState(result, url);
			cout << "Url State: " << url << endl;
			cout << result;
			cout << endl << flush;
		}
		return 0;
	}
private:
	Anything fUrls;
};
RegisterApplication(UnscrambleUrl);

int main(int argc, char **argv)
{
	int result = MyAppBooter().Run(argc, argv);
	return result;
}

