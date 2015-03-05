/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LDAPConnection.h"
#include "LDAPMessageEntry.h"
#include "Tracer.h"
#if defined(USE_OPENLDAP)
static const long fgOptNetworkTimeout = LDAP_OPT_NETWORK_TIMEOUT;
static const String fgstrOptNetworkTimeout = String("LDAP_OPT_NETWORK_TIMEOUT", -1, coast::storage::Global());
static unsigned long ldap_utf8getcc( const char **src )
{
	char UTF8len[64]
	= {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 5, 6
	  };

	register unsigned long c;
	register const unsigned char *s = (const unsigned char *) * src;
	switch (UTF8len [(*s >> 2) & 0x3F]) {
		case 0: /* erroneous: s points to the middle of a character. */
			c = (*s++) & 0x3F;
			goto more5;
		case 1:
			c = (*s++);
			break;
		case 2:
			c = (*s++) & 0x1F;
			goto more1;
		case 3:
			c = (*s++) & 0x0F;
			goto more2;
		case 4:
			c = (*s++) & 0x07;
			goto more3;
		case 5:
			c = (*s++) & 0x03;
			goto more4;
		case 6:
			c = (*s++) & 0x01;
			goto more5;
		more5:
			if ((*s & 0xC0) != 0x80) {
				break;
			}
			c = (c << 6) | ((*s++) & 0x3F);
		more4:
			if ((*s & 0xC0) != 0x80) {
				break;
			}
			c = (c << 6) | ((*s++) & 0x3F);
		more3:
			if ((*s & 0xC0) != 0x80) {
				break;
			}
			c = (c << 6) | ((*s++) & 0x3F);
		more2:
			if ((*s & 0xC0) != 0x80) {
				break;
			}
			c = (c << 6) | ((*s++) & 0x3F);
		more1:
			if ((*s & 0xC0) != 0x80) {
				break;
			}
			c = (c << 6) | ((*s++) & 0x3F);
			break;
	}
	*src = (const char *)s;
	return c;
}
#else
static const long fgOptNetworkTimeout = LDAP_X_OPT_CONNECT_TIMEOUT;
static const String fgstrOptNetworkTimeout = String("LDAP_X_OPT_CONNECT_TIMEOUT", -1, coast::storage::Global());
#endif

LDAPConnection::LDAPConnection(ROAnything connectionParams)
{
	StartTrace(LDAPConnection.LDAPConnection);
	fServer = connectionParams["Server"].AsString("localhost");
	fPort = connectionParams["Port"].AsLong(389L);
	fSearchTimeout  = (int) connectionParams["Timeout"].AsLong(60L);
	// A reasonable connection timeout default
	fConnectionTimeout = connectionParams["ConnectionTimeout"].AsLong(10);
	fConnectionTimeout *= 1000;
	TraceAny(connectionParams, "ConnectionParams");
	fHandle = (LDAP *) NULL;
}

LDAPConnection::~LDAPConnection()
{
	StartTrace(LDAPConnection.~LDAPConnection);
}

LDAP *LDAPConnection::GetConnection()
{
	StartTrace(LDAPConnection.GetConnection);
	return fHandle;
}

int LDAPConnection::GetSearchTimeout()
{
	StartTrace(LDAPConnection.GetSearchTimeout);
	return fSearchTimeout;
}

int LDAPConnection::GetConnectionTimeout()
{
	StartTrace(LDAPConnection.GetConnectionTimeout);
	return fConnectionTimeout;
}

bool LDAPConnection::Connect(ROAnything bindParams, LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.Connect);
	return IsConnectOk(DoConnect(bindParams, eh));
}

bool LDAPConnection::IsConnectOk(LDAPConnection::EConnectState eConnectState)
{
	StartTrace(LDAPConnection.IsConnectOk);
	switch (eConnectState) {
		case eOk:
		case eRebindOk:
			return true;
			break;
		default:
			break;
	}
	return false;
}

