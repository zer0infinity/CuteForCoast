/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLSocket_H
#define _SSLSocket_H

#include "Socket.h"
#include "SSLAPI.h"

//--- SSLSocketArgs-------------------------------------------------------------------------
//! DataObject to hold arguments needed for SSLSockets.
//! This class supports assignment and copy construction.
class SSLSocketArgs
{
public:
	SSLSocketArgs(bool verifyCertifiedEntity, const String &certVerifyString, bool certVerifyStringIsFilter, bool sessionResumption);

	SSLSocketArgs();
	~SSLSocketArgs();
	SSLSocketArgs &operator=(const SSLSocketArgs &sa);

	// Compare this string against the value stored in CertVerifyEntityName presented in a certificate
	String CertVerifyString();
	// Whether to check the certified entity (subject) or not
	bool VerifyCertifiedEntity();
	// Do resume sessions
	bool CertVerifyStringIsFilter();
	bool SessionResumption();
	String ShowState();
	inline friend std::ostream &operator<< (std::ostream &os, SSLSocketArgs &sa) {
		os << sa.ShowState();
		return os;
	}

private:
	bool fVerifyCertifiedEntity;
	String fCertVerifyString;
	bool fCertVerifyStringIsFilter;
	bool fSessionResumption;
};

//--- SSLSocket ---
class SSLSocket : public Socket
{
	// opens a client side socket connection
	// and closes it in destructor
public:
	SSLSocket(SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose = true,
			  long timeout = 0L /* which is blocking */ , Allocator *a = coast::storage::Global()); // use socket descriptor

	SSLSocket(SSLSocketArgs &sslSocketArgs, SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose = true,
			  long timeout = 0L /* which is blocking */ , Allocator *a = coast::storage::Global()); // use socket descriptor

	~SSLSocket();
	String GetSessionID(SSL *ssl);
	bool CheckPeerCertificate(SSL *ssl, Anything &sslinfo);
	// SSL callbacks you may wish to use.
	static int SSLVerifyCallbackOk(int preverify_ok, X509_STORE_CTX *ctx);
	static int SSLVerifyCallbackFalse(int preverify_ok, X509_STORE_CTX *ctx);
	static int SSLVerifyCallback(int preverify_ok, X509_STORE_CTX *ctx);

	virtual X509 *GetPeerCert()	{
		return fPeerCert;
	}
	//!takes result from PrepareSocket or SSL_xxx calls and checks for errors
	//! NB: res == 1 means everything was OK
	//! \param ssl the ssl structure if available
	//! \param res the return value of the previous SSL_xxx call
	static unsigned long GetSSLError(SSL *ssl = 0, int res = 1);
	//!emit SSL error message on SystemLog and Trace
	//! \param err value returned by GetSSLError
	static void ReportSSLError(unsigned long err);
	//!Store SSL error in supplied Anything. No logging takes place.
	static Anything ReportSSLError(Anything &errAny, unsigned long err);
	//!determine if a non-fatal error occured on a SSL_xxx
	//! tests if the error is SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE
	//! this allows for non-blocking socket io with SSL
	//! \param ssl the ssl structure if available
	//! \param res the return value of the previous SSL_xxx call
	bool ShouldRetry(SSL *ssl, int res, bool handshake = false);

	//!returns amount of bytes read from belonging iostream:
	virtual long GetReadCount() const;
	//!returns amount of bytes wrote from belonging iostream:
	virtual long GetWriteCount() const;

	//! Application queries the overall outcome of the ssl handshake. The returncode
	//! depends on the settings given to SSLModule (used for SSL_CTX creation)
	//! and the per-request parameters (affecting the SSL "object") given in
	//! SSLSocketArgs. If the parameters given result in the net effect of not
	//! checking the certifiate at all, true is returned.
	//! For this reason IsCertCheckPassed may be called even if no cert checking is
	//! needed/required.
	virtual bool IsCertCheckPassed(ROAnything config);

protected:
	virtual std::iostream *DoMakeStream();
	virtual int PrepareSocket(SSL *) = 0;
	virtual void DoCheckPeerCertificate(Anything &sslinfo, SSL *ssl);
	virtual SSL_SESSION *SessionResumptionHookResumeSession(SSL *ssl);
	virtual void SessionResumptionHookSetSession(SSL *ssl, SSL_SESSION *sslSessionStored, bool wasResumed);

	SSL_CTX *fContext;
	X509 *fPeerCert;
	SSLSocketArgs fSSLSocketArgs;

private:
	SSLSocket(); // don't use these
	SSLSocket(const SSLSocket &);
	SSLSocket &operator=(const SSLSocket &);
};

//--- SSLSocket ---
class SSLClientSocket : public SSLSocket
{
	// opens a client side socket connection
	// and closes it in destructor
public:
	SSLClientSocket(SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose = true, long timeOut = 0L, Allocator *a = coast::storage::Global());
	SSLClientSocket(SSLSocketArgs sslSocketArgs, SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose = true, long timeOut = 0L, Allocator *a = coast::storage::Global());

