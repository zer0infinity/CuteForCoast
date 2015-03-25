/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringTokenizeRenderer.h"
#include "StringStream.h"

RegisterRenderer(StringTokenizeRenderer);

void StringTokenizeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(StringTokenizeRenderer.RenderAll);
	TraceAny(config, "config");

	Anything anyTokens, anyOutputTokenList;
	if ( SplitStringIntoTokens(ctx, config, anyTokens) ) {
		BuildTokenList(ctx, config, anyTokens, anyOutputTokenList);

		String strOut;
		BuildOutputString(ctx, config, strOut, anyOutputTokenList, anyTokens);

		Trace("string to return [" << strOut << "]");
		reply << strOut;
	}
}

void StringTokenizeRenderer::BuildTokenList(Context &ctx, const ROAnything &config, Anything &anyTokens, Anything &anyOutputTokenList)
{
	StartTrace(StringTokenizeRenderer.BuildTokenList);

	ROAnything roaRenderToken;
	String strRenderToken;
	if (config.LookupPath(roaRenderToken, "RenderToken")) {
		RenderOnString(strRenderToken, ctx, roaRenderToken);
	}

	Trace("RenderToken: [" << strRenderToken << "]");
	long lMaxTokens = anyTokens.GetSize() - 1L;

	StringTokenizer tokens(strRenderToken, ';');
	String strTok;
	while ( tokens.NextToken(strTok) ) {
		IStringStream stream(strTok);
		long lStart = 0L, lEnd = 0L, lDiff = 0L;
		char c = '\0';
		stream >> lStart;
		if ( stream.good() ) {
			if ( stream.get(c) && c == '-' ) {
				stream >> lEnd;
				if ( lEnd == 0L ) {
					lEnd = (lStart >= 0L) ? lMaxTokens : -lMaxTokens;
					Trace("setting second number from tokensize: " << lEnd);
				} else if ( lEnd > lMaxTokens ) {
					lEnd = lMaxTokens;
					Trace("setting second number from tokensize: " << lEnd);
				} else if ( lEnd < -lMaxTokens ) {
					lEnd = -lMaxTokens;
					Trace("setting second number from tokensize: " << lEnd);
				}
				lStart = (lStart < 0 ? (lMaxTokens+lStart+1) : lStart);
				lEnd = (lEnd < 0 ? (lMaxTokens+lEnd+1) : lEnd);
				lDiff = abs(lEnd - lStart);
			}
			Trace("char:" << c);
		}
		long lIncr = ( ( lStart <= lEnd ) ? 1L : -1L );
		Trace("start: " << lStart << " end: " << lEnd << " diff: " << lDiff);
		Trace("incr: " << lIncr);
		while ( lDiff >= 0L ) {
			anyOutputTokenList.Append(lStart);
			lStart += lIncr;
			--lDiff;
		}
	}

	if (anyOutputTokenList.GetSize() == 0L) {
		// append the first token as default to use
		anyOutputTokenList.Append("1");
	}
	TraceAny(anyOutputTokenList, "Tokens to render");
}

bool StringTokenizeRenderer::SplitStringIntoTokens(Context &ctx, const ROAnything &config, Anything &anyTokens)
{
	StartTrace(StringTokenizeRenderer.SplitStringIntoTokens);

	String value, token;
	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "String")) {
		RenderOnString(value, ctx, roaSlotConfig);
	} else {
		SYSERROR("String not defined");
		return false;
	}
	Trace("String: [" << value << "]");

	if (config.LookupPath(roaSlotConfig, "Token")) {
		RenderOnString(token, ctx, roaSlotConfig);
	} else {
		SYSERROR("Token not defined");
		return false;
	}
	Trace("Token: [" << token << "]");

	// set index 0 to empty string
	anyTokens.Append("");

	if (value.Contains(token) >= 0L) {
		if (token.Length() == 1L) {
			Trace("got char token [" << token << "]");
			StringTokenizer tokens(value, token[0L]);
			String strTok;
			while ( tokens.NextToken(strTok) ) {
				Trace("current segment is [" << strTok << "]");
				anyTokens.Append(strTok);
			}
		} else {
			Trace("got string token [" << token << "]");

			String strTmp = value, strTok;
			long lIdx = strTmp.Contains(token);
			while (lIdx >= 0L) {
				strTok = strTmp.SubString(0, lIdx);
				Trace("current segment is [" << strTok << "]");
				anyTokens.Append(strTok);
				strTmp.TrimFront(lIdx + token.Length());
				lIdx = strTmp.Contains(token);
				if (lIdx < 0L) {
					// add remaining part as last token
					Trace("last segment is [" << strTmp << "]");
					anyTokens.Append(strTmp);
				}
			}
		}
	}
	TraceAny(anyTokens, "" << anyTokens.GetSize() - 1L << " Tokens");
	return true;
}

void StringTokenizeRenderer::BuildOutputString(Context &ctx, const ROAnything &config, String &strOut, Anything &anyTokensToOutput, Anything &anyTokens)
{
	StartTrace(StringTokenizeRenderer.BuildOutputString);

	String filler;
	ROAnything roaSlotConfig;
	bool bFoundToken = false;

	if (config.LookupPath(roaSlotConfig, "OutputFiller")) {
		RenderOnString(filler, ctx, roaSlotConfig);
	}

	String strDefault;
	if (config.LookupPath(roaSlotConfig, "Default")) {
		RenderOnString(strDefault, ctx, roaSlotConfig);
	}

	// create string to return
	Trace("Number of Tokens:" << anyTokensToOutput.GetSize());
	for (long lIdx = 0L, sz = anyTokensToOutput.GetSize(); lIdx < sz; ++lIdx) {
		long lTokIdx = anyTokensToOutput[lIdx].AsLong(0L);
		Trace("abs of lTokIdx: " << (long)abs(lTokIdx));
		if ( abs(lTokIdx) < anyTokens.GetSize() ) {
			bFoundToken = true;
			if ( lIdx > 0L && filler.Length() ) {
				Trace("inserting output filler [" << filler << "]");
				strOut << filler;
			}
			// handle negative values
			Trace("index of list:" << lTokIdx);
			if (lTokIdx < 0L) {
				lTokIdx = anyTokens.GetSize() + lTokIdx;
			}
			Trace("accessing index:" << lTokIdx);
			strOut << anyTokens[lTokIdx].AsString();
		}
	}
	if ( !bFoundToken ) {
		// no token output, use either the value from Default slot or return the empty string
		strOut = strDefault;
	}
}
