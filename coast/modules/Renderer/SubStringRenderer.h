/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SubStringRenderer_H
#define _SubStringRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- SubStringRenderer ----------------------------------------------------------
//! <B>String.SubString() as Renderer</B>
/*!
\par Configuration
\code
{
	/String		Rendererspec	mandatory, String from which to get the substring
	/Start		Rendererspec	optional, default 0, index of first character of substring
	/Length		Rendererspec	optional, default -1, length of substring, -1 means all remaining chars
}
\endcode
*/
class EXPORTDECL_RENDERER SubStringRenderer : public Renderer
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	SubStringRenderer(const char *name);
	~SubStringRenderer();

	//! Renders String.SubString() from \c Start with length \c Length onto \em reply
	/*! @copydetails Renderer::RenderAll(ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
