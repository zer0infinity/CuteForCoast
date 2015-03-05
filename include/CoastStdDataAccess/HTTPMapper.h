/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPMAPPER_H
#define _HTTPMAPPER_H

#include "Mapper.h"

//! HTTP header parameter mapping
/*!
 * This mapper will do a Context::Lookup() using the given key. It expects a list
 * (Anything) split into <tt>/headerkey value</tt> pairs. This list will be used as
 * input to write onto the \c ostream argument given in the DoGetStream() function. The
 * contents of every headerkey not listed in the <em>Suppress</em> slot will get
 * rendered as <b><tt>headerkey: value\\r\\n</tt></b> pairs onto the stream.
 * Hierarchical <em>Suppress</em> slot configuration is possible so that derived
 * Mappers will inherit suppressed entries of their parents.
 *
 * @section headermapperconfig Mapper configuration
 *
 * @par \c Suppress
 * \b optional, default is to render all headerkey/value pairs\n
 * Anything, list specifying HTTP header fields to suppress.
 *
 * @section headermapperexample Example
 * @subsection headermapperexampleconfig Configuration (InputMapperMeta.any)
 * \code
/MyMapperAlias {
	/Suppress {
        keep-alive
        ACCEPT-ENCODING
        { accept connection }
	}
}
 * \endcode
 * @subsection headermapperexampletmpstore Prepared content of TmpStore
 * Assume the configured DataAccess will issue a ParameterMapper::Get("Header", ...)
 * \code
/TmpStore {
	/Header {
	    /HOST "my.host.dom"
        /USER-AGENT "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.6) Gecko/20100629 Ubuntu/10.10 (maverick) Firefox/3.6.6 GTB7.0"
        /ACCEPT {
          "text/html"
          "application/xhtml+xml"
          "application/xml;q=0.9"
        }
        /ACCEPT-ENCODING {
          "gzip"
          "deflate"
        }
        /KEEP-ALIVE "115"
        /CONNECTION {
          "keep-alive"
        }
	}
}
 * \endcode
 * @subsection headermapperexampleoutput Returned content on stream
 * \code
HOST: my.host.dom\r\nUSER-AGENT: Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.6) Gecko/20100629 Ubuntu/10.10 (maverick) Firefox/3.6.6 GTB7.0\r\n
 * \endcode
 */
class HTTPHeaderParameterMapper: public ParameterMapper {
	HTTPHeaderParameterMapper();
	HTTPHeaderParameterMapper(const HTTPHeaderParameterMapper &);
	HTTPHeaderParameterMapper &operator=(const HTTPHeaderParameterMapper &);
	bool DoInitialize();
	bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;
public:
	HTTPHeaderParameterMapper(const char *name) :
		ParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPHeaderParameterMapper(fName);
	}
protected:
	//! Render http header fields, look'd up in the Context using the given key and render onto os
	/*! @copydoc ParameterMapper::DoGetStream(const char *, std::ostream &, Context &, ROAnything) */
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything conf);
};

class HTTPBodyResultMapper: public ResultMapper {
	HTTPBodyResultMapper();
	HTTPBodyResultMapper(const HTTPBodyResultMapper &);
	HTTPBodyResultMapper &operator=(const HTTPBodyResultMapper &);
public:
	HTTPBodyResultMapper(const char *name) :
		ResultMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPBodyResultMapper(fName);
	}

	bool DoFinalPutStream(const char *key, std::istream &is, Context &ctx);

protected:
	virtual void ReadBody(String &body, std::istream &is, Context &ctx);
};

class HTTPBodyParameterMapper: public ParameterMapper {
	HTTPBodyParameterMapper();
	HTTPBodyParameterMapper(const HTTPBodyParameterMapper &);
	HTTPBodyParameterMapper &operator=(const HTTPBodyParameterMapper &);
public:
	HTTPBodyParameterMapper(const char *name) :
		ParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPBodyParameterMapper(fName);
	}

	bool DoFinalGetStream(const char *key, std::ostream &os, Context &ctx);
};

#endif