String LDAPConnection::ConnectRetToString(LDAPConnection::EConnectState eConnectState)
{
	StartTrace(LDAPConnection.ConnectRetToString);
	Anything conv;
	conv[eOk]				= "eOk";
	conv[eNok]				= "eNok";
	conv[eMustRebind]		= "eMustRebind";
	conv[eMustNotRebind]	= "eMustNotRebind";
	conv[eRebindOk]			= "eRebindOk";
	conv[eInitNok] 			= "eInitNok";
	conv[eBindNok] 			= "eBindNok";
	conv[eSetOptionsNok] 	= "eSetOptionsNok";
	return conv[eConnectState].AsString();
}

bool LDAPConnection::ReleaseHandleInfo()
{
	StartTrace(LDAPConnection.ReleaseHandleInfo);
	return DoReleaseHandleInfo();
}

bool LDAPConnection::DoReleaseHandleInfo()
{
	StartTrace(LDAPConnection.DoReleaseHandleInfo);
	Disconnect();
	return true;
}

LDAPConnection::EConnectState LDAPConnection::DoConnect(ROAnything bindParams, LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.DoConnect);

	// set handle to null
	fHandle = NULL;
	String errMsg;
	String bindName = bindParams["BindName"].AsString("");
	String bindPW = bindParams["BindPW"].AsString("");
	EConnectState aRetState = LDAPConnection::eInitNok;
	// get connection handle
	if ( (fHandle = Init(eh)) != NULL ) {
		aRetState = LDAPConnection::eSetOptionsNok;
		// set protocol, timeout and rebind procedure
		if ( SetProtocol(eh) && SetConnectionTimeout(eh) && SetSearchTimeout(eh) /* && SetRebindProc(eh) */  ) {
			aRetState = LDAPConnection::eBindNok;
			// send bind request (asynchronous)
			int msgId;
			if ( Bind(bindName, bindPW, msgId, eh) ) {
				// wait for bind result (using msgId)
				Anything result;
				if ( WaitForResult(msgId, result, eh) ) {
					aRetState = LDAPConnection::eOk;
				}
			}
		}
	}
	return aRetState;
}

LDAP *LDAPConnection::Init(LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.Init);

	LDAP *locLdap = ::ldap_init( fServer, fPort );
	if ( !locLdap ) {
		Trace("ldap_init FAILED");
		String errMsg = "ldap_init(";
		errMsg << fServer << "," << fPort << ") failed";
		eh.HandleSessionError(fHandle, errMsg);
	}
	return locLdap;
}

bool LDAPConnection::SetProtocol(LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.SetProtocol);

	int ldapVersion = LDAP_VERSION_MAX;
	if ( ::ldap_set_option( fHandle, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion ) != LDAP_SUCCESS ) {
		Trace("ldap_set_option: LDAP_OPT_PROTOCOL_VERSION   FAILED");
		eh.HandleSessionError(fHandle, "Could not set protocol version.");
		return false;
	}
	return true;
}

bool LDAPConnection::SetSearchTimeout(LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.SetSearchTimeout);

	// This will be overridden in DoLDAPRequest - valid only for searches
	if ( ::ldap_set_option( fHandle, LDAP_OPT_TIMELIMIT, &fSearchTimeout ) != LDAP_SUCCESS ) {
		Trace("ldap_set_option: LDAP_OPT_TIMELIMIT   FAILED");
		eh.HandleSessionError(fHandle, "Could not set timelimit for searches.");
		return false;
	}
	return true;
}

bool LDAPConnection::SetConnectionTimeout(LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.SetConnectionTimeout);

	if ( ::ldap_set_option( fHandle, fgOptNetworkTimeout, &fConnectionTimeout ) != LDAP_SUCCESS ) {
		Trace("==================================== ::ldap_set_option: " << fgstrOptNetworkTimeout << "   FAILED");
		eh.HandleSessionError(fHandle, String("ldap_set_option  [") << fgstrOptNetworkTimeout << "]");
		return false;
	}
	return true;
}

