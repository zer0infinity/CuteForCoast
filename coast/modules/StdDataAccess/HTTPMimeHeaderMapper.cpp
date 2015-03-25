/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPMimeHeaderMapper.h"
#include "MIMEHeader.h"
#include "RE.h"
#include "AnyIterators.h"
#include "AnythingUtils.h"
//---- HTTPMimeHeaderMapper ------------------------------------------------------------------
RegisterResultMapper(HTTPMimeHeaderMapper);

bool HTTPMimeHeaderMapper::DoPutStream(const char *, std::istream &is, Context &ctx, ROAnything config) {
	// Ignore key, store under Mapper.HTTPHeader by default
	StartTrace(HTTPMimeHeaderMapper.DoPutStream);
	TraceAny(config, "My Config");
	MIMEHeader mh(coast::urlutils::eDownshift);
	bool result = false;
	try {
		result = mh.ParseHeaders(is);
	} catch (MIMEHeader::StreamNotGoodException &e) {
		;
	}

	if (result && is.good()) {
		Anything header(mh.GetHeaderInfo());
		TraceAny(header, "header");
		//!@FIXME: all of the following should go to separate mappers
		if (config["StoreCookies"].AsLong(0)) {
			StoreCookies(header, ctx);
		}
		if (config.IsDefined("Suppress")) {
			ROAnything suppresslist(config["Suppress"]);
			SuppressHeaders(header, suppresslist);
		}
		if (config.IsDefined("Add")) {
			ROAnything addlist(config["Add"]);
			AddHeaders(header, addlist);
		}
		if (config.IsDefined("Substitute")) {
			ROAnything addlist(config["Substitute"]);
			Substitute(header, addlist, ctx);
		}
		//!@FIXME: here we should use Put()/DoPutAny() to allow further processing of header data before storing
		result = DoFinalPutAny("HTTPHeader", header, ctx);
	}
	return result && is.good();
}

void HTTPMimeHeaderMapper::SuppressHeaders(Anything &header, ROAnything &suppresslist) {
	for (long i = 0, sz = suppresslist.GetSize(); i < sz; ++i) {
		String keytosuppress = suppresslist[i].AsCharPtr();
		keytosuppress.ToLower();
		if (header.IsDefined(keytosuppress)) {
			header.Remove(keytosuppress);
		}
	}
}

void HTTPMimeHeaderMapper::AddHeaders(Anything &header, ROAnything &addlist) {
	for (long i = 0, sz = addlist.GetSize(); i < sz; ++i) {
		String key(addlist.SlotName(i));
		String hdr(addlist[i].AsString());
		// normalize key to lower-type (header info stays as is)
		key.ToLower();
		// overwrite, if existing
		header[key] = hdr;
	}
}

void HTTPMimeHeaderMapper::Substitute(Anything &header, ROAnything &addlist, Context &ctx) {
	StartTrace(HTTPMimeHeaderMapper.Substitute);

	TraceAny(ctx.GetQuery(), "query:");

	String key;
	String pattern;
	String replacement;
	String slotName = addlist.SlotName(0);
	if (slotName.Length() > 0) {
		key = addlist["Key"].AsString();
		pattern = addlist["Pattern"].AsString();
		replacement = addlist["Replacement"].AsString();
		RE substRegex(pattern, RE::MATCH_ICASE);
		for (int i = 0; i < header.GetSize(); i++) {
			if (header.SlotName(i) == key) {
				header[i] = substRegex.Subst(header[i].AsString(), replacement);
			}
		}
	} else {
		for (int i = 0; i < addlist.GetSize(); i++) {
			key = addlist[i]["Key"].AsString();
			pattern = addlist[i]["Pattern"].AsString();
			replacement = addlist[i]["Replacement"].AsString();

			RE substRegex(pattern, RE::MATCH_ICASE);
			for (int j = 0; j < header.GetSize(); j++) {
				if (header.SlotName(j) == key) {
					header[j] = substRegex.Subst(header[j].AsString(), replacement);
				}
			}
		}
	}
}

void HTTPMimeHeaderMapper::StoreCookies(ROAnything const header, Context &ctx) {
	StartTrace(HTTPMimeHeaderMapper.StoreCookies);

	const String valueSlotName("_value_");
	const String attrSlotName("_attrs_");
	const String cookieID("set-cookie");
	static const char cookieEnd = ';';
	static const char cookie_path_sep = '=';
	String destSlotname("StoredCookies");
	const String backendName(ctx.Lookup("Backend.Name", ""));
	destSlotname.Append(cookie_path_sep).Append(backendName);

	String strKeyValue(64L), strCookie(128L), strKey(32L), strValue(64L);
	ROAnything roaCookie;
	AnyExtensions::Iterator<ROAnything> cookieIterator(header[cookieID]);
	while (cookieIterator.Next(roaCookie)) {
		TraceAny(roaCookie, "current cookie entry");
		strCookie = roaCookie.AsString();
		String cookieName(32L);
		StringTokenizer semiTokenizer(strCookie, cookieEnd);
		Anything anyNamedCookie;
		while (semiTokenizer.NextToken(strKeyValue)) {
			StringTokenizer valueTokenizer(strKeyValue, '=');
			if (valueTokenizer.NextToken(strKey)) {
				strKey.TrimWhitespace();
				strValue = valueTokenizer.GetRemainder();
				if (!cookieName.Length()) {
					cookieName = strKey;
					if (strValue.Length()) {
						anyNamedCookie[valueSlotName] = strValue;
					}
				} else {
					if (strValue.Length()) {
						anyNamedCookie[attrSlotName][strKey] = strValue;
					} else {
						anyNamedCookie[attrSlotName].Append(strKey);
					}
				}
			}
		}
		if (anyNamedCookie.GetSize() > 0 && cookieName.Length()) {
			String strStructured(destSlotname);
			strStructured.Append(cookie_path_sep).Append("Structured").Append(cookie_path_sep).Append(cookieName);
			TraceAny(anyNamedCookie, "storing named cookie at [" << strStructured << "]");
			StorePutter::Operate(anyNamedCookie, ctx, "Session", strStructured, true, cookie_path_sep);
		}
	}
	//!@FIXME: this part should be factored out into separate mapper
	ROAnything anyCookies = ctx.Lookup(String(destSlotname).Append(cookie_path_sep).Append("Structured"), cookie_path_sep);
	if (anyCookies.GetSize() > 0) {
		TraceAny(anyCookies, "prepared cookie values");
		String plainCookieString(256L), cookieName(32L);
		AnyExtensions::Iterator<ROAnything> structureIter(anyCookies);
		ROAnything roaEntry;
		while (structureIter.Next(roaEntry)) {
			if (!plainCookieString.Length()) {
				plainCookieString.Append("Cookie:");
			}
			structureIter.SlotName(cookieName);
			//!@FIXME: in future we should decide on attributes to select correct value entry to use, currently we use the last one (roaEntry.GetSize()-1) if multiple entries exist
			plainCookieString.Append(' ').Append(cookieName).Append('=').Append(roaEntry[roaEntry.GetSize() - 1][valueSlotName].AsString()).Append(
					cookieEnd);
		}
		Anything anyPlainString(plainCookieString);
		Trace("plain cookie string [" << plainCookieString << "]");
		StorePutter::Operate(anyPlainString, ctx, "Session", String(destSlotname).Append(cookie_path_sep).Append("Plain"), false,
				cookie_path_sep);
	}
	TraceAny(ctx.Lookup("StoredCookies"), "cookies from context");
}
