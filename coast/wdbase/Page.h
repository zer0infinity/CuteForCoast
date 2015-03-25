/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PAGE_H
#define _PAGE_H

#include "WDModule.h"
#include "Registry.h"
#include "Tracer.h"

class Context;

class PagesModule: public WDModule {
public:
	PagesModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

//!abstraction for generation of html page content; it manages preparation, rendering and postprocessing of pages
//!a page manages the preparation, rendering and postprocessing of pages.<br>
//!Preparation and postprocessing uses the Action class to execute code related to transition tokens.<br>
//!Render normally uses the Renderer class to generate page content. Although it is possible to overwrite hook methods in subclasses,
//!it should not be necessary since almost everything is configurable.
class Page : public HierarchConfNamed
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	Page(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! deprecated- use Prepare and Render
	virtual void Start(std::ostream &reply, Context &context);

	//! do Preprocessing which will in fact execute all relevant Actions
	/*! otherwise return false and set a transition where to go
		or set currentpage directly if something really goes wrong
		\param transition - in/out action to call and transition to take if false
		\param c Context in which to process token
		\return true if preparation was successful, Render will then work fine
		\return false otherwise, transition will show the way to go */
	virtual bool Prepare(String &transition, Context &c);

	//!generates page content by using subclass page rendering hooks
	virtual void Render(std::ostream &reply, Context &c);

	//!postprocessing of a request coming from this page
	virtual bool Finish(String &action, Context &context);

	RegCacheDef(Page);

	static const char* gpcCategory;
	static const char* gpcConfigPath;

protected:
	//! transition token processing
	/*! \param transitionToken in/out actionscript to call
		\param context Context in which to process token
		\return success of actionscript */
	bool ProcessToken(String &transitionToken, Context &context);

	//!deprecated hook
	virtual void Preprocess(Context &c);

	//! render protocol status line
	virtual void RenderProtocolStatus(std::ostream &reply, Context &c);
	//! render protocol header lines
	virtual void RenderProtocolHeader(std::ostream &reply, Context &c);
	//! renders protocol body using "PageLayout" tag or subclass api sequence of Header, Title, Body, Footer to render HTML content
	virtual void RenderProtocolBody(std::ostream &reply, Context &c);
	//! render the tail e.g. debug output
	virtual void RenderProtocolTail(std::ostream &reply, Context &c);

	//! render HTML Title
	virtual void Title(std::ostream &reply, Context &c);
	//!render HTML Header
	virtual void Header(std::ostream &reply, Context &c);
	//!render HTML Body
	virtual void Body(std::ostream &reply, Context &c);
	//!renders HTML end tags
	virtual void Footer(std::ostream &reply, Context &c);

	//! Mime output
	virtual void Mime(std::ostream &reply, Context &c);

	friend class PreprocessAction;
private:
	//!subclass hook to implement postprocessing; legacy, use actions instead
	virtual bool Postprocess(String &action, Context &c) {
		return false;
	}
	//!subclass hook to implement preprocessing; legacy, use actions instead
	virtual bool DoPrepare(String &transition, Context &c) {
		return false;
	}
};

#define RegisterPage(name) RegisterObject(name, Page)

#endif
