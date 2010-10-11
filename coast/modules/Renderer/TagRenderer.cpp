/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TagRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "OptionsPrinter.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- TagRenderer ---------------------------------------------------------
RegisterRenderer(TagRenderer);

TagRenderer::TagRenderer(const char *name) : Renderer(name)
{
}

TagRenderer::~TagRenderer()
{
}

void TagRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(TagRenderer.Render);
	TraceAny(config, "config");

	ROAnything ROtag;
	if ( config.LookupPath(ROtag, "Tag") ) {
		// open start tag
		String tag = ROtag.AsString();
		if ( tag.Length() ) {
			reply << '<' << tag;

			// render options
			PrintOptions3(reply, c, config);

			// close start tag
			reply << ">\n" ;

			ROAnything content;
			if ( config.LookupPath(content, "Content") ) {
				// render content
				Render(reply, c, content);
			}

			// render end tag
			// suppressed if config contains slot /NoEndTag
			if ( !config.IsDefined("NoEndTag") ) {
				reply << "</" << tag << ">\n";
			}
		}
	}
}
