/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ServiceHandler.h"
#include "Policy.h"

const char* ServiceHandler::gpcCategory = "ServiceHandler";
const char* ServiceHandler::gpcConfigPath = "ServiceHandlers";

ServiceHandler::ServiceHandler(const char *ServiceHandlerName) :
	HierarchConfNamed(ServiceHandlerName) {
	StatTrace(ServiceHandler.Ctor, ServiceHandlerName, coast::storage::Current());
}

bool ServiceHandler::HandleService(std::ostream &os, Context &ctx) {
	StartTrace(ServiceHandler.HandleService);
	Trace("Service [" << fName << "]");
	String strKey("ServiceHandler");
	ctx.Push(strKey, this);
	bool status = DoHandleService(os, ctx);
	ctx.Pop(strKey); //!@FIXME: use PushPopEntry for LookupInterfaces too
	return status;
}

RegCacheImpl(ServiceHandler);
RegisterModule(ServiceHandlersModule);

bool ServiceHandlersModule::Init(const ROAnything config) {
	ROAnything roaHandlerConfig;
	if (config.LookupPath(roaHandlerConfig, ServiceHandler::gpcConfigPath)) {
		HierarchyInstaller ai(ServiceHandler::gpcCategory);
		return RegisterableObject::Install(roaHandlerConfig, ServiceHandler::gpcCategory, &ai);
	}
	return false;
}

bool ServiceHandlersModule::ResetFinis(const ROAnything config) {
	AliasTerminator at(ServiceHandler::gpcCategory);
	return RegisterableObject::ResetTerminate(ServiceHandler::gpcCategory, &at);
}

bool ServiceHandlersModule::Finis() {
	return StdFinis(ServiceHandler::gpcCategory, ServiceHandler::gpcConfigPath);
}
