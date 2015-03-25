/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataAccessStresser_h_
#define _DataAccessStresser_h_

#include "Stresser.h"

//! Simple Stresser which executes a DataAccess
/*!
\par Configuration
\code
{
	/DataAccess							mandatory, Name of the DataAccess to use
	/NumberOfCalls						optional, default 10, how many times the DataAccess is called
	/InfoMessage {						optional, specify this slot and configure it if you like to print DataAccess ouput in the summary
		/Source.Slot	OutputSlotName	optiona, the "Source.Slot" will be looked up in the TmpStore of the executed DataAccess and put into OutputSlotName
		...
	}
}
\endcode

*/
class DataAccessStresser: public Stresser {
public:
	DataAccessStresser(const char *DataAccessStresserName) :
		Stresser(DataAccessStresserName) {
	}
	/*! The method doing the DataAccess and summarize the results
	 \param id A number to identify this Stresser instance, used when more than one Stresser is used
	 \return The stressers summary Anything which will be analyzed from within StressApp::ShowResult() */
	virtual Anything Run(long id);

	/*! Cloning interface
	 \return Instance of this class */
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DataAccessStresser(fName);
	}
};

#endif
