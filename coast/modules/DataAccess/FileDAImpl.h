/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FileDAImpl_H
#define _FileDAImpl_H

#include "DataAccessImpl.h"
#include "SystemFile.h"

//---- FileDAImpl ----------------------------------------------------------
//! Abstract DataAccess for reading file from disk, see FileDAImpl relatives for concrete implementations
/*!
<B>Inputmapper-Configuration:</B><PRE>
{
	/DocumentRoot	Mapperspec		optional, if specified, this path will be prepended to the given filename
	/Filename		Mapperspec		mandatory, path and name of file
	/Extension		Mapperspec		optional, extension of the file if not already specified in Filename slot
	/Mode			Mapperspec		optional, [text|binary] (all lowercase!), default text, mode to open file
}
</PRE>
<B>Outputmapper-Configuration:</B><PRE>
{
	/FileContent	Mapperspec		optional, store the content of the file in this slot
}
</PRE>
*/
class FileDAImpl: public DataAccessImpl {
	FileDAImpl();
	FileDAImpl(const FileDAImpl &);
	FileDAImpl &operator=(const FileDAImpl &);
	friend class FileDAImplTest;
public:
	FileDAImpl(const char *name) :
		DataAccessImpl(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

protected:
	//! override to specify different file access modes
	virtual coast::system::openmode DoGetMode(ROAnything roaModes);

	bool GetFileName(String &filename, String &ext, Context &context, ParameterMapper *in);
	std::iostream *GetFileStream(Context &context, ParameterMapper *in);
	coast::system::openmode GetMode(Context &context, ParameterMapper *in);
};

#endif		//not defined _FileDAImpl_H
