/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPFileLoader.h"
#include "StringStream.h"
#include "Renderer.h"
#include "HTTPConstants.h"
RegisterDataAccessImpl(HTTPFileLoader);

bool HTTPFileLoader::GenReplyStatus(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(HTTPFileLoader.GenReplyHeader);

	Anything statusSpec;

	Anything verSpec;
	verSpec[0L]["ContextLookupRenderer"] = String("Mapper.").Append(coast::http::constants::protocolVersionSlotname);
	statusSpec[coast::http::constants::protocolVersionSlotname] = verSpec;

	Anything resCodeSpec;
	resCodeSpec[0L]["ContextLookupRenderer"] = String("Mapper.").Append(coast::http::constants::protocolCodeSlotname);
	statusSpec[coast::http::constants::protocolCodeSlotname] = resCodeSpec;

	Anything resMsgSpec;
	resMsgSpec[0L]["ContextLookupRenderer"] = String("Mapper.").Append(coast::http::constants::protocolMsgSlotname);
	statusSpec[coast::http::constants::protocolMsgSlotname] = resMsgSpec;

	return out->Put("HTTPStatus", statusSpec, context);
}

bool HTTPFileLoader::GenReplyHeader(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(HTTPFileLoader.GenReplyHeader);

	GenReplyStatus(context, in, out);

	Anything contentLengthSpec;
	contentLengthSpec[0L] = "Content-Length: ";
	contentLengthSpec[1L]["ContextLookupRenderer"] = "Mapper.content-length";
	contentLengthSpec[2L] = ENDL;

	Anything condSpec;
	condSpec["ContextCondition"] = "Mapper.content-length";
	condSpec["Defined"] = contentLengthSpec;

	//!@FIXME allow for content-compression...

	Anything headerSpec;
	headerSpec[0L] = "Content-Type: ";
	headerSpec[1L]["ContextLookupRenderer"] = "Mapper.content-type";
	headerSpec[2L] = ENDL;
	headerSpec[3L]["ConditionalRenderer"] = condSpec;
	SubTraceAny(HTTPHeader, headerSpec, "HTTPHeader:");
	return out->Put("HTTPHeader", headerSpec, context);
}

bool HTTPFileLoader::Exec(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(HTTPFileLoader.Exec);
	bool retVal = true;
	String filename;

	context.Push("HTTPFileLoader", this);

	retVal = in->Get("FileName", filename, context);
	SubTrace(FileName, "FileName:<" << filename << ">");

	retVal = GenReplyHeader(context, in, out) && retVal;
	retVal = out->Put(coast::http::constants::protocolVersionSlotname, String("HTTP/1.1"), context) && retVal; // PS Fix binary &

	if (retVal) {
		retVal = ProcessFile(filename, context, in, out);
	}

	if (!retVal) {
		ProduceErrorReply(filename, context, in, out);
	}
	context.Remove("HTTPFileLoader");
	return retVal;
}

void HTTPFileLoader::ProduceErrorReply(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace1(HTTPFileLoader.ProduceErrorReply, "Filename: >" << filename << "<");

	long errorCode(context.Lookup("HTTPError", 400L));
	String errormsg(context.Lookup("HTTPResponse", String("Bad Request")));
	String errorReply;

	errorReply << "<html><head>\n";
	errorReply << "<title>" << errorCode << " " << errormsg << "</title>\n";
	errorReply << "</head><body>\n";
	errorReply << "<h1>" << errormsg << "</h1>\n";

	String taintedRequest(context.Lookup("REQUEST_URI", "/"));
	Anything unTaintRConf;
	unTaintRConf["UnTaintRenderer"]["ToClean"] = taintedRequest;
	errorReply << "<p>The requested URL <b>" << Renderer::RenderToString(context, unTaintRConf) << "</b> is invalid.</p>\n";
	errorReply << "<hr />\n";
	errorReply << "<address>Coast 2.0 Server</address>\n";
	errorReply << "</body></html>\n";

	Trace("errorCode :" << errorCode );
	Trace("errorMsg :" << errormsg );

	out->Put(coast::http::constants::protocolCodeSlotname, errorCode, context);
	out->Put(coast::http::constants::protocolMsgSlotname, errormsg, context);
	out->Put("content-type", String("text/html"), context);
	IStringStream is(errorReply);
	out->Put("HTTPBody", is, context);

	TraceAny(context.GetTmpStore()["Mapper"], "Error handling");
}

bool HTTPFileLoader::ProcessFile(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace1(HTTPFileLoader.ProcessFile, "Filename: >" << filename << "<");

	bool retVal = true;
	std::iostream *Ios = 0;
	String ext;
	Ios = coast::system::OpenStream(filename, ext, std::ios::in | std::ios::binary);
	if (Ios) {
		Trace("Stream opened ok");
		retVal = out->Put(coast::http::constants::protocolCodeSlotname, 200L, context) && retVal;
		retVal = out->Put(coast::http::constants::protocolMsgSlotname, String("Ok"), context) && retVal;

		long posDot = filename.StrRChr('.');
		if (posDot != -1) {
			ext = filename.SubString(posDot + 1, filename.Length());
		}
		String ctquery("Ext2MIMETypeMap");
		ctquery << '.' << ext;
		retVal = out->Put("content-type", String(context.Lookup(ctquery, "text/plain")), context) && retVal;

		ul_long ulFileSize = 0ULL;
		if (coast::system::GetFileSize(filename, ulFileSize)) {
			Trace("file [" << filename << "] has size (stat): " << (l_long)ulFileSize);
			retVal = out->Put("content-length", (long) ulFileSize, context) && retVal;
		}
		retVal = out->Put("HTTPBody", (*(std::istream *) Ios), context) && retVal;
		delete Ios;
	} else {
		retVal = false;
		Anything tmpStore(context.GetTmpStore());
		tmpStore["HTTPError"] = 403L;
		tmpStore["HTTPResponse"] = "Forbidden";
	}
	return retVal;
}