bool LDAPConnection::Bind(String bindName, String bindPW, int &msgId, LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.Bind);

	String errMsg = "Binding request failed. ";
	errMsg << "[Server: '" << fServer << "', Port: '" << fPort << "'] ";

	if ( bindName.IsEqual("") || bindName.IsEqual("Anonymous") ) {
		Trace("Binding as Anonymous");
		errMsg << " (Anonymous bind.)";
		msgId = ::ldap_simple_bind( fHandle, NULL, NULL );
	} else {
		errMsg << " (Authorized bind.)";

		if ( bindPW == "undefined" ) {
			Trace("Bindpassword NOT OK: pwd = ["  << bindPW << "]");
			eh.HandleSessionError(fHandle, errMsg);
			return false;
		} else {
			Trace("Binding with <" << bindName << "><" << bindPW << ">");
			msgId = ::ldap_simple_bind( fHandle, bindName, bindPW );
		}
	}

	// bind request successful?
	if ( msgId == -1 ) {
		DoHandleBindFailure(eh, errMsg);
		return false;
	}

	Trace("Binding request SUCCEEDED, waiting for connection...");
	return true;
}

void LDAPConnection::DoHandleBindFailure(LDAPErrorHandler &eh, String &errMsg)
{
	StartTrace1(LDAPConnection.DoHandleBindFailure, "Binding request FAILED!");
	eh.HandleSessionError(fHandle, errMsg);
}

bool LDAPConnection::WaitForResult(int msgId, Anything &result, LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.WaitForResult);
	Trace("msgId: [" << msgId << "]");
	timeval tv;
	tv.tv_sec  = fSearchTimeout;
	tv.tv_usec = 0;

	timeval *tvp = (fSearchTimeout == 0) ? NULL : &tv;

	bool finished = false;
	bool success = false;

	String errMsg;
	int resultCode;

	LDAPMessage *ldapResult;
	LDAPMessageEntry lmAutoDestruct(&ldapResult);	// automatic destructor for LDAPMessage
	lmAutoDestruct.Use();

	while (!finished) {
		// wait for result
		resultCode = ldap_result(fHandle, msgId, 1, tvp, &ldapResult);

		// check result
		if (resultCode == -1 && fSearchTimeout == 0) {
			// error, abandon!
			Trace("[Timeout: 0] received an error");
			int opRet;
			ldap_parse_result( fHandle, ldapResult, &opRet, NULL, NULL, NULL, NULL, 0 );
			errMsg << "Synchronous Wait4Result: ErrorCode: [" << (long)opRet << "] ErrorMsg: " << ldap_err2string( opRet );
			HandleWait4ResultError(msgId, errMsg, eh);
			finished = true;
		} else if (resultCode == 0 || (resultCode == -1 && fSearchTimeout != 0)) {
			// resultCode 0 means timeout, abandon
			Trace("[Timeout != 0] encountered a timeout ...");
			errMsg << "Asynchronous Wait4Result: The request <" << (long) msgId << "> timed out.";
			HandleWait4ResultError(msgId, errMsg, eh);
			DoHandleWaitForResultTimeout(eh);
			finished = true;
		} else {
			// received a result
			int errCode = ldap_result2error(fHandle, ldapResult, 0);
			if (errCode == LDAP_SUCCESS || errCode == LDAP_SIZELIMIT_EXCEEDED) {
				Trace("recieved a result and considers it to be ok ...");
				success = true;

				// transform LDAPResult into an Anything with Meta Information
				TransformResult(ldapResult, result, eh);

				// add extra flag to inform client, if sizelimit was exceeded
				if (errCode == LDAP_SIZELIMIT_EXCEEDED) {
					result["SizeLimitExceeded"] = 1;
				}
			} else if (errCode == LDAP_COMPARE_FALSE || errCode == LDAP_COMPARE_TRUE) {
				Trace("recieved a result and considers it to be ok (compare) ...");
				success = true;

				// this is a bit special
				int rc;
				ldap_get_option(fHandle, LDAP_OPT_ERROR_NUMBER, &rc);
				result["Type"] = "LDAP_RES_COMPARE";
				result["Equal"] = (errCode == LDAP_COMPARE_TRUE);
				result["LdapCode"] = rc;
				result["LdapMsg"] = ldap_err2string(rc);
			} else {
				Trace("recieved a result and considers it to be WRONG ...");
				errMsg = "LDAP request failed.";
				eh.HandleSessionError(fHandle, errMsg);
			}
			finished = true;
		}
	}
	return success;
}

