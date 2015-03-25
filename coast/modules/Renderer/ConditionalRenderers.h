/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConditionalRenderers_H
#define _ConditionalRenderers_H

#include "Renderer.h"

// ---- ConditionalRenderer ---------------------------------------------------------
//! This renderer checks the value of a specified slot in the Context and - depending on the outcome of the test - renders a predefined renderer specification.
/*!
\par Configuration
\code
{
	/ContextCondition	Rendererspec	mandatory, resulting a String used to Lookup the Context. (value looked up is usually an integer)
	/True				Rendererspec	optional, if ContextCondition-value evaluates to true, the content of this slot will be rendered
	/False				Rendererspec	optional, if ContextCondition-value evaluates to false, the content of this slot will be rendered
	/Defined			Rendererspec	optional, if ContextCondition-value is defined in Context, the content of this slot will be rendered
	/Undefined			Rendererspec	optional, if ContextCondition-value not defined in Context, the content of this slot will be rendered
	/Error				Rendererspec	optional, if the renderer received invalid input, the content of this slot will be rendered
}
\endcode
*/
class ConditionalRenderer : public Renderer
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	ConditionalRenderer(const char *name);

	//! Render result to stream based on existence of rendered ContextCondition string in Context
	/*! @copydoc Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);

protected:
	/*! TestCondition does the actual testing. Result of test is returned via res. Errors are signaled using the value "Error".
	 * @param ctx Context to work with
	 * @param config configuration which drives the output generation
	 * @param res conditions test result */
	virtual void TestCondition(Context &ctx, const ROAnything &config, String &res);
};

// ---- SwitchRenderer ---------------------------------------------------------
//! The SwitchRenderer provides an indirection depending on some value in the Context
/*!
\par Configuration
\code
{
	/ContextLookupName	Rendererspec	mandatory, resulting a String used to Lookup the Context
	/PathDelim			Rendererspec	optional, an arbitrary char or "Ignore" which is a synonym for a char with hex value 0.
												  The later results in a literal, uninterpreted lookup of the /ContextLookupName
	/IndexDelim			Rendererspec	optional, see /PathDelim description.
	/Case {								mandatory, list of different cases to compare ContextLookupName-value with
		/xxx			Rendererspec	optional, if context.Lookup("ContextLookupName-value") == xxx, the content of this slot will be rendered
		/yyy			Rendererspec	optional, if context.Lookup("ContextLookupName-value") == yyy, the content of this slot will be rendered
		/''				Rendererspec	optional, if context.Lookup("ContextLookupName-value") is empty, the content of this slot will be rendered, Important: you must use single quotes when defining Anythings empty slotname!
	}
	/Default			Rendererspec	optional, for all unspecified conditions (not listed in Case slot), the content of this slot will be rendered
}
\endcode
*/
class SwitchRenderer : public Renderer
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	SwitchRenderer(const char *name);

	//! Render result to stream based on evaluation of ContextLookupName
	/*! @copydoc Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif	//not defined _ConditionalRenderers_H
