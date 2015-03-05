/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingNotNullAction_H
#define _AnythingNotNullAction_H

#include "Action.h"

//---- AnythingNotNullAction ----------------------------------------------------------
//! Check if a given slot in a store is not null
/*!
\par Configuration
\code
{
	/Store	String			optional, default TempStore one of [Role|Session|Query|Fields], lookup Slot in this store
	/Slot	Rendererscript	mandatory, slotname to check for
}
\endcode
*/
class AnythingNotNullAction : public Action
{
public:
	//--- constructors
	AnythingNotNullAction(const char *name);
	~AnythingNotNullAction();

	//! DoSomething method for configured Actions
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