void LDAPConnection::HandleWait4ResultError(int msgId, String &errMsg, LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.HandleWait4ResultError);
	if ( msgId != -1 ) {
		int errCode = ldap_abandon(fHandle, msgId);
		errMsg << " Request abandoned: " << (errCode == LDAP_SUCCESS ? " successfully." : " FAILED!");
	} else {
		errMsg << " Request abandoned: binding handle was invalid, ldap_abandon not called";
	}
	eh.HandleSessionError(fHandle, errMsg);
	DoHandleWait4ResultError(eh);
}

void LDAPConnection::TransformResult(LDAPMessage *ldapResult, Anything &result, LDAPErrorHandler &eh)
{
	StartTrace(LDAPConnection.TransformResult);
	TraceAny(eh.GetQueryParams(), "QueryParams");
	TraceAny(eh.GetConnectionParams(), "ConnectionParams");

	String type = GetTypeStr(ldap_msgtype(ldapResult));
	Trace("Type of result: " << type);

	if (type.Length() > 0) {
		// meta information about the result
		result["Type"] = type;
		result["Query"] = eh.GetQueryParams().DeepClone();
		int nofResults = ldap_count_messages(fHandle, ldapResult);
		result["NumberOfResultMessages"] = nofResults;
		int nofEntries = ldap_count_entries(fHandle, ldapResult);
		result["NumberOfEntries"] = nofEntries;
		result["SizeLimitExceeded"] = 0;

		Trace("#Messages: " << (long)nofResults << ", #Entries: " << (long)nofEntries);

		// extract data
		LDAPMessage *entry = ldap_first_entry(fHandle, ldapResult);
		String valStr, attrString, dn;
		struct berval **vals;
		BerElement *ber;
		int count = 0;
		int nofVals;
		Anything entries;

		// step through all entries
		while (entry) {
			dn.Trim(0L);
			char *ptrToDn = ldap_get_dn(fHandle, entry);
			if ( ptrToDn != ( char * ) NULL ) {
				dn = ptrToDn;
				ldap_memfree( ptrToDn );
			}
			char *attr = ldap_first_attribute(fHandle, entry, &ber);
			//
			// kgu: might want to add dn ass attribute to each
			// entry, some clients of old system might rely on it
			//
			// entries[dn]["dn"] = dn;

			Trace("processing entries of dn [" << dn << "]");
			// step through all attributes
			while ( attr != (char *) NULL ) {
				attrString = attr;
				// normalize all attributes to lowercase
				attrString.ToLower();
				Trace("getting length of attribute [" << attrString << "]");
				vals = ldap_get_values_len(fHandle, entry, attr);
				nofVals = ldap_count_values_len(vals);

				if ( nofVals == 0 ) {
					// there might be attributes without values
					// (eg. if AttrsOnly was requested)
					entries[dn].Append(attrString);
				} else {
					for (int i = 0; i < nofVals; ++i) {
						valStr = String((void *)vals[i]->bv_val, (long)vals[i]->bv_len);

						if ( !eh.GetConnectionParams()["PlainBinaryValues"].AsBool(false) ) {
							MapUTF8Chars(valStr, eh.GetConnectionParams()["MapUTF8"].AsBool(true));
						}
						if (i == 0) {
							entries[dn][attrString] = valStr;
						} else {
							entries[dn][attrString].Append(valStr);
						}
					}
				}
				// free used memory
				if ( vals != ( berval ** ) NULL ) {
					ldap_value_free_len(vals);
				}
				if ( attr != ( char * ) NULL ) {
					ldap_memfree(attr);
				}
				attr = ldap_next_attribute(fHandle, entry, ber);
			}
			entry = ldap_next_entry(fHandle, entry);
			++count;
			// free used memory
			if (ber) {
				ber_free(ber, 0);
			}
		}
		result["Entries"] = entries;
	}
}

bool LDAPConnection::Disconnect()
{
	StartTrace(LDAPConnection.Disconnect);

	Trace("Disconnecting LDAP...");
	return Disconnect(fHandle);
}

