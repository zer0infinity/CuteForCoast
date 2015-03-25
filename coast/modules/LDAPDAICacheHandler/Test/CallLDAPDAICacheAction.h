/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CallLDAPDAICacheAction_H
#define _CallLDAPDAICacheAction_H

#include "Action.h"

//---- CallLDAPDAICacheAction ----------------------------------------------------------
//! Action for testing LDAPCachePoliy.
//! Overwrites TmpStore in Context.
class CallLDAPDAICacheAction : public Action
{
public:
	//--- constructors
	CallLDAPDAICacheAction(const char *name);
	~CallLDAPDAICacheAction();

	//!Calls the DataAccess
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the DataAccess run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

};

#endif
