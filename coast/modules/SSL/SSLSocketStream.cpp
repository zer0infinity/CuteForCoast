/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLSocketStream.h"
#include "SSLAPI.h"
#include "SSLSocket.h"
#include "SSLObjectManager.h"
#include "SystemLog.h"
#include "Tracer.h"
//#define STREAM_TRACE

#undef Free

iosITOSSLSocket::iosITOSSLSocket(SSL *ctx, SSLSocket *ssl, long timeout)
	: 	fSSLSockBuf(ctx, ssl, timeout),
		fAllocator(ssl->GetAllocator())
{
	init(&fSSLSockBuf);
}

SSLSocketStreamBuf::SSLSocketStreamBuf(SSL *ctx, SSLSocket *ssl, long timeout) : SocketStreamBuf(ssl, timeout), fContext(ctx)
{
}

SSLSocketStreamBuf::~SSLSocketStreamBuf()
{
	StartTrace(SSLSocketStreamBuf.~SSLSocketStreamBuf);
	// sync before releasing ssl context
	sync(); // clear the buffer
	// don't use virtual derivations
//	setb(0, 0, 0);          	 	// delete the buffer, if required
	setp(0, 0);
	setg(0, 0, 0);

	if ( fContext) {
		Trace("SSLCtx SSL_get_quiet_shutdown(): " << SSL_get_quiet_shutdown(fContext));
		TraceAny(SSLObjectManager::TraceSSLSession(SSL_get_session(fContext)), "sslSessionCurrent");
		SSL_shutdown(fContext);
		SSL_free (fContext);
	}
	ERR_clear_error();
	fContext = 0;
}

long SSLSocketStreamBuf::DoWrite(const char *buf, long len)
{
	StartTrace(SSLSocketStreamBuf.DoWrite);

	long bytesSent = 0;

	// SOP: should we try to send everything as with Socket::DoWrite()?
	do {
		bytesSent = SSL_write(fContext, (char *)buf, len);
		if ( bytesSent > 0 ) {
			AddWriteCount( bytesSent );
		}
	} while (bytesSent < 1 && len > 0
			 && ((SSLSocket *)fSocket)->ShouldRetry(fContext, bytesSent, false));
#ifdef STREAM_TRACE
	if ( bytesSent > 0 ) {
		SystemLog::WriteToStderr(buf, bytesSent);
	}
#endif
	SetStreamState(bytesSent);
	return bytesSent;
}

long SSLSocketStreamBuf::DoRead(char *buf, long len) const
{
	StartTrace(SSLSocketStreamBuf.DoRead);

	long bytesRead = 0;
	do {
		bytesRead = SSL_read(fContext, buf, len);
		//if ( bytesRead > 0 )
		//{
		//	AddReadCount( bytesRead );
		//}
		// -> moved this to SocketStreamBuf
	} while (bytesRead < 1 && len > 0
			 && ((SSLSocket *)fSocket)->ShouldRetry(fContext, bytesRead, false));
#ifdef STREAM_TRACE
	if ( bytesRead > 0 ) {
		SystemLog::WriteToStderr(buf, bytesRead);
	}
#endif
	SetStreamState(bytesRead);
	return bytesRead;
}

void SSLSocketStreamBuf::SetStreamState(long bytesProcessed) const
{
	if (bytesProcessed < 1) { //!@FIXME refactor in a method
		// tell the stream that something is really wrong.
		std::iostream *Ios = fSocket->GetStream();
		Ios->clear(std::ios::badbit);
	}
}
