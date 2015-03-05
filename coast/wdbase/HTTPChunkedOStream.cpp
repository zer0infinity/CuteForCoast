/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPChunkedOStream.h"
#include "Tracer.h"
#include <ctype.h>

HTTPChunkedStreamBuf::HTTPChunkedStreamBuf(std::ostream &os, long chunklength, Allocator *alloc)
	: fAllocator(alloc ? alloc : coast::storage::Current())
	, fStore(chunklength, fAllocator)
	, fOs(&os)
	, fBufSize(chunklength)
{
	xinit();
}

HTTPChunkedStreamBuf::~HTTPChunkedStreamBuf()
{
	close();
}

void HTTPChunkedStreamBuf::close()
{
	StartTrace(HTTPChunkedStreamBuf.close);
	if (!fOs) {
		return;
	}
	sync();
	(*fOs) << '0' << ENDL << ENDL;
	fOs->flush();
	fOs = 0;
}

int HTTPChunkedStreamBuf::sync()
{
	StartTrace(HTTPChunkedStreamBuf.sync);

	if (!fOs) {
		return EOF;
	}
	long len = pptr() - pbase();
	if (len > 0) {
#if defined(WIN32)
		long old = fOs->flags();
#else
		std::ios::fmtflags old = fOs->flags();
#endif
		(*fOs) << std::hex << len << ENDL;
		fOs->flags(old);
		fOs->write((const char *)fStore, len);
		(*fOs) << ENDL ;
		pinit();
	}
	return 0;
}

HTTPChunkedStreamBuf::pos_type HTTPChunkedStreamBuf::seekpos(pos_type, openmode mode)
{
	return pos_type(0);
}

HTTPChunkedStreamBuf::pos_type HTTPChunkedStreamBuf::seekoff(off_type, seekdir, openmode mode)
{
	return pos_type(0);
}

int HTTPChunkedStreamBuf::overflow(int c)
{
	sync();
	if (c != EOF && (pptr() < epptr())) { // guard against recursion
		sputc(c);
	}
	return 0L;  // return 0 if successful
}

int HTTPChunkedStreamBuf::underflow()
{
	return 0;
}

void HTTPChunkedStreamBuf::xinit()
{
	setg(0, 0, 0);
	pinit();
}

void HTTPChunkedStreamBuf::pinit()
{
	char *sc = (char *)(const char *)fStore;
	char *endptr = sc + fBufSize;
	setp(sc, endptr);
}
