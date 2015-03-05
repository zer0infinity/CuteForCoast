/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLECHECKCLOSEOPENEDCONNECTIONSACTION_H_
#define ORACLECHECKCLOSEOPENEDCONNECTIONSACTION_H_

#include "Action.h"

//! Periodic action to check for timed out connections
/*!
 * The action will get called from within a PeriodicAction through its registered name.
 * The setup of the PeriodicAction is done in coast::oracle::ConnectionPool::Init.
 */
class OracleCheckCloseOpenedConnectionsAction : public Action
{
public:
	/*! Default ctor using a name
	 * @param name unique name to register this action
	 */
	OracleCheckCloseOpenedConnectionsAction(const char *name) : Action(name) { }
	/*! cleans the session list from timeouted sessions
	 * @param transitionToken string passed in when action was executed
	 * @param ctx Context to be used for action processing
	 * @param config actoin specific configuration from context
	 * @return true in case the actions was executed successfully
	 */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif /* ORACLECHECKCLOSEOPENEDCONNECTIONSACTION_H_ */
