/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WriteFileDAImpl_H
#define _WriteFileDAImpl_H

#include "FileDAImpl.h"

//---- WriteFileDAImpl ----------------------------------------------------------
//! DataAccess for writing a file to disk
/*!
<B>Inputmapper-Configuration:</B><PRE>
{
	/DocumentRoot	Mapperspec		optional, if specified, this path will be prepended to the given filename
	/Filename		Mapperspec		mandatory, path and name of file
	/Extension		Mapperspec		optional, extension of the file if not already specified in Filename slot
	/Mode			Mapperspec		optional, [text|binary|append|truncate|noreplace] (all lowercase!), default text and noreplace, mode to open file for writing
}
</PRE>
<B>Inputmapper-Configuration:</B><PRE>
{
	/FileContent	Mapperspec		optional, store the content of this slot into the file
}
*/
class WriteFileDAImpl: public FileDAImpl
{
public:
	WriteFileDAImpl(const char *name);
	~WriteFileDAImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	virtual coast::system::openmode DoGetMode(ROAnything roaModes);

private:
	//constructor
	WriteFileDAImpl();
	WriteFileDAImpl(const WriteFileDAImpl &);
	//assignement
	WriteFileDAImpl &operator=(const WriteFileDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _WriteFileDAImpl_H