	~SSLClientSocket();

protected:
	virtual int PrepareSocket(SSL *ssl);
	SSL_SESSION *SessionResumptionHookResumeSession(SSL *ssl);
	void SessionResumptionHookSetSession(SSL *ssl, SSL_SESSION *sslSessionStored, bool wasResumed);

private:
	SSLClientSocket(); // don't use these
	SSLClientSocket(const SSLClientSocket &);
	SSLClientSocket &operator=(const SSLClientSocket &);
};

//--- SSLServerSocket ---
class SSLServerSocket : public SSLSocket
{
	// opens a server side socket connection
	// and closes it in destructor
public:
	SSLServerSocket(SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose = true,
					long timeOut = 300 * 1000 /* 5 minutes */, Allocator *a = coast::storage::Global()); // use socket descriptor
	SSLServerSocket(SSLSocketArgs sslSocketArgs, SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose,
					long timeOut = 300 * 1000 /* 5 minutes */, Allocator *a = coast::storage::Global()); // use socket descriptor
	~SSLServerSocket();

protected:
	virtual int PrepareSocket(SSL *ssl);

private:
	SSLServerSocket(); // don't use these
	SSLServerSocket(const SSLServerSocket &);
	SSLServerSocket &operator=(const SSLServerSocket &);
};

//--- SSLConnector --------------------------------------------
class SSLConnector : public Connector
{
	// this class takes an active end point specification
	// of a socket and connects to a server on the other
	// side, creating a socket connection that's read/writeable
public:
	//! If no SSL_CTX  is provided by the caller, a default client SSL_CTX will be created.
	SSLConnector(const char *ipAdr, long port, long connectTimeout = 0, SSL_CTX *ctx = 0, const char *srcIpAdr = 0, long srcPort = 0, bool threadLocal = false);
	//! If no SSL_CTX  is provided by the caller, a default client SSL_CTX will be created.
	SSLConnector(ConnectorArgs &connectorArgs, SSL_CTX *ctx = NULL, const char *srcIpAdr = 0, long srcPort = 0, bool threadLocal = false);
	//! If no SSL_CTX is provided by the caller and the sslModuleCfg  is NULL, a new default client SSL_CTX will be created.
	//! If SSL_CTX  is NULL and sslModuleCfg is not NULL,  a new client SSL_CTX is created considering the sslModuleCfg settings.
	//! In both cases the created SSL_CTX is stored by SSLObjectManager for later reuse.
	//! If you pass in a SSL_CTX (not NULL) this SSL_CTX is used. It will not be deleted.
	//! Use this  method when you need a specialized SSL_CTX which  corresponds  to your SSLSocketArgs
	SSLConnector(ConnectorArgs &connectorArgs, SSLSocketArgs sslSocketArgs, ROAnything sslModuleCfg = ROAnything(), SSL_CTX *ctx = NULL, const char *srcIpAdr = 0, long srcPort = 0, bool threadLocal = false);
	//! pass socket parameters as anything, use /Address, /Port, /Timeout like AcceptorFactory config
	//! Use this constructor if you want SSLObjectManager to store the created SSL_CTX for later reuse
	SSLConnector(ROAnything config);
	//! pass socket parameters as anything, if SSL_CTX is null the created SSL_CTX will be deleted in SSLConnector destuctor.
	//! Use this constructor if you want to keep control over the SSL_CTX
	SSLConnector(ROAnything config, SSL_CTX *, bool deleteCtx = true);

	virtual ~SSLConnector();

protected:

	virtual Socket *MakeSocket(bool doClose = true);
	virtual Socket *DoMakeSocket(int socket, Anything &clientInfo, bool doClose = true);
	virtual Socket *DoMakeSocket(SSLSocketArgs sslSocketArgs, int socket, Anything &clientInfo, bool doClose = true);

	SSL_CTX *fContext;
	SSLSocketArgs fSSLSocketArgs;
	bool fDeleteCtx;

private:
	// don't use these
	SSLConnector();
	SSLConnector(const SSLConnector &);
	SSLConnector &operator=(const SSLConnector &);
};

//--- SSLAcceptor ---
class SSLAcceptor : public Acceptor
{
	// This class handles an accept loop
	// in its own thread
public:
	SSLAcceptor(SSL_CTX *ctx, const char *ipadress, long port, long backlog, AcceptorCallBack *cb, SSLSocketArgs sslSocketArgs);
	~SSLAcceptor();

protected:
	virtual Socket *DoMakeSocket(int socket, Anything &clientInfo, bool doClose = true);
	SSL_CTX *fContext;
	SSLSocketArgs fSSLSocketArgs;

private:
	SSLAcceptor();
	SSLAcceptor(const SSLAcceptor &);
	SSLAcceptor &operator=(const SSLAcceptor &);
};

#endif
