/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RemoveAction.h"
#include "AnythingUtils.h"
#include "Renderer.h"
#include "Tracer.h"

//---- RemoveAction ---------------------------------------------------------------
RegisterAction(RemoveAction);

bool RemoveAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(RemoveAction.DoExecAction);

	String slotName;
	Renderer::RenderOnString(slotName, ctx, config["Slot"]);
	if (slotName.Length()) {
		// first of all, get the correct store
		String store = config["Store"].AsString("TmpStore");
		char delim = config["Delim"].AsCharPtr(".")[0L];
		char indexdelim = config["IndexDelim"].AsCharPtr(":")[0L];
		// must use reference here to prevent copying the content due to different allocator locations
		Anything &anyStore = StoreFinder::FindStore(ctx, store), anyParent(anyStore, anyStore.GetAllocator());
		SubTraceAny(TraceContent, anyParent, String("content in store [") << store << "], looking for slot [" << slotName << "]");

		// test if the path to be deleted exists in the store, avoids creation of nonexisting slot
		// also avoid content copying here using same allocator as parent any
		Anything anySlotTest(anyParent.GetAllocator());
		if (anyParent.LookupPath(anySlotTest, slotName, delim, indexdelim)) {
			// use SlotFinders IntOperate to get the correct parent anything and the slotname/index
			// to remove from
			long slotIndex = -1L;
			if (SlotFinder::IntOperate(anyParent, slotName, slotIndex, delim, indexdelim)) {
				if (slotName.Length()) {
					Trace("removing named slot [" << slotName << "]");
					anyParent.Remove(slotName);
				} else if (slotIndex != -1L) {
					Trace("removing index slot [" << slotIndex << "]");
					anyParent.Remove(slotIndex);
				}
				return true;
			}
		} else {
			Trace("path to be deleted not found! [" << slotName << "]");
			// as we do not have to delete anything we return true anyway
			return true;
		}
	}
	return false;
}
