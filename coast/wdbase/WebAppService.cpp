/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WebAppService.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Session.h"
#include "SessionListManager.h"
#include "RequestProcessor.h"
#include "Server.h"
#include "URLUtils.h"
#include "Dbg.h"

RegisterServiceHandler(WebAppService);
//---- WebAppService ----------------------------------------------------------------
void WebAppService::DoHandleService(std::ostream &reply, Context &ctx)
{
	StartTrace(WebAppService.DoHandleService);

	PrepareRequest(ctx);
	SplitURI2PathAndQuery(ctx.GetRequest()["env"]);
	DecodeWDQuery(ctx.GetQuery(), ctx.GetRequest()["env"]);

	// first stage: verify the request
	if (!VerifyRequest(reply, ctx)) {
		Trace("request verification failed");
		SystemLog::Info("request verification failed");
		RequestProcessor::Error(reply, "Access denied. Lookuptoken: VFSF", ctx);
		return;
	}

	// second stage: prepare the query and get the session id if any
	String sessionId = SessionListManager::SLM()->FilterQueryAndGetId(ctx);
	Trace("SessionId:<" << sessionId << ">");

	// third stage: lookup the session
	bool isBusy = false;
	Session *session = SessionListManager::SLM()->LookupSession(sessionId, ctx);
	Trace("session " << ((session) ? "found" : "not found"));

	// fourth stage: prepare session i.e. check if found, busy and verifiable
	session = SessionListManager::SLM()->PrepareSession(session, isBusy, ctx);
	Trace("session " << ((isBusy) ? "is busy" : "is not busy"));

	// fifth stage: now act on the session
	if (session) {
		ROAnything roaConfig;
		roaConfig = Lookup("RenderNextPage");
		session->RenderNextPage(reply, ctx, roaConfig);
	} else {
		RequestProcessor::Error(reply, "Access denied. Lookuptoken: SLR/NSA", ctx);
	}
}

void WebAppService::PrepareRequest(Context &ctx)
{
	StartTrace(WebAppService.PrepareRequest);
	Anything anyValue, request(ctx.GetEnvStore());
	if ( request.LookupPath(anyValue, "header.COOKIE") ) {
		request["WDCookies"] = anyValue;
	}
}

bool WebAppService::VerifyRequest(std::ostream &, Context &ctx)
{
	if ( ctx.GetRequest().IsNull() ) {
		SystemLog::Info("got no valid request");
		return false;
	}
	if ( ctx.GetEnvStore().IsNull() ) {
		SystemLog::Info("got no valid env from request");
		return false;
	}
	if ( ctx.Lookup("header.REMOTE_ADDR").IsNull() ) {
		SystemLog::Info("request doesn't contain header.REMOTE_ADDR field");
		return false;
	}
	return true;
}

void WebAppService::ExtractPostBodyFields(Anything &query, const Anything &requestBody)
{
	StartTrace(WebAppService.ExtractPostBodyFields);

	Anything reqBody = requestBody;
	long sz = reqBody.GetSize();
	for (long i = 0; i < sz; ++i) {
		String fieldName = reqBody[i]["header"]["CONTENT-DISPOSITION"]["NAME"].AsString();
		if (fieldName != "") {
			if (reqBody[i]["body"].GetSize() == 1) {
				query[fieldName] = reqBody[i]["body"][0L];
			} else {
				long ssz = reqBody[i]["body"].GetSize();
				for (long ii = 0; ii < ssz; ++ii) {
					query[fieldName].Append(reqBody[i]["body"][ii]);
				}
			}
		}
	}
}

void WebAppService::DecodeWDQuery(Anything &query, const Anything &request)
{
	StartTrace(WebAppService.DecodeWDQuery);

	String queryString = ((ROAnything)request)["QUERY_STRING"].AsCharPtr();
	String pathString = ((ROAnything)request)["PATH_INFO"].AsCharPtr();
	Trace("QUERY_STRING =" << queryString);
	Trace("PATH_INFO =" << pathString);

	// analyze the encoded request uri and add it to the query
	Add2Query(query, BuildQuery(pathString, queryString));

	if ( request.IsDefined("REQUEST_BODY")) {
		TraceAny(request["REQUEST_BODY"], "REQUEST_BODY");

		String contentType = request["header"]["CONTENT-TYPE"].AsString();
		Trace("Content type:" << contentType);
		// get the decoded body or the parts of multipart
		if ( contentType.StartsWith("multipart/form-data;") ) {
			// extract multi-part
			ExtractPostBodyFields(query, request["REQUEST_BODY"]);
		} else {
			// extract single part
			Add2Query(query, request["REQUEST_BODY"]);
		}
	}
}

void WebAppService::Add2Query(Anything &query, const Anything &queryItems, bool overwrite)
{
	StartTrace(WebAppService.Add2Query);
	for (long i = 0, sz = queryItems.GetSize(); i < sz; ++i) {
		const char *slotname = queryItems.SlotName(i);
		if ( slotname ) {
			if (overwrite || !query.IsDefined(slotname)) {
				query[slotname] = queryItems[i];
			}
		} else {
			query.Append(queryItems[i]);
		}
	}
}

Anything WebAppService::BuildQuery(const String &pathString, const String &queryString)
{
	StartTrace(WebAppService.BuildQuery);
	Anything query;

	// prepare query anything
	Coast::URLUtils::Split( pathString, '/', query);
	Coast::URLUtils::Split( queryString, '&', query);
	Coast::URLUtils::DecodeAll(query);
	TraceAny(query, "built query");
	return query;
}

void WebAppService::SplitURI2PathAndQuery(Anything &request)
{
	StartTrace(WebAppService.SplitURI2PathAndQuery);
	TraceAny(request, "request");

	String strRequestURI = ((ROAnything)request)["REQUEST_URI"].AsCharPtr(), strPath;
	Trace("REQUEST_URI [" << strRequestURI << "]");

	long lSplitIdx = strRequestURI.StrChr('?');
	if ( lSplitIdx >= 0 ) {
		// non-baseURL request
		strPath = strRequestURI.SubString(0, lSplitIdx);
		request["QUERY_STRING"] = strRequestURI.SubString(lSplitIdx + 1);
	} else {
		// baseURL or non-param request
		strPath = strRequestURI;
	}
	request["PATH_INFO"] = strPath;
	TraceAny(request, "changed request");
}