bool LDAPConnection::Disconnect(LDAP *handle)
{
	StartTrace(LDAPConnection.Disconnect);

	Trace("Disconnecting LDAP...");
	if (handle) {
		int errCode = ldap_unbind(handle);
		if (errCode != LDAP_SUCCESS) {
			LDAPErrorHandler::HandleUnbindError(handle);
			Trace("Disconnect (unbind) failed!");
			return false;
		}
	}
	Trace("Disconnecting successful.");
	return true;
}

void LDAPConnection::MapUTF8Chars(String &str, bool bMapUTF8)
{
	StartTrace(LDAPConnection.MapUTF8Chars);
	Trace("Do UTF8 to HTML mapping: " << (bMapUTF8 ? "yes" : "no"));

	String result;
	for (long i = 0; i < str.Length(); ) {
		const char *theChar = ((const char *)str) + i;
		const char *theStartChar = theChar;
		unsigned long utfChar = ldap_utf8getcc(&theChar);
		i += (theChar - theStartChar); // one utf8 symbol might be several characters

		if ( bMapUTF8 ) {
			switch (utfChar) {
				case 252:
					result.Append("&uuml;");
					break;
				case 228:
					result.Append("&auml;");
					break;
				case 246:
					result.Append("&ouml;");
					break;
				case 179:
					result.Append("&Uuml;");
					break;
				case 196:
					result.Append("&Auml;");
					break;
				case 214:
					result.Append("&Ouml;");
					break;
				case 234:
					result.Append("&ecirc;");
					break;
				case 226:
					result.Append("&acirc;");
					break;
				case 224:
					result.Append("&agrave;");
					break;
				case 232:
					result.Append("&egrave;");
					break;
				case 242 :
					result.Append("&Egrave;");
					break;
				case 250 :
					result.Append("&Euml;");
					break;
				case 233:
					result.Append("&eacute;");
					break;
				case 167 :
					result.Append("&Eacute;");
					break;
				case 231 :
					result.Append("&ccedil;");
					break;
				case 174 :
					result.Append("&Ccedil;");
					break;
				default :
					// undo UTF8-encoding
					result.Append((char)utfChar);
			}
		} else {
			// append UTF8 like normal character
			// might lead to loss of data!
			result.Append((char)utfChar);
		}
	}
	str = result;
}

String LDAPConnection::GetTypeStr(int msgType)
{
	StartTrace(LDAPConnection.GetTypeStr);

	String type;
	switch (msgType) {
		case LDAP_RES_BIND:
			type = "LDAP_RES_BIND";
			break;

		case LDAP_RES_SEARCH_ENTRY:
			type = "LDAP_RES_SEARCH_ENTRY";
			break;

		case LDAP_RES_SEARCH_REFERENCE:
			type = "LDAP_RES_SEARCH_REFERENCE";
			break;

		case LDAP_RES_SEARCH_RESULT:
			type = "LDAP_RES_SEARCH_RESULT";
			break;

		case LDAP_RES_MODIFY:
			type = "LDAP_RES_MODIFY";
			break;

		case LDAP_RES_ADD:
			type = "LDAP_RES_ADD";
			break;

		case LDAP_RES_DELETE:
			type = "LDAP_RES_DELETE";
			break;

		case LDAP_RES_MODDN:
			type = "LDAP_RES_MODDN";
			break;

		case LDAP_RES_COMPARE:
			type = "LDAP_RES_COMPARE";
			break;

		case LDAP_RES_EXTENDED:
			type = "LDAP_RES_EXTENDED";
			break;

		default:
			break;
	}
	return type;
}

String LDAPConnection::DumpConnectionHandle(LDAP *handle)
{
	StartTrace(LDAPConnection.DumpConnectionHandle);
	String returnString("Connection handle is: [");
	if ( handle != (LDAP *) NULL ) {
		// LDAP * is an opaque data type, but we know it is a pointer
		returnString.AppendAsHex((const unsigned char *) (void *) handle, sizeof(long));
		return returnString << "]";
	}
	return returnString << "NULL POINTER!]";
}
