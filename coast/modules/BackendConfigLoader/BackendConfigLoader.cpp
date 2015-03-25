/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "BackendConfigLoader.h"
#include "AnyIterators.h"
#include "SystemFile.h"
#include "HierarchyInstallerWithConfig.h"
#include "DataAccessImpl.h"
#include "ServiceHandler.h"
#include "Page.h"
#include "Role.h"

using namespace coast;

RegisterModule(BackendConfigLoaderModule);

Anything BackendConfigLoaderModule::backendConfigurations = Anything(Anything::ArrayMarker(),storage::Global());

bool BackendConfigLoaderModule::Init(const ROAnything config) {
	StartTrace(BackendConfigLoaderModule.Init);
	bool retCode = true;
	ROAnything roaModuleConfig, roaConfigDir;

	if (config.LookupPath(roaModuleConfig, "BackendConfigLoaderModule") && roaModuleConfig.LookupPath(roaConfigDir, "BackendConfigDir")) {
		TraceAny(roaModuleConfig, "BackendConfigLoaderConfig:");
		SystemLog::WriteToStderr("\tReading Backend Configuration Files");
		String path(roaConfigDir.AsString());
		if (!system::IsAbsolutePath(path)) {
			String cwd(system::GetRootDir());
			path = cwd << "/" << path;
			system::ResolvePath(path);
		}
		Anything dirlist = system::DirFileList(path, "any");
		String backendName;
		ROAnything roaBackend;
		AnyExtensions::Iterator<ROAnything> backendIterator(dirlist);
		while (retCode && backendIterator.Next(roaBackend)) {
			SystemLog::WriteToStderr(".");
			backendName = roaBackend.AsString();
			Trace("processing backend [" << backendName << "]");
			Anything backendConfig;
			if (!system::LoadConfigFile(backendConfig, backendName)) {
				retCode = false;
			}
			backendConfigurations[backendName] = backendConfig;
			retCode = retCode && RegisterBackend(backendName, backendConfig);
		}
		TraceAny(backendConfigurations, "Backend Configurations:");
	} else {
		retCode = false;
	}
	SystemLog::WriteToStderr(retCode ? " done\n" : " failed\n");
	return retCode;
}

bool BackendConfigLoaderModule::Finis() {
	StartTrace(BackendConfigLoaderModule.Finis);
	SystemLog::WriteToStderr("\tTerminating BackendConfigLoader Module done\n");
	return true;
}

ROAnything BackendConfigLoaderModule::GetBackendConfig(const String &backendName) {
	StartTrace(BackendConfigLoaderModule.GetBackendConfig);
	TraceAny(backendConfigurations, "Backend Configurations:");
	return backendConfigurations[backendName];
}

ROAnything BackendConfigLoaderModule::GetBackendConfig() {
	return backendConfigurations;
}

Anything BackendConfigLoaderModule::GetBackendList() {
	StartTrace(BackendConfigLoaderModule.GetBackendList);
	Anything backendList;
	for (int i = 0, sz = backendConfigurations.GetSize(); i < sz; ++i) {
		backendList.Append(backendConfigurations.SlotName(i));
	}
	TraceAny(backendList, "List of all Backends:");
	return backendList;
}

bool BackendConfigLoaderModule::RegisterBackend(const String& backendName, ROAnything roaBackendConfig) {
	StartTrace(BackendConfigLoaderModule.RegisterBackend);
	TraceAny(roaBackendConfig, "Registering backend [" << backendName << "]");
	bool ret = true;
	ROAnything roaObjectConfig;
	if (roaBackendConfig.LookupPath(roaObjectConfig, ParameterMapper::gpcConfigPath)) {
		TraceAny(roaObjectConfig, "ParameterMapper config");
		HierarchyInstallerWithConfig ip(ParameterMapper::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, ParameterMapper::gpcCategory, &ip) && ret;
	}
	if (roaBackendConfig.LookupPath(roaObjectConfig, ResultMapper::gpcConfigPath)) {
		TraceAny(roaObjectConfig, "ResultMapper config");
		HierarchyInstallerWithConfig ip(ResultMapper::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, ResultMapper::gpcCategory, &ip) && ret;
	}
	if (roaBackendConfig.LookupPath(roaObjectConfig, DataAccessImpl::gpcConfigPath)) {
		TraceAny(roaObjectConfig, "DataAccessImpl config");
		HierarchyInstallerWithConfig ip(DataAccessImpl::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, DataAccessImpl::gpcCategory, &ip) && ret;
	}
	if (roaBackendConfig.LookupPath(roaObjectConfig, Page::gpcConfigPath)) {
		TraceAny(roaObjectConfig, "Page config");
		HierarchyInstallerWithConfig ip(Page::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, Page::gpcCategory, &ip) && ret;
	}
	if (roaBackendConfig.LookupPath(roaObjectConfig, Role::gpcConfigPath)) {
		TraceAny(roaObjectConfig, "Role config");
		HierarchyInstallerWithConfig ip(Role::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, Role::gpcCategory, &ip) && ret;
	}
	if (roaBackendConfig.LookupPath(roaObjectConfig, ServiceHandler::gpcConfigPath)) {
		TraceAny(roaObjectConfig, "ServiceHandler config");
		// here we need to pass an unusual slot delim because we want a.b.c to be found as a single piece and not a path
		HierarchyInstallerWithConfig ip(ServiceHandler::gpcCategory, roaBackendConfig, '#');
		ret = RegisterableObject::Install(roaObjectConfig, ServiceHandler::gpcCategory, &ip) && ret;
	}
	return ret;
}
