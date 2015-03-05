/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StresserThread.h"
#include "Stresser.h"
#include "Timers.h"

void StresserThread::Init(const String &StresserName, long Id, Condition *Cond, Mutex *Mtx, long *Counter) {
	StartTrace1(StresserThread.Init, "Stresser: " << StresserName << " / " << Id);

	fMyId = Id;
	fCond = Cond;
	fMutex = Mtx;
	fPending = Counter;
	fStresserName = StresserName;
}

void StresserThread::Run() {
	StartTrace1(StresserThread.Run, "running: " << fMyId << " with<" << fStresserName << ">");

	DiffTimer dt;
	fResult = Stresser::RunStresser(fStresserName, fMyId);
	Trace ("Finished " << fMyId << " : " << (String)(dt.Diff() / 1000) );
}

void StresserThread::DoTerminatedHook() {
	StartTrace(StresserThread.DoTerminatedHook);

	// synchronize with main thread (StressApp)
	LockUnlockEntry me(*fMutex);
	(*fPending)--;
	// give StressApp the opportunity to check if this is
	// the last thread it is waiting for..
	fCond->Signal();
}

Anything StresserThread::GetResult() {
	StartTrace(StresserThread.GetResult);
	TraceAny(fResult, "Result");
	return fResult;
}
