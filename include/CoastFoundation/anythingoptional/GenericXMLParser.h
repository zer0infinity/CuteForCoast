/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _GenericXMLParser_H
#define _GenericXMLParser_H

//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
#include "Anything.h"

//! construct a simple DOM representation using Anything from an XML file
/*!
construct an anything representation of DOM parsing the XML
*/
class GenericXMLParser
{
public:
	virtual ~GenericXMLParser() {}//lint !e1401//lint !e1401
	//! do the parsing,
	//! \return the constructed Anything using the given Allocator
	//! \param reader the input source
	//! \param filename for giving convenient error messages when reading from a real file
	//! \param startline the line number when starting the parsing for convenient error messages
	//! \param a the allocator to use, provide coast::storage::Global() for config data
	Anything Parse(std::istream &reader, const char *filename = "NO_FILE", long startline = 1L, Allocator *a = coast::storage::Current());
protected:
	virtual void DoParse(String endTag, Anything &tag);
	virtual Anything ParseComment();
	virtual Anything ParseXmlOrProcessingInstruction();
	virtual bool ParseTag(String &tag, Anything &attributes);
	virtual String ParseValue();
	virtual bool ParseAttribute(String &name, String &value);
	virtual Anything ParseCommentCdataOrDtd(bool withindtd = false);
	virtual Anything ParseDtd();
	virtual Anything ParseExternalId();
	virtual Anything ParseDtdElements();
	virtual String SkipToClosingAngleBracket();
	virtual Anything ParseCdata();
	virtual String SkipToCdataClosing();

	virtual String ParseAsStringUpToEndTag(String &tagName);
	virtual void Error(const char *msg);

	virtual Anything ProcessArgs(const String &renderer, const String &args);
	virtual void Store(Anything &cache, String &htmlBlock);
	virtual void Store(Anything &cache, const String &htmlBlock);
	virtual void Store(Anything &cache, const Anything &args);

	virtual String ParseName();
	virtual String ParseQuotedString();
	virtual	String ParseToSemicolon();

	virtual void SkipWhitespace();
	virtual bool IsValidNameChar(int c);

	virtual int Get();
	virtual int Peek();
	virtual bool IsEof();
	virtual void PutBack(char c);
	Anything fParseTree;
	std::istream *fReader;
	String	fFileName;
	long	fLine;
};

class GenericXMLPrinter
{
public:
	static void PrintXml(std::ostream &os, ROAnything domany);
	static void DoPrintXml(std::ostream &os, ROAnything domany);
	static void DoPrintXmlTag(std::ostream &os, const String &tag, ROAnything attributes);
	static void DoPrintXmlPI(std::ostream &os, const String &pitag, ROAnything subdomany);
	static void DoPrintXmlComment(std::ostream &os, ROAnything subdomany);
	static void DoPrintXmlCdata(std::ostream &os, ROAnything subdomany);
	static void DoPrintXmlDtd(std::ostream &os, ROAnything subdomany);
	static void DoPrintXmlSubDtd(std::ostream &os, ROAnything subdomany);
};

#endif
