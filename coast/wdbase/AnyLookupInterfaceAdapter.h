/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyLookupInterfaceAdapter_H
#define _AnyLookupInterfaceAdapter_H

#include "IFAConfObject.h"
#include "ITOTypeTraits.h"
#include "Tracer.h"

/*! <B>Wraps [RO]Anything in a LookupInterface (as needed eg. by Context::Push())</B> */
template < class ContainerType >
class AnyLookupInterfaceAdapter : public LookupInterface
{
public:
	typedef typename boost_or_tr1::remove_reference< typename boost_or_tr1::remove_const<ContainerType>::type>::type PlainType;
	typedef typename boost_or_tr1::add_reference<typename boost_or_tr1::add_const<PlainType>::type>::type _ParameterType;
	typedef typename boost_or_tr1::mpl::if_<boost_or_tr1::is_same<PlainType, ROAnything>, ROAnything, _ParameterType>::type ParameterType;

	/*! Constructor for LookupAdapter
		\param container [RO]Anything to use as underlying data container
		\param pcBaseKey optional param which specifies the segment name used to emulate nested content in a Lookup. If the lookup-key starts with this name we cut it away before doing a concrete lookup.*/
	explicit AnyLookupInterfaceAdapter(ParameterType container, const char *pcBaseKey = NULL)
		: fContainer(container)
		, fstrBaseKey(pcBaseKey)
	{}

protected:
	ROAnything fContainer;
	String	fstrBaseKey;

	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		StartTrace(AnyLookupInterfaceAdapter.DoLookup);
		String strKey(key);
		// check if we need to adjust the key
		if ( fstrBaseKey.Length() ) {
			if ( strKey.StartsWith(fstrBaseKey) ) {
				Trace("given key [" << strKey << "] starts with [" << fstrBaseKey << "]");
				long lKeyLength = fstrBaseKey.Length();
				if ( ( strKey.Length() > lKeyLength ) && ( strKey[lKeyLength] == delim ) ) {
					++lKeyLength;
					Trace("deep non-index key detected, also cut away delim, length to trim:" << lKeyLength);
				}
				strKey.TrimFront(lKeyLength);
				Trace("key after trimming [" << strKey << "]");
				if ( !strKey.Length() ) {
					// here we return the whole internal store when fstrBaseKey was the only key
					result = fContainer;
					return true;
				}
			} else {
				Trace("given key [" << strKey << "] does not start with [" << fstrBaseKey << "], content not found and returning false");
				return false;
			}
		}
		bool bRet = fContainer.LookupPath(result, strKey, delim, indexdelim);
		TraceAny(result, "Looking up [" << strKey << "] " << (bRet?"resulted in":"failed, result any unchanged"));
		return bRet;
	}
};

class ROAnyConfNamedObjectLookupAdapter : public AnyLookupInterfaceAdapter< ROAnything >
{
public:
	typedef AnyLookupInterfaceAdapter<ROAnything> BaseClassType;

	ROAnyConfNamedObjectLookupAdapter(ROAnything ro, ConfNamedObject *cfno)
		: BaseClassType(ro) {
		fCfno = cfno;
	}

protected:
	ConfNamedObject *fCfno;

	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		if ( !BaseClassType::DoLookup(key, result, delim, indexdelim) ) {
			if ( fCfno != (ConfNamedObject *)NULL ) {
				return fCfno->Lookup(key, result, delim, indexdelim);
			}
			return false;
		}
		return true;
	}
};
#endif
