/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLECONNECTION_H_
#define ORACLECONNECTION_H_

#include "Anything.h"
#include "SystemLog.h"
#include "OracleEnvironment.h"

class OracleStatement;

//! Abstraction for an Oracle connection
/*!
 * This class serves as abstraction for all the OCI calls which need to be done to establish a connection to an oracle back end.
 * The OracleConnection itself will be created when calling OracleEnvironment::createConnection. All this is done from within
 * OraclePooledConnection which will be tracked by coast::oracle::ConnectionPool.
 * The main functions this class serves for is to OracleConnection::Open and OracleConnection::Close the connection
 * to the back end and to let OracleConnection::createStatement give us an OracleStatement. The statement is used to
 * execute any type of valid oracle queries.
 */
class OracleConnection
{
public:
	enum Status {
		eUnitialized, //!< state if constructor failed and connection can not be used
		eHandlesAllocated, //!< signals that all handles were allocated successfully
		eServerAttached, //!< server attached/connected
		eSessionValid,
		//!< user authenticated, ready to execute statements
	};

	//! Specify pseudo object type of a statement
	enum ObjectType {
		TYPE_UNK = OCI_PTYPE_UNK, //!< unknown type (0)
		TYPE_PROC = OCI_PTYPE_PROC, //!< procedure type (3)
		TYPE_FUNC = OCI_PTYPE_FUNC, //!< function type (4)
		TYPE_SYN = OCI_PTYPE_SYN, //!< synonym type (7)
		TYPE_SIMPLE = 177,
		//!< simple query type, like select, update etc
	};

private:
	//! track the connection status
	Status fStatus;
	//! hold a reference to the surrounding environment
	OracleEnvironment &fOracleEnv;
	//! OCI error handle
	ErrHandleType fErrhp;
	//! OCI server connection handle
	SrvHandleType fSrvhp;
	//! OCI service context handle
	SvcHandleType fSvchp;
	//! OCI user session handle
	UsrHandleType fUsrhp;

	OracleConnection();
	OracleConnection( const OracleConnection & );
public:
	/*! Main construction entry point
	 * @param rEnv the surrounding OracleEnvironment which was used to create us
	 */
	OracleConnection( OracleEnvironment &rEnv );
	/*! Destruction of the connection, close if needed */
	~OracleConnection();

	/*! Open a connection to the back end
	 *
	 * @param strServer Server connection string to connect with
	 * @param strUsername Username to connect as
	 * @param strPassword Password for the given user
	 * @return true in case we could establish a connection to the back end
	 */
	bool Open( String const &strServer, String const &strUsername, String const &strPassword );
	/*! Close the connection to the backend if it was opened before
	 */
	void Close();

	/*! Check whether the connection is open
	 * @return true if we are connected to the back end */
	bool isOpen() const {
		return fStatus == eSessionValid;
	}

	/*! Gain access to the surrounding OracleEnvironment class
	 *
	 * @return Reference to the OracleEnvironment which was used to create this OracleConnection
	 */
	OracleEnvironment &getEnvironment() const {
		return fOracleEnv;
	}

	/*! Create a new OracleStatement object based on this connection
	 *
	 * @param strStatement Statement string to use for the newly created OracleStatement object
	 * @param lPrefetchRows How many rows to fetch in a server round trip, can be used to optimize performance on a per statement basis
	 * @param roaSPDescription Needs to be supplied if the statement is a stored procedure or function. This data is needed to supply the correct parameter names and types.
	 * @return newly created OracelStatment in case of success
	 */
	OracleStatementPtr createStatement( String strStatement, long lPrefetchRows, OracleConnection::ObjectType aObjType =
											OracleConnection::TYPE_SIMPLE, String strReturnName = String() );

	/*! access OCIError handle
	 *
	 * @return OCIError handle
	 */
	OCIError *ErrorHandle() {
		return fErrhp.getHandle();
	}

	/*! access OCISvcCtx handle
	 *
	 * @return OCISvcCtx handle
	 */
	OCISvcCtx *SvcHandle() {
		return fSvchp.getHandle();
	}

	/*! check the given status value against an error condition and report it as message
	 *
	 * @param status OCI status value to check
	 * @param message String with appropriate error message filled in
	 * @return true in case there was an error
	 */
	bool checkError( sword status, String &message );

	/*! Return an error message string for the given status value
	 *
	 * @param status OCI status value to use
	 * @return Appropriate error message
	 */
	String errorMessage( sword status );

	/*! check the given status value against an error condition
	 *
	 * @param status OCI status value to check
	 * @return true in case there was an error
	 */
	bool checkError( sword status );

	template<class handlePtrType>
	bool AllocateHandle(handlePtrType &aHandlePtr) {
		if (OCIHandleAlloc(fOracleEnv.EnvHandle(), aHandlePtr.getVoidAddr(), aHandlePtr.getHandleType(), (size_t) 0, (dvoid **) 0)
				!= OCI_SUCCESS) {
			SystemLog::Error(String("FAILED: OCIHandleAlloc(): alloc handle of type ") << (long) aHandlePtr.getHandleType() << " failed");
			return false;
		}
		return true;
	}

private:
	ObjectType GetSPDescription( const String &command, ROAnything &desc );
	ObjectType ReadSPDescriptionFromDB( const String &command, Anything &desc );
	ObjectType DescribeObjectByName(const String &command, DscHandleType &aDschp, OCIParam *&parmh);

	String ConstructSPStr( String const &command, bool pIsFunction, ROAnything desc, const String &strReturnName );
};

#endif /* ORACLECONNECTION_H_ */
