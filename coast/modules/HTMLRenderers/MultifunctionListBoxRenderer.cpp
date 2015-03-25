/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MultifunctionListBoxRenderer.h"
#include "StringStream.h"
#include "utf8.h"

namespace {
	long getStringLength(String const &str) {
		long len = 0L;
		try {
			len = utf8::distance(str.begin(), str.end());
		} catch (utf8::invalid_utf8& e) {
			len = str.Length();
		}
		StatTrace(MultifunctionListBoxRenderer.getStringLength, "len: " << len << " str [" << str << "]", coast::storage::Current());
		return len;
	}
}

RegisterRenderer(HeaderListRenderer);
void HeaderListRenderer::RenderEntry(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(HeaderListRenderer.RenderEntry);
	TraceAny(entryRendererConfig, "entryRendererConfig");
	TraceAny(listItem, "listItem");
	bool boHide = MultifunctionListBoxRenderer::IsHiddenField(c, listItem);
	if (!boHide) {
		reply << "<th";
		if (config.IsDefined("CellOptions")) {
			PrintOptions3(reply, c, config["CellOptions"]);
		}
		reply << ">";
		static Renderer *pRenderer = Renderer::FindRenderer("FormattedStringRenderer");
		if (pRenderer) {
			Anything formatConfig;
			String strName;
			Renderer::RenderOnString(strName, c, listItem["Name"]);
			formatConfig["Value"] = strName;

			if (listItem.IsDefined("HeaderAlign")) {
				formatConfig["Align"] = listItem["HeaderAlign"].DeepClone();
			} else {
				formatConfig["Align"] = "center";
			}

			ROAnything roaWidth;
			String strWidth;
			if (listItem.LookupPath(roaWidth, "Width")) {
				RenderOnString(strWidth, c, roaWidth);
			}
			long const nameLen = getStringLength(strName);
			long lWidth = strWidth.AsLong(nameLen);

			// Have we got any data in the Box?
			long dataSize = 0L;
			if (config.IsDefined("ListDataSize")) {
				dataSize = config["ListDataSize"].AsLong(0L);
			}
			if (config.IsDefined("SortIcon")) {
				if (listItem.IsDefined("Sortable") && dataSize > 1) {
					lWidth = lWidth - 2;

					// we have to check if the width isn't minus
					if ( lWidth < 0 ) {
						lWidth = nameLen;
					}
				}
			}
			formatConfig["Width"] = (lWidth + 1);
			if (listItem.IsDefined("Filler")) {
				formatConfig["Filler"] = listItem["Filler"].DeepClone();
			} else {
				formatConfig["Filler"] = "&nbsp;";
			}

			TraceAny(formatConfig, "formatConfig");
			pRenderer->RenderAll(reply, c, formatConfig);

			if (config.IsDefined("SortIcon")) {
				if (listItem.IsDefined("Sortable") && dataSize > 1) {
					DoRenderSortIcons(reply, c, config, listItem);
				}
			}
		}
		c.GetTmpStore()["VisibleColumns"] = c.GetTmpStore()["VisibleColumns"].AsLong(0L) + 1L;
		reply << "</th>\n";
	}
}

void HeaderListRenderer::DoRenderSortIcons(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem)
{
	ROAnything roSortIcon = config["SortIcon"];
	if (!roSortIcon.IsNull()) {
		String strSortOrderAsc, strSortOrderDesc, strTmpSort;

		if (IsSortableColumn(c, listItem)) {
			strSortOrderAsc = listItem["Sortable"].AsString();
			strSortOrderAsc.Append(" asc");

			strSortOrderDesc = listItem["Sortable"].AsString();
			strSortOrderDesc.Append(" desc");

			long lSortColumn = c.Lookup("SortColumn", -1L);
			long lCurrentColumn = c.Lookup("HeaderColumnIndex", -1L);
			strTmpSort = c.Lookup("SortOrder", "");

			// gets the sortorder from tempstore
			// strTmpSort= c.GetTmpStore()["Sort"].AsString();
			// strTmpSort= sortVar["Sort"].AsString();

			ROAnything imageAscConfig;
			ROAnything imageDescConfig;
			// Is already a sortorder chosen
			if ( lCurrentColumn == lSortColumn ) {
				if ( strTmpSort.IsEqual("asc") ) {
					if (roSortIcon.LookupPath(imageAscConfig, "ImageAscChosen")) {
						RenderSortIcon(reply, c, config, strSortOrderAsc, "asc", imageAscConfig);
					}
				} else {
					if (roSortIcon.LookupPath(imageAscConfig, "ImageAscending")) {
						RenderSortIcon(reply, c, config, strSortOrderAsc, "asc", imageAscConfig);
					}
				}
				if ( strTmpSort.IsEqual("desc") ) {
					if (roSortIcon.LookupPath(imageDescConfig, "ImageDescChosen")) {
						RenderSortIcon(reply, c, config, strSortOrderDesc, "desc", imageDescConfig);
					}
				} else {
					if (roSortIcon.LookupPath(imageDescConfig, "ImageDescending")) {
						RenderSortIcon(reply, c, config, strSortOrderDesc, "desc", imageDescConfig);
					}
				}
			}
			// no temporary sortorder chosen
			else {
				if (roSortIcon.LookupPath(imageAscConfig, "ImageAscending")) {
					RenderSortIcon(reply, c, config, strSortOrderAsc, "asc", imageAscConfig);
				}

				if (roSortIcon.LookupPath(imageDescConfig, "ImageDescending")) {
					RenderSortIcon(reply, c, config, strSortOrderDesc, "desc", imageDescConfig);
				}
			}
		}
	}
}

void HeaderListRenderer::RenderSortIcon(std::ostream &reply, Context &c, const ROAnything &config, String strSortString, String strSortOrder, const ROAnything &imageConfig)
{
	StartTrace(MultifunctionListBoxRenderer.RenderSortIcons);

	String strActionName, strFormName;
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);

	ROAnything roSortIcon = config["SortIcon"];
	if (!roSortIcon.IsNull()) {
		String strStyle;
		if ( roSortIcon.IsDefined("LinkStyle") ) {
			strStyle << "class=\"" << roSortIcon["LinkStyle"].AsString() << "\" ";
		}
		reply << "<a " << strStyle << "href=\"#\" onClick=\" document.forms[" << strFormName << "].action='";

		static Renderer *pRenderer = Renderer::FindRenderer("URLRenderer");
		if (pRenderer) {
			Anything urlConfig;
			if (roSortIcon.IsDefined("Action")) {
				urlConfig["Action"] = roSortIcon["Action"].DeepClone();;
			}
			if (roSortIcon.IsDefined("Params")) {
				// take the configured values
				urlConfig["Params"] = roSortIcon["Params"].DeepClone();
				urlConfig["Params"]["Sort"] = strSortString;
				urlConfig["Params"]["SortOrder"] = strSortOrder;
				urlConfig["Params"]["SortColumn"] = c.Lookup("HeaderColumnIndex", -1L);
			}

			pRenderer->RenderAll(reply, c, urlConfig);
			reply << "'; document.forms[" << strFormName << "].submit(); return false;\" >";

			if (!imageConfig.IsNull()) {
				Render(reply, c, imageConfig);
			}
			reply << "</a>";
		}
	}
}

//-- diese methode liefert zurück, ob eine column sortierbar ist oder nicht -- ist ein helpermethode
bool HeaderListRenderer::IsSortableColumn(Context &c, const ROAnything &toCheckConfig)
{
	StartTrace(MultifunctionListBoxRenderer.IsSortableColumn);
	ROAnything anySort;
	bool bSortable = false;
	if (toCheckConfig.LookupPath(anySort, "Sortable")) {
		String strSortable;
		RenderOnString(strSortable, c, anySort);
		if ( strSortable.Length() > 0 ) {
			bSortable = true;
		}
		Trace("Column is " << (bSortable ? "" : "not ") << "sortable");
	}
	return bSortable;
}

//---- MultifunctionListBoxRenderer ---------------------------------------------------------------

RegisterRenderer(MultifunctionListBoxRenderer);

MultifunctionListBoxRenderer::MultifunctionListBoxRenderer(const char *name) : SelectBoxRenderer(name) { }

MultifunctionListBoxRenderer::~MultifunctionListBoxRenderer() { }

bool MultifunctionListBoxRenderer::Lookup(const ROAnything &nameConfig, Context &c, const ROAnything &config, Anything &result)
{
	StartTrace(MultifunctionListBoxRenderer.Lookup);
	bool found(false);
	String name;
	RenderOnString(name, c, nameConfig);

	Anything tempStore = c.GetTmpStore();
	// Check first within config - prevents us from DeepCloning
	if ( config.IsDefined(name) ) {
		// data is found in config
		Trace("[" << name << "] found in config");
		result = config[name].DeepClone();
		found = true;
	} else if ( tempStore.IsDefined(name) ) {
		// data is found in tempStore
		Trace("[" << name << "] found in tempStore");
		result = tempStore[name];
		found = true;
	} else {
		ROAnything ROlist = c.Lookup(name);
		if ( ! ROlist.IsNull() ) {
			// data is found in Context
			Trace("[" << name << "] found in Context");
			result = ROlist.DeepClone();
			found = true;
		}
	}

	return found;
}

bool MultifunctionListBoxRenderer::IsHiddenField(Context &c, const ROAnything &toCheckConfig)
{
	StartTrace(MultifunctionListBoxRenderer.IsHiddenField);
	ROAnything anyHide;
	bool boHide = false;
	if (toCheckConfig.LookupPath(anyHide, "Hide")) {
		String strHide;
		RenderOnString(strHide, c, anyHide);
		boHide = strHide.AsLong(0L);
		Trace("Field is " << (boHide ? "hidden" : "visible"));
	}
	return boHide;
}

bool MultifunctionListBoxRenderer::IsEditableField(Context &c, const ROAnything &toCheckConfig)
{
	StartTrace(MultifunctionListBoxRenderer.IsEditableField);
	ROAnything anyEdit;
	bool bEditable = false;
	if (toCheckConfig.LookupPath(anyEdit, "Editable")) {
		String strEditable;
		RenderOnString(strEditable, c, anyEdit);
		bEditable = strEditable.AsLong(0L);
		Trace("Field is " << (bEditable ? "" : "not ") << "editable");
	}
	return bEditable;
}

bool MultifunctionListBoxRenderer::IsPulldownField(Context &c, const ROAnything &toCheckConfig)
{
	StartTrace(MultifunctionListBoxRenderer.IsPulldownField);
	bool bPulldown = toCheckConfig.IsDefined("Pulldown");
	Trace("Field is " << (bPulldown ? "" : "not ") << "pulldown");
	return bPulldown;
}

bool MultifunctionListBoxRenderer::IsChangeableField(Context &c, const ROAnything &toCheckConfig)
{
	StartTrace(MultifunctionListBoxRenderer.IsChangeableField);
	ROAnything anyChangeable;
	bool bChangeable = true;
	if (toCheckConfig.LookupPath(anyChangeable, "Changeable")) {
		String strChangeable;
		RenderOnString(strChangeable, c, anyChangeable);
		bChangeable = strChangeable.AsLong(0L);
		Trace("Field is " << (bChangeable ? "" : "not ") << "changeable");
	}
	return bChangeable;
}

bool MultifunctionListBoxRenderer::IsDataTypeDefined(Context &c, const ROAnything &toCheckConfig)
{
	StartTrace(MultifunctionListBoxRenderer.IsDataTypeDefined);
	bool bIsDataTypeDefined = toCheckConfig.IsDefined("DataType");
	Trace("Datatype is " << (bIsDataTypeDefined ? "" : "not ") << "defined");
	return bIsDataTypeDefined;
}

void MultifunctionListBoxRenderer::RenderCellName(String &cellName, Context &c, const ROAnything &cellItem)
{
	StartTrace(MultifunctionListBoxRenderer.RenderCellName);
	static Renderer *pCharReplRenderer = Renderer::FindRenderer("CharReplaceRenderer");
	if (pCharReplRenderer) {
		Anything charReplaceConfig;

		charReplaceConfig["CharsToReplace"] = "/+-* ";
		ROAnything roaName;
		if ( !cellItem.LookupPath(roaName, "IntName") ) {
			cellItem.LookupPath(roaName, "Name");
		}
		charReplaceConfig["String"] = roaName.DeepClone();

		OStringStream os(cellName);
		pCharReplRenderer->RenderAll(os, c, charReplaceConfig);

	}
	Trace("Cellname:[" << cellName << "]");
}

void MultifunctionListBoxRenderer::RenderBoxName(String &boxName, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderBoxName);
	ROAnything n = config["Name"];
	if ( ! n.IsNull() ) {
		static Renderer *pCharReplRenderer = Renderer::FindRenderer("CharReplaceRenderer");
		if (pCharReplRenderer) {
			Anything charReplaceConfig;

			charReplaceConfig["CharsToReplace"] = "/+-* ";
			charReplaceConfig["String"] = n.DeepClone();

			OStringStream os(boxName);
			pCharReplRenderer->RenderAll(os, c, charReplaceConfig);

		}
	}
	Trace("Boxname:[" << boxName << "]");
	c.GetTmpStore()["MultifunctionListBoxName"] = boxName;
}

void MultifunctionListBoxRenderer::GetBoxName(String &boxName, Context &c)
{
	ROAnything roaValue;
	if ( c.Lookup("MultifunctionListBoxName", roaValue) ) {
		boxName = roaValue.AsString();
	}
}

void MultifunctionListBoxRenderer::RenderFormName(String &formName, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderFormName);
	ROAnything roFormCfg;
	if (config.LookupPath(roFormCfg, "FormName")) {
		formName = "";
		formName << "'";
		RenderOnString(formName, c, roFormCfg);
		formName << "'";
	} else {
		// use default, first form!
		formName = "0";
	}
	Trace("FormName:[" << formName << "]");
	c.GetTmpStore()["MultifunctionFormName"] = formName;
}

void MultifunctionListBoxRenderer::GetFormName(String &formName, Context &c)
{
	ROAnything roaValue;
	if ( c.Lookup("MultifunctionFormName", roaValue) ) {
		formName = roaValue.AsString();
	} else {
		formName = "0";
	}
}

void MultifunctionListBoxRenderer::GetDataType(String &dataType, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.GetDataType);
	ROAnything roDataType;
	dataType = "";
	if (config.LookupPath(roDataType, "DataType")) {
		dataType << "'";
		RenderOnString(dataType, c, roDataType);
		dataType << "'";
		Trace("DataType is:[" << dataType << ']');
	}
}

void MultifunctionListBoxRenderer::GetTagStyle(String &tagStyle, Context &ctx)
{
	if (ctx.GetTmpStore().IsDefined("TagStyle")) {
		tagStyle = ctx.GetTmpStore()["TagStyle"].AsString();
	} else {
		tagStyle = "0";
	}
}

void MultifunctionListBoxRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderAll);
	TraceAny(config, "config");
	ROAnything n = config["Name"];
	if ( ! n.IsNull() ) {
		String strBoxName, strFormName;
		MultifunctionListBoxRenderer::RenderBoxName(strBoxName, c, config);
		MultifunctionListBoxRenderer::RenderFormName(strFormName, c, config);

		reply << "\n<!-- BEGIN [" << strBoxName << "] -->\n";

		reply << "<!-- begin of backgroundtable -->\n";
		reply << "<table width=\"10%\"";
		reply << " border=" << config["BorderWidth"].AsLong(0L);
		ROAnything roBgColor;
		String strBgColor;
		if (config.LookupPath(roBgColor, "BgColor")) {
			strBgColor << " bgcolor='";
			RenderOnString(strBgColor, c, roBgColor);
			strBgColor << "'";
		}
		reply << strBgColor;
		reply << "><tbody>\n";
		{
			reply << "<tr><td>\n";
			reply << "<!-- begin of innertable -->\n";
			reply << "<table width=\"10%\" border=" << config["InnerTable"]["BorderWidth"].AsLong(0L) << " cellspacing=\"2\" cellpadding=\"1\">\n";
			{
				c.GetTmpStore().Remove("VisibleColumns");
				long lColumns = 0L;
				//-- begin of headerrow, check if it should be suppressed
				if (!config.IsDefined("NoHeader")) {
					RenderHeader(reply, c, config, lColumns);
				}
				reply << "\n<!-- now do the box itself -->\n";
				RenderBoxRow(reply, c, config, lColumns);

				reply << "\n<!-- here we go with some edit fields if required -->\n";
				RenderColumnInputFields(reply, c, config, lColumns);

				reply << "\n<!-- here we have filter fields if required -->\n";
				RenderAdditionalFilters(reply, c, config, lColumns);

				reply << "\n<!-- and now the StatusMessage -->\n";
				RenderStatusMessage(reply, c, config, lColumns);
			}
			reply << "</table>\n";
			reply << "<!-- end of innertable -->\n";
			reply << "</td></tr>";
		}
		reply << "</tbody>\n";
		reply << "</table>\n";
		reply << "<!-- end of backgroundtable -->\n";

		reply << "\n<!-- and now the navigation -->\n";
		RenderNavigation(reply, c, config);

		reply << "<!-- hidden fields for editable box -->\n";
		RenderHiddenFieldsForEdit(reply, c, config);

		reply << "\n<!-- Scripts for [" << strBoxName << "] -->\n";
		RenderScripts(reply, c, config);

		reply << "<!-- END [" << strBoxName << "] -->\n";
	} else {
		SystemLog::Warning("MultifunctionListBoxRenderer::RenderAll: mandatory 'Name' slot is missing in configuration!");
	}
}

void MultifunctionListBoxRenderer::RenderStyleSheet(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderStyleSheet);
	reply << "\n<style type=\"text/css\">\n<!--\n";
	reply << ".SelectBoxHeader {\n";
	reply << "  font-family : \"Courier New\",Courier,monospace;font-size : 9pt;font-weight:bold;color: white;\n";
	reply << "}\n";
	reply << ".SelectBoxSuchen, .SelectBoxFooter {\n";
	reply << "  font-family : \"Courier New\",Courier,monospace;font-size : 9pt;font-weight:normal;color: black;\n";
	reply << "}\n";
	reply << "//-->\n</style>\n";
}

void MultifunctionListBoxRenderer::RenderHeader(std::ostream &reply, Context &c, const ROAnything &config, long &nColumns)
{
	StartTrace(MultifunctionListBoxRenderer.RenderHeader);
	static Renderer *pRenderer = Renderer::FindRenderer("HeaderListRenderer");

	if (pRenderer) {
		ROAnything theList;
		if (config.LookupPath(theList, "ColList")) {
			// column list slots:
			//	{
			//		/Name			Rendererspec, name of the column
			//		/HeaderAlign	Rendererspec, alignment of header, { left, center, right }
			//		/Hide			long value, 0 or 1, default 0
			//		/Width			Rendererspec, width of column
			//		/Filler			Rendererspec, filler to align text in header, default &nbsp;
			//	}
			reply << "\n<!-- lets insert a row for the column header -->\n";
			Anything rendererConfig;
			ROAnything roOptions;
			rendererConfig["ListName"] = theList.DeepClone();
			ROAnything roBgColor;
			String strBgColor, strClass;
			if (config.LookupPath(roBgColor, "Header.BgColor")) {
				strBgColor << " bgcolor='";
				RenderOnString(strBgColor, c, roBgColor);
				strBgColor << "'";
			}
			rendererConfig["ListHeader"] = String("<tr") << strBgColor << ">\n";
			if (config.LookupPath(roOptions, "Header.Options")) {
				rendererConfig["CellOptions"]["Options"] = roOptions.DeepClone();
				Renderer::RenderOnString(strClass, c, roOptions["class"]);
			} else {
				// render default style
				rendererConfig["CellOptions"]["Options"]["class"] = "SelectBoxHeader";
				strClass = "SelectBoxHeader";
			}
			rendererConfig["EntryRenderer"] = "dummy";
			rendererConfig["IndexSlot"] = "HeaderColumnIndex";

			// we fill in the SortIcon Slot
			ROAnything roSortIcons;
			if ( config.LookupPath(roSortIcons, "SortIcon")) {
				rendererConfig["SortIcon"] = roSortIcons.DeepClone();
			}
			if (config.IsDefined("ListName")) {
				String strListName;
				Renderer::RenderOnString(strListName, c, config["ListName"]);
				ROAnything roaPlainList;
				if (strListName.Length() && c.Lookup(strListName, roaPlainList)) {
					rendererConfig["ListDataSize"] = roaPlainList.GetSize();
				}
			}

			TraceAny(rendererConfig, "ListRenderer-Config");

			pRenderer->RenderAll(reply, c, rendererConfig);
			// get the number of columns rendered by the ListRenderer
			// this is needed to set the correct colspan for the SelectBox-Row
			// -> adjust scroller column (+1L)
			nColumns = c.GetTmpStore()["VisibleColumns"].AsLong(0L) + 1L;
			Trace("visible columns:" << nColumns);

			// to simulate scroller column
			reply << "<th class='" << strClass << "'>&nbsp;&nbsp;</th>\n";
			reply << "</tr>\n";
		} else {
			SystemLog::Warning("No ColumnList given");
		}
	}
}

void MultifunctionListBoxRenderer::RenderBoxRow(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns)
{
	StartTrace(MultifunctionListBoxRenderer.RenderBoxRow);
	if (config.IsDefined("ColList")) {
		reply << "<tr";
		ROAnything roBgColor;
		String strBgColor;
		if (config.LookupPath(roBgColor, "SelectBox.BgColor")) {
			strBgColor << " bgcolor='";
			RenderOnString(strBgColor, c, roBgColor);
			strBgColor << "'";
		}
		reply << strBgColor;
		reply << ">";
		{
			reply << "<td colspan=" << nColumns << ">\n";
			{
				ROAnything roOptions;
				Anything anyOptions;
				bool bHasOptions = false;

				reply << "<span";
				if ((bHasOptions = config.LookupPath(roOptions, "SelectBox.Options"))) {
					anyOptions["Options"] = roOptions.DeepClone();
				} else {
					anyOptions["Options"]["class"] = "SelectBoxSuchen";
				}
				PrintOptions3(reply, c, anyOptions);
				reply << ">\n";
				{
					Anything rendererConfig;
					String strBoxName;
					MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);

					rendererConfig["Name"] = strBoxName;
					if (config.IsDefined("Size")) {
						rendererConfig["Size"] = config["Size"].DeepClone();
					}
					if (config.IsDefined("Multiple")) {
						rendererConfig["Multiple"] = config["Multiple"].DeepClone();
					}
					if (config.IsDefined("ListName")) {
						rendererConfig["ListName"] = config["ListName"].DeepClone();
					}
					if (config.IsDefined("ValueRenderer")) {
						rendererConfig["ValueRenderer"] = config["ValueRenderer"].DeepClone();
					}
					if (config.IsDefined("SelectedRenderer")) {
						rendererConfig["SelectedRenderer"] = config["SelectedRenderer"].DeepClone();
					}

					Anything columnRendererConfig;
					columnRendererConfig["ListName"] = config["ColList"].DeepClone();
					columnRendererConfig["EntryStore"] = "ColumnEntryStore";
					columnRendererConfig["IndexSlot"] = "ColumnIndexSlot";
					columnRendererConfig["SlotnameSlot"] = "ColumnSlotnameSlot";
					if (config.IsDefined("ColSep")) {
						columnRendererConfig["EntryFooter"]["ColSep"] = config["ColSep"].DeepClone();
					}
					columnRendererConfig["EFSuppressLast"] = 1L;
					columnRendererConfig["EntryRenderer"] = "dummy";
					rendererConfig["TextRenderer"] = columnRendererConfig;
					rendererConfig["OptionListRenderer"] = "MultiOptionListRenderer";

					// add the options slot
					// we need the OnChange script as soon as we have the box editable
					if ( ( RenderToString(c, config["EditableList"]).AsLong(0L) == 1L ) || config.IsDefined("OnChangeScript") ) {
						rendererConfig["Options"]["OnChange"][0L] = String("OnChange") << strBoxName << "(this)";
					}
					if (bHasOptions) {
						TraceAny(rendererConfig["Options"], "options before");
						for (long i = 0, sz = roOptions.GetSize(); i < sz; ++i) {
							Trace("appending at [" << roOptions.SlotName(i) << "]");
							TraceAny(roOptions[i], "content:");
							rendererConfig["Options"][roOptions.SlotName(i)] = roOptions[i].DeepClone();
						}
						TraceAny(rendererConfig["Options"], "options after");
					} else {
						rendererConfig["Options"]["class"] = "SelectBoxSuchen";
					}
					TraceAny(rendererConfig, "SelectBoxRenderer-Config");
					SelectBoxRenderer::RenderAll(reply, c, rendererConfig);
				}
				reply << "</span>\n";
			}
			reply << "</td>";
		}
		reply << "</tr>\n";
	}
}

void MultifunctionListBoxRenderer::RenderColumnInputFields(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns)
{
	StartTrace(MultifunctionListBoxRenderer.RenderColumnInputFields);
	static Renderer *pRenderer = Renderer::FindRenderer("ColumnInputFieldRenderer");

	if (pRenderer) {
		TraceAny(config, "config");
		Anything theList;
		if (config.IsDefined("ColList") && Lookup(config["ColList"], c, config, theList)) {
			// check if editable fields should be rendered
			bool bExist = false;
			for (long lx = 0L, sz = theList.GetSize(); lx < sz; ++lx) {
				bool bHidden = MultifunctionListBoxRenderer::IsHiddenField(c, theList[lx]);
				bool bEditable = MultifunctionListBoxRenderer::IsEditableField(c, theList[lx]);
				bool bPulldown =  MultifunctionListBoxRenderer::IsPulldownField(c, theList[lx]);
				TraceAny(theList[lx], "checking:");
				Trace("hidden:" << (bHidden ? "true" : "false") << " editable:" << (bEditable ? "true" : "false")   << " pulldown:" << (bPulldown ? "true" : "false"));
				if ( !bHidden && ( bEditable || bPulldown) ) {
					Trace("found an editable item");
					bExist = true;
					break;
				}
			}
			if (bExist) {
				Anything rendererConfig;
				rendererConfig["ListName"] = config["ColList"].DeepClone();
				rendererConfig["IndexSlot"] = "ColumnIndex";
				rendererConfig["EntryStore"] = "Column";
				rendererConfig["SlotnameSlot"] = "SubColumnSlotnameSlot";
				ROAnything roOptions;
				if (config.LookupPath(roOptions, "EditLine.Options")) {
					rendererConfig["CellOptions"] = roOptions.DeepClone();
				}
				if (config.LookupPath(roOptions, "EditLine.OptionsNotChangeable")) {
					rendererConfig["CellInputOptions"] = roOptions.DeepClone();
				}
				ROAnything roBgColor;
				String strBgColor;

				if (config.LookupPath(roBgColor, "EditLine.BgColor")) {
					strBgColor << " bgcolor='";
					RenderOnString(strBgColor, c, roBgColor);
					strBgColor << "'";
				}
				rendererConfig["ListHeader"] = String("<tr") << strBgColor << ">\n";
				// to simulate scroller column
				rendererConfig["ListFooter"] = "<td>&nbsp;&nbsp;</td>\n</tr>\n";
				rendererConfig["EntryRenderer"] = "dummy";

				TraceAny(rendererConfig, "ColumnInputFieldRenderer-Config");
				pRenderer->RenderAll(reply, c, rendererConfig);
			}
		} else {
			SystemLog::Warning("No ColumnList given");
		}
	}
}

void MultifunctionListBoxRenderer::RenderAdditionalFilters(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns)
{
	StartTrace(MultifunctionListBoxRenderer.RenderAdditionalFilters);

	ROAnything filterConfig;
	if (config.LookupPath(filterConfig, "AdditionalFilters")) {
		reply << "<tr><td colspan=" << nColumns << ">";
		Render(reply, c, filterConfig);
		reply << "</td></tr>\n";
	}
}

bool MultifunctionListBoxRenderer::RenderPrintButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderPrintButton);
	bool bRet = false;
	String strBoxName, strButtonName, strOnClick, strFormName;

	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	strButtonName << strBoxName << "_Print";
	strOnClick << strBoxName << "_DoPrintNow(this.form);";

	ROAnything roButton = navConfig["PrintButton"];
	if (!roButton.IsNull()) {
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "Print";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			if (roButton.IsDefined("Options")) {
				// override default
				buttonConfig["Options"] = roButton["Options"].DeepClone();
			} else {
				buttonConfig["Options"]["OnClick"] = strOnClick << "return false;";

			}
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderExportButton(std::ostream &reply, Context &ctx, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderExportButton);
	bool bRet = false;
	String strBoxName, strButtonName, strOnClick, strFormName;

	TraceAny(config, "config");
	TraceAny(navConfig, "NavConfig");

	MultifunctionListBoxRenderer::GetBoxName(strBoxName, ctx);
	MultifunctionListBoxRenderer::GetFormName(strFormName, ctx);
	strButtonName << strBoxName << "_Export";

	strOnClick << strBoxName << "_DoExportNow(this.form);";

	ROAnything roButton = navConfig["ExportButton"];
	if (!roButton.IsNull()) {
		ROAnything actionConfig, paramsConfig;
		bool isButtonLinked = ( (roButton.LookupPath(actionConfig, "Action")) && (roButton.LookupPath(paramsConfig, "Params")) );
		Trace("Button is a " << ((isButtonLinked) ? "Linked button" : "Normal Button"));
		Renderer *pRenderer = ((isButtonLinked) ? Renderer::FindRenderer("DownloadLinkRenderer") : Renderer::FindRenderer("ButtonRenderer"));
		if (pRenderer) {
			Anything buttonConfig, buttonStyleConfig;
			String strAction, strTableDataBeginTag, strEndParagraphTag, strButtonStyle, strButtonTableLinkStyle;
			strButtonStyle = "FormButton";
			if (isButtonLinked) {
				// config Button with download-link
				Renderer::RenderOnString(strAction, ctx, actionConfig);
				buttonConfig["Action"] = strAction;
				buttonConfig["Params"] = paramsConfig.DeepClone();
				buttonConfig["DownloadLink"] = String(strBoxName) << ".csv";
				if (roButton.IsDefined("DownloadLink")) {
					buttonConfig["DownloadLink"] = roButton["DownloadLink"].DeepClone();
				}
				strButtonStyle = "linkButtonLabel";
				strButtonTableLinkStyle = "linkButton";
				buttonStyleConfig = roButton["Options"].DeepClone();
				if (buttonStyleConfig.IsDefined("class")) {
					strButtonStyle = buttonStyleConfig["class"].AsString("linkButtonLabel");
				}
				if (buttonStyleConfig.IsDefined("classTable")) {
					strButtonTableLinkStyle = buttonStyleConfig["classTable"].AsString("linkButton");
				}
				strTableDataBeginTag = "<td align=center class=";
				strTableDataBeginTag << strButtonStyle << ">";
				strTableDataBeginTag << "<p class=" << strButtonTableLinkStyle << ">";
				strEndParagraphTag = "</p>";
			} else {
				// specific config simple JS-Button
				if (roButton.IsDefined("Options")) {
					// override default
					buttonConfig["Options"] = roButton["Options"].DeepClone();
				} else {
					buttonConfig["Options"]["OnClick"] = strOnClick << "return false;";
				}
				strTableDataBeginTag = "<td align=center>";
				strEndParagraphTag = "";
			}
			// config for both
			buttonConfig["Options"]["class"] = strButtonStyle;
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "CSV-Export";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			reply << strTableDataBeginTag;
			pRenderer->RenderAll(reply, ctx, buttonConfig);
			reply << strEndParagraphTag;
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		ctx.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderPrevButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderPrevButton);
	bool bRet = false;
	String strBoxName, strButtonName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	strButtonName << strBoxName << "_Prev";
	ROAnything roButton = navConfig["PrevButton"];
	if (!roButton.IsNull()) {
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "  <  ";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << "</td>\n";
			bRet = true;
		}
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderNextButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderNextButton);
	bool bRet = false;
	String strBoxName, strButtonName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	strButtonName << strBoxName << "_Next";
	ROAnything roButton = navConfig["NextButton"];
	if (!roButton.IsNull()) {
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "  >  ";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << "</td>\n";
			bRet = true;
		}
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderSearchButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderSearchButton);
	bool bRet = false;
	String strBoxName, strButtonName, strOnClick, strFormName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	strButtonName << strBoxName << "_Search";
	strOnClick << strBoxName << "_ProcessEditFieldScripts(" << "document.forms[" << strFormName << "])";
	ROAnything roButton = navConfig["SearchButton"];
	if (!roButton.IsNull()) {
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "Suchen";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			buttonConfig["Options"]["OnClick"] = String("return ") << strOnClick << ";";
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderClearButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderClearButton);
	bool bRet = false;
	String strBoxName, strButtonName, strOnClick, strFormName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	strButtonName << strBoxName << "_Clear";
	strOnClick << strBoxName << "_ClearEditFields(" << "document.forms[" << strFormName << "])";
	ROAnything roButton = navConfig["ClearButton"];
	if (!roButton.IsNull()) {
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "Clear";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			buttonConfig["Options"]["OnClick"] = String("return ") << strOnClick << ";";
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderSaveButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderSaveButton);
	bool bRet = false;
	String strBoxName, strButtonName, strFormName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	strButtonName << strBoxName << "_Save";

	bool bBoxEditable = (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L);
	bool bLineMode = (config["EditableMode"].AsString("Field") == "Line");
	bool bHasNavigation = !navConfig.IsNull();

	ROAnything roButton = navConfig["SaveButton"];
	bool bDoButton = (bHasNavigation && bBoxEditable && !roButton.IsNull());

	String strProps = strBoxName, strFormPath;
	strProps << "Properties";

	String strFunction(1024L), strOnClick;
	StringStream saveFunction(strFunction);

	if (bDoButton) {
		// if we are a form button we can use the secure way to get a reference to the parent form
		strFormPath = "buttonObj.form";
	} else {
		// if the script could be used from elsewhere we have to specify the full path to the form
		// strFormPath = String("document.forms['") << strFormName << "']";
		if (!config.IsDefined("TagStyle")) {
			// default MSIE-Style
			strFormPath = String("document.forms[") << strFormName << "]";
		} else {
			// Netscape-Style
			strFormPath = String("document.forms['") << strFormName << "']";
		}
	}

	saveFunction << ENDL << "<script language=\"JavaScript1.2\" type=\"text/javascript\">" << ENDL;
	saveFunction << "function " << strBoxName << "_SaveButtonClicked(buttonObj)" << ENDL;
	saveFunction << "{" << ENDL;
	saveFunction << "	var bIsModified = " << strProps << ".IsModified();" << ENDL;

	if (bDoButton && roButton.IsDefined("PreDoScript")) {
		Render(saveFunction, c, roButton["PreDoScript"]);
	}

	saveFunction << "	if (bIsModified) {" << ENDL;
	if (bLineMode) {
		saveFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Added.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetAddedRows());" << ENDL;
		saveFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Changed.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetChangedRows());" << ENDL;
		saveFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Deleted.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetDeletedRows());" << ENDL;
	} else {
		saveFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Added.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetAddedFields());" << ENDL;
		saveFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Changed.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetChangedFields());" << ENDL;
		saveFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Deleted.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetDeletedFields());" << ENDL;
	}
	// krebs: kein clear beim saven! saveFunction << "		" << strBoxName << "_ClearEditFields(" << strFormPath << ");" << ENDL;
	saveFunction << "	};" << ENDL;
	saveFunction << "	return bIsModified;" << ENDL;
	saveFunction << "};" << ENDL;
	saveFunction << "</script>" << ENDL;
	saveFunction << std::flush;

	if (bDoButton) {
		strOnClick << "return " << strBoxName << "_SaveButtonClicked(this);";

		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "Speichern";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			buttonConfig["Options"]["OnClick"] = strOnClick;
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << strFunction;
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		reply << strFunction;
		strOnClick << strBoxName << "_SaveButtonClicked(this);";
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderResetButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderResetButton);
	bool bRet = false;
	String strBoxName, strButtonName, strFormName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	strButtonName << strBoxName << "_Reset";

	bool bBoxEditable = (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L);
	bool bHasNavigation = !navConfig.IsNull();

	String strProps = strBoxName;
	strProps << "Properties";

	String strOnClick;
	strOnClick << "ResetDataAndRows(" << strProps << ", document.forms[" << strFormName << "].elements['fld_" << strBoxName << "']);";
	strOnClick << strBoxName << "_ClearEditFields(" << "document.forms[" << strFormName << "]);";

	ROAnything roButton = navConfig["ResetButton"];
	if (bHasNavigation && bBoxEditable && !roButton.IsNull()) {
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "�nderungen zur�cksetzen";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			// do not submit the form; return false
			buttonConfig["Options"]["OnClick"] = strOnClick << "return false;";
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderAddButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderAddButton);
	bool bRet = false;
	String strBoxName, strButtonName, strFormName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	strButtonName << strBoxName << "_Add";

	bool bBoxEditable = (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L);
	bool bLineMode = (config["EditableMode"].AsString("Field") == "Line");
	bool bHasNavigation = !navConfig.IsNull();

	ROAnything roButton = navConfig["AddButton"];
	bool bDoButton = (bHasNavigation && bBoxEditable && !roButton.IsNull());
	String strProps = strBoxName, strFormPath;
	strProps << "Properties";

	String strFunction(1024L), strOnClick, strSpecifiedOnClick;
	StringStream addFunction(strFunction);

	if (bDoButton) { // if rendering this button is needed
		// if we are a form button we can use the secure way to get a reference to the parent form
		strFormPath = "buttonObj.form";
	} else {
		// if the script could be used from elsewhere we have to specify the full path to the form
		// strFormPath = String("document.forms['") << strFormName << "']";
		if (!config.IsDefined("TagStyle")) {
			// default MSIE-Style
			strFormPath = String("document.forms[") << strFormName << "]";
		} else {
			// Netscape-Style
			strFormPath = String("document.forms['") << strFormName << "']";
		}
	}
	addFunction << ENDL << "<script language=\"JavaScript1.2\" type=\"text/javascript\">" << ENDL;
	addFunction << "function " << strBoxName << "_AddButtonClicked(buttonObj)" << ENDL;
	addFunction << "{" << ENDL;
	addFunction << "	var bIsAdded = " << strProps << ".IsAdded();" << ENDL;
	addFunction << "	var bIsModified = " << strProps << ".IsModified();" << ENDL;

	if (bDoButton && roButton.IsDefined("PreDoScript")) {
		Render(addFunction, c, roButton["PreDoScript"]);
	}

	addFunction << "    if (bIsAdded)" << ENDL;
	addFunction << "    {" << ENDL;
	if (bLineMode) { // if edit a complete line is desired
		addFunction << "    " << strFormPath << ".fld_edt" << strBoxName << "_Added.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetAddedRows());" << ENDL;
		addFunction << "    " << strFormPath << ".fld_edt" << strBoxName << "_Changed.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetChangedRows());" << ENDL;
		addFunction << "    " << strFormPath << ".fld_edt" << strBoxName << "_Deleted.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetDeletedRows());" << ENDL;
	} else {
		addFunction << "    " << strFormPath << ".fld_edt" << strBoxName << "_Added.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetAddedFields());" << ENDL;
		addFunction << "    " << strFormPath << ".fld_edt" << strBoxName << "_Changed.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetChangedFields());" << ENDL;
		addFunction << "    " << strFormPath << ".fld_edt" << strBoxName << "_Deleted.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetDeletedFields());" << ENDL;
	}
	addFunction << "    }" << ENDL;
	if (bDoButton) { // if rendering this button is needed
		if (roButton.IsDefined("Options")) {
			// no submit in any case is desired. submit of the current form is handled by an optional javascript
			ROAnything optionsConfig = roButton["Options"];
			TraceAny(optionsConfig, "options of AddButton");
			if (optionsConfig.IsDefined("OnClick")) {
				RenderOnString(strOnClick, c, optionsConfig["OnClick"]);
				Trace("specified strOnClick is : " << strOnClick);
				addFunction << "	return false;" << ENDL;
			}
		} else {
			addFunction << "    return bIsAdded;" << ENDL;
		}
	}
	addFunction << "};" << ENDL;
	addFunction << "</script>" << ENDL;
	addFunction << std::flush;

	if (bHasNavigation && bBoxEditable && !roButton.IsNull()) {
		strOnClick << "return " << strBoxName << "_AddButtonClicked(this);";
		Trace("common strOnClick is : " << strOnClick);
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "Hinzuf�gen";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			buttonConfig["Options"]["OnClick"] = strOnClick;
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << strFunction;
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		reply << strFunction;
		strOnClick << "return " << strBoxName << "_AddButtonClicked(this);";
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

bool MultifunctionListBoxRenderer::RenderDeleteButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderDeleteButton);
	bool bRet = false;
	String strBoxName, strButtonName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	strButtonName << strBoxName << "_Delete";

	bool bBoxEditable = (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L);
	bool bHasNavigation = !navConfig.IsNull();

	String strProps = strBoxName;
	strProps << "Properties";

	ROAnything roButton = navConfig["DeleteButton"];

	bool bLineMode = (config["EditableMode"].AsString("Field") == "Line");
	bool bDoButton = (bHasNavigation && bBoxEditable && !roButton.IsNull());
	String strFormName, strFormPath;
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);

	String strFunction(1024L), strOnClick;
	StringStream deleteFunction(strFunction);

	if (bDoButton) {
		// if we are a form button we can use the secure way to get a reference to the parent form
		strFormPath = "buttonObj.form";
	} else {
		// if the script could be used from elsewhere we have to specify the full path to the form
		// strFormPath = String("document.forms['") << strFormName << "']";
		if (!config.IsDefined("TagStyle")) {
			// default MSIE-Style
			strFormPath = String("document.forms[") << strFormName << "]";
		} else {
			// Netscape-Style
			strFormPath = String("document.forms['") << strFormName << "']";
		}
	}

	deleteFunction << ENDL << "<script language=\"JavaScript1.2\" type=\"text/javascript\">" << ENDL;
	deleteFunction << "function " << strBoxName << "_DeleteButtonClicked(buttonObj)" << ENDL;
	deleteFunction << "{" << ENDL;
	deleteFunction << "	listbox=" << strFormPath << ".elements['fld_" << strBoxName << "'];" << ENDL;
	deleteFunction << "	row=listbox.selectedIndex;" << ENDL;
	deleteFunction << "	var bDoDelete = ((row>=0) && confirm('Wollen Sie die selektierte Zeile wirklich löschen?') );" << ENDL;

	if (bDoButton && roButton.IsDefined("PreDoScript")) {
		Render(deleteFunction, c, roButton["PreDoScript"]);
	}

	deleteFunction << "	if ( bDoDelete ) {" << ENDL;
	deleteFunction << "		" << strProps << ".DeleteRow(row, listbox);" << ENDL;
	if (bLineMode) {
		deleteFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Added.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetAddedRows());" << ENDL;
		deleteFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Changed.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetChangedRows());" << ENDL;
		deleteFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Deleted.value = MakeAnyFromRows(" << strProps << ", " << strProps << ".Changes.GetDeletedRows());" << ENDL;
	} else {
		deleteFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Added.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetAddedFields());" << ENDL;
		deleteFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Changed.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetChangedFields());" << ENDL;
		deleteFunction << "		" << strFormPath << ".fld_edt" << strBoxName << "_Deleted.value = MakeAnyFromFields(" << strProps << ", " << strProps << ".Changes.GetDeletedFields());" << ENDL;
	}
	deleteFunction << "		" << strBoxName << "_ClearEditFields(" << strFormPath << ");" << ENDL;
	deleteFunction << "		listbox.selectedIndex = row;" << ENDL;
	deleteFunction << "	};" << ENDL;
	deleteFunction << "	return bDoDelete;" << ENDL;
	deleteFunction << "};" << ENDL;
	deleteFunction << "</script>" << ENDL;
	deleteFunction << std::flush;

	if (bDoButton) {
		strOnClick << "return " << strBoxName << "_DeleteButtonClicked(this);";
		static Renderer *pRenderer = Renderer::FindRenderer("ButtonRenderer");
		if (pRenderer) {
			Anything buttonConfig;
			reply << "<td align=center>";
			buttonConfig["Name"] = strButtonName;
			buttonConfig["Label"] = "Löschen";
			if (roButton.IsDefined("Label")) {
				// override default
				buttonConfig["Label"] = roButton["Label"].DeepClone();
			}
			buttonConfig["Options"]["OnClick"] = strOnClick;
			buttonConfig["Options"]["class"] = "FormButton";
			pRenderer->RenderAll(reply, c, buttonConfig);
			reply << strFunction;
			reply << "</td>\n";
			bRet = true;
		}
	} else {
		reply << strFunction;
		strOnClick << strBoxName << "_DeleteButtonClicked(this);";
		c.GetTmpStore()[String("JS_") << strButtonName] = strOnClick;
	}
	return bRet;
}

void MultifunctionListBoxRenderer::RenderStatusMessage(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns)
{
	StartTrace(MultifunctionListBoxRenderer.RenderStatusMessage);
	ROAnything statusMessageConfig;
	if (config.LookupPath(statusMessageConfig, "StatusMessage")) {
		reply << "<tr><td align=right colspan=" << nColumns << ">";
		Render(reply, c, statusMessageConfig);
		reply << "</td></tr>\n";
	}
}

void MultifunctionListBoxRenderer::RenderNavigation(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderNavigation);

	bool bBoxEditable = (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L);

	ROAnything navigationConfig;
	bool bHasNavigation = (config.LookupPath(navigationConfig, "Navigation"));

	if (bHasNavigation || bBoxEditable) {
		// take order of buttons according to definition in config
		String strButtonName;
		Anything anyAllButtonNames;
		long lIdx = -1;
		anyAllButtonNames[++lIdx] = "PrevButton";
		anyAllButtonNames[++lIdx] = "NextButton";
		anyAllButtonNames[++lIdx] = "SearchButton";
		anyAllButtonNames[++lIdx] = "ClearButton";
		anyAllButtonNames[++lIdx] = "PrintButton";
		anyAllButtonNames[++lIdx] = "SaveButton";
		anyAllButtonNames[++lIdx] = "ResetButton";
		anyAllButtonNames[++lIdx] = "AddButton";
		anyAllButtonNames[++lIdx] = "DeleteButton";
		anyAllButtonNames[++lIdx] = "ExportButton";
		TraceAny(navigationConfig, "navigationConfig");
		// render buttons specified in config
		bool bHasButtonsRendered = false;
		String strButtonBuffer(1024L), strScriptBuffer(1024L);
		{
			OStringStream streamOut(strButtonBuffer);
			for (long szButtons = 0, szb = navigationConfig.GetSize(); szButtons < szb; ++szButtons) {
				strButtonName = navigationConfig.SlotName(szButtons);
				Trace("rendering specified button [" << strButtonName << "]");
				bHasButtonsRendered = ( DoRenderButton(streamOut, c, navigationConfig, config, strButtonName) || bHasButtonsRendered );
				if ((lIdx = anyAllButtonNames.FindValue(strButtonName)) >= 0L) {
					anyAllButtonNames.Remove(lIdx);
				}
			}
		}
		Trace("ButtonsRendered:" << (bHasButtonsRendered ? "yes" : "no") << " length of temporary stream buffer:" << strButtonBuffer.Length());
		TraceAny(anyAllButtonNames, "unused Buttons");
		{
			OStringStream streamOut(strScriptBuffer);
			while (anyAllButtonNames.GetSize() > 0) {
				Trace("force creation of replacement script for unrendered button [" << anyAllButtonNames[0L].AsString() << "]");
				DoRenderButton(streamOut, c, navigationConfig, config, anyAllButtonNames[0L].AsString());
				anyAllButtonNames.Remove(0L);
			}
		}
		Trace("content of stream [" << strScriptBuffer << "]");
		if ( bHasButtonsRendered ) {
			reply << "<table border=" << navigationConfig["BorderWidth"].AsLong(0L);
			ROAnything roBgColor;
			if (navigationConfig.LookupPath(roBgColor, "BgColor")) {
				reply << " bgcolor='";
				Render(reply, c, roBgColor);
				reply << "'";
			}
			reply << " cellpadding=0 cellspacing=1><tr>\n";
			// strBuffer should contain at least one <td>...</td>
			reply << strButtonBuffer;
			reply << "</tr>\n</table>\n";
		}
		reply << strScriptBuffer;
	}
}

bool MultifunctionListBoxRenderer::DoRenderButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config, const String &strButtonName)
{
	StartTrace(MultifunctionListBoxRenderer.DoRenderButton);
	bool bRet = false;
	if (strButtonName == "PrevButton") {
		bRet = RenderPrevButton(reply, c, navConfig, config);
	} else if (strButtonName == "NextButton") {
		bRet = RenderNextButton(reply, c, navConfig, config);
	} else if (strButtonName == "SearchButton") {
		bRet = RenderSearchButton(reply, c, navConfig, config);
	} else if (strButtonName == "ClearButton") {
		bRet = RenderClearButton(reply, c, navConfig, config);
	} else if (strButtonName == "PrintButton") {
		bRet = RenderPrintButton(reply, c, navConfig, config);
	} else if (strButtonName == "SaveButton") {
		bRet = RenderSaveButton(reply, c, navConfig, config);
	} else if (strButtonName == "ResetButton") {
		bRet = RenderResetButton(reply, c, navConfig, config);
	} else if (strButtonName == "AddButton") {
		bRet = RenderAddButton(reply, c, navConfig, config);
	} else if (strButtonName == "DeleteButton") {
		bRet = RenderDeleteButton(reply, c, navConfig, config);
	} else if (strButtonName == "ExportButton") {
		bRet = RenderExportButton(reply, c, navConfig, config);
	}
	return bRet;
}

void MultifunctionListBoxRenderer::RenderScripts(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderScripts);
	TraceAny(config, "config");
	Anything theList;
	if (config.IsDefined("ColList") && Lookup(config["ColList"], c, config, theList)) {
		bool bFoundEditFields = false, bFoundPulldownFields = false, bBoxEditable = false;

		String strClearFields, strProcessFields, strOnChangeFields, strBoxName, strFormName;
		MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
		MultifunctionListBoxRenderer::GetFormName(strFormName, c);
		bBoxEditable = (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L);
		// we need the OnChange script as soon as we have the box editable
		bool bHasOnChangeScript = (bBoxEditable ? bBoxEditable : config.IsDefined("OnChangeScript"));
		//
		bool bHasNavigation = false;
		bool bHasPrintButton = false;
		bool bHasExportButton = false;
		bool bIsAnyDataTypeDefined = false;
		bool bCreatePrintviewOnClientsite = false;
		ROAnything navigationConfig;
		bHasNavigation = (config.LookupPath(navigationConfig, "Navigation"));
		if (bHasNavigation) {
			if (navigationConfig.IsDefined("PrintButton")) {
				bHasPrintButton = true;
				bCreatePrintviewOnClientsite = true;

				ROAnything printbuttonConfig;
				if (navigationConfig.LookupPath(printbuttonConfig, "PrintButton")) {
					TraceAny(printbuttonConfig, "PrintButton-Config");
					if (printbuttonConfig["Options"].IsDefined("OnClick")) {
						bCreatePrintviewOnClientsite = false;
					}
					// String strButtonOptions;
					// RenderOnString(strButtonOptions, c, printbuttonConfig["Options"]);
					// Trace("options : [" << strButtonOptions << "]");
				}
			}
			if (navigationConfig.IsDefined("ExportButton")) {
				bHasExportButton = true;
			}
		}

		// clear selectedindex in the box.
		strClearFields << "    myform.fld_" << strBoxName << ".selectedIndex = -1;\n";

		String strColumnSize, strColumnAlign, strColumnIntName, strColumnKeys, strColumnName, strColumnHidden;
		for (long lx = 0, sz = theList.GetSize(); lx < sz; ++lx) {
			bool bHidden = MultifunctionListBoxRenderer::IsHiddenField(c, theList[lx]);
			bool bEditable = MultifunctionListBoxRenderer::IsEditableField(c, theList[lx]);
			bool bPulldown = MultifunctionListBoxRenderer::IsPulldownField(c, theList[lx]);
			if (!bIsAnyDataTypeDefined) {
				bIsAnyDataTypeDefined = MultifunctionListBoxRenderer::IsDataTypeDefined(c, theList[lx]);
			}
			TraceAny(theList[lx], "checking:");
			Trace("hidden:" << (bHidden ? "true" : "false") << " editable:" << (bEditable ? "true" : "false"));

			String strCellName;
			MultifunctionListBoxRenderer::RenderCellName(strCellName, c, theList[lx]);

			// if box is editable or box should be rendered on Client
			if ((bBoxEditable) || (bHasPrintButton) || (bHasExportButton)) {
				RenderOnString(strColumnSize, c, theList[lx]["Width"]);
				if (theList[lx].IsDefined("Align")) {
					RenderOnString(strColumnAlign, c, theList[lx]["Align"]);
				} else {
					strColumnAlign << "left";
				}
				strColumnIntName << strCellName;
				strColumnKeys << (theList[lx].IsDefined("IsKey") ? "1" : "0");
				strColumnHidden << (bHidden ? "1" : "0");

				if (theList[lx].IsDefined("Name")) {
					RenderOnString(strColumnName, c, theList[lx]["Name"]);
				} else {
					RenderOnString(strColumnName, c, theList[lx]["IntName"]);
				}
				if (lx < theList.GetSize() - 1) {
					strColumnSize << ';';
					strColumnAlign << ';';
					strColumnIntName << ';';
					strColumnKeys << ';';
					strColumnName << ';';
					strColumnHidden << ';';
				}
			}

			if (bEditable) {
				bFoundEditFields = true;
				Trace("found a not hidden and editable item");
				// clear edit field
				strClearFields << "    myform.fld_edt" << strBoxName << "_" << strCellName << ".value = \"\";\n";

				String strValue;
				if (theList[lx].IsDefined("OnSearchScript")) {
					RenderOnString(strValue, c, theList[lx]["OnSearchScript"]);
					strProcessFields << "    if (" << strValue << " == false)\n";
					strProcessFields << "      return false;\n";
				}

				strOnChangeFields << "      document.forms[" << strFormName << "].fld_edt" << strBoxName << "_" << strCellName << ".value = (Column[" << lx << "] == undefined )?\"\":Column[" << lx << "];\n";
			} else if (bPulldown) {
				bFoundPulldownFields = true;
				Trace("found a not hidden pulldown field");

				String strVarName("document.forms[");
				strVarName << strFormName << "].fld_cbo" << strBoxName << "_" << strCellName;
				// if we find a defaultSelected definition we use this index when clearing the fields else -1 (nothing selected)
				strClearFields << "    var " << strCellName << "Idx = -1;\n";
				strClearFields << "    for(var " << strCellName << " = 0; " << strCellName << " < " << strVarName << ".length; " << strCellName << "++)\n";
				strClearFields << "    {\n";
				strClearFields << "      if (" << strVarName << ".options[" << strCellName << "].defaultSelected)\n";
				strClearFields << "      {\n";
				strClearFields << "        " << strCellName << "Idx = " << strCellName << ";\n";
				strClearFields << "        break;\n";
				strClearFields << "      }\n";
				strClearFields << "    }\n";
				strClearFields << "    " << strVarName << ".selectedIndex = " << strCellName << "Idx;\n";

				// compare the text in the column against the .text of the option to find the correct index
				strOnChangeFields << "      for(var " << strCellName << " = 0; " << strCellName << " < " << strVarName << ".length; " << strCellName << "++)\n";
				strOnChangeFields << "      {\n";
				strOnChangeFields << "        if (" << strVarName << ".options[" << strCellName << "].text == Column[" << lx << "])\n";
				strOnChangeFields << "        {\n";
				strOnChangeFields << "          " << strVarName << ".selectedIndex = " << strCellName << ";\n";
				strOnChangeFields << "          break;\n";
				strOnChangeFields << "        }\n";
				strOnChangeFields << "      }\n";
			}
		}

		if (bHasOnChangeScript || bFoundEditFields || bFoundPulldownFields || bHasPrintButton || bHasExportButton) {
			if (bBoxEditable || bHasPrintButton || bHasExportButton) {
				// load scripts for client-side rendering and management of edit functionality
				reply << "\n<script language=\"JavaScript1.2\" type=\"text/javascript\" src=\"";
				ROAnything roPath;
				if (c.Lookup("FilePath", roPath)) {
					Render(reply, c, roPath);
				}
				reply << "ChangeableData.js\"></script>\n";
				reply << "<script language=\"JavaScript1.2\" type=\"text/javascript\" src=\"";
				if (c.Lookup("FilePath", roPath)) {
					Render(reply, c, roPath);
				}
				reply << "OptionBoxScripts.js\"></script>\n";
			}
			// fill in script part now
			reply << "\n<script language=\"JavaScript1.2\" type=\"text/javascript\">\n";
			reply << "  <!-- hide from old browsers\n";

			// OnChange script for SelectBox
			if (bHasOnChangeScript) {
				reply << "  function OnChange" << strBoxName << "(selbox)\n";
				reply << "  {\n";
				reply << "    var indx = selbox.selectedIndex;\n";
				reply << "    var text = selbox[indx].value;\n";
				reply << "    var myform = selbox.form;\n";
				reply << "    Column = text.split(" << strBoxName << "Properties.ValueSep);\n";
				reply << "    var bDoChange = " << (bBoxEditable ? "true" : "false") << ";\n";
				if (config.IsDefined("OnChangeScript")) {
					Render(reply, c, config["OnChangeScript"]);
				}
				reply << "    if (bDoChange)\n";
				reply << "    {\n";
				reply << strOnChangeFields;
				reply << "    }\n";
				reply << "    return true;\n";
				reply << "  }\n\n";
			}

			// common part for edit and pulldown fields
			if (bFoundEditFields || bFoundPulldownFields) {
				reply << "  function " << strBoxName << "_ClearEditFields(myform)\n";
				reply << "  {\n";
				reply << strClearFields;
				reply << "    return false; /* do not submit the form*/\n";
				reply << "  }\n";

				reply << "  function " << strBoxName << "_ProcessEditFieldScripts(myform)\n";
				reply << "  {\n";
				reply << strProcessFields;
				reply << "    return true;\n";
				reply << "  }\n\n";
			}
			if (bFoundEditFields) {
				// common for all editfields, use OnChange in Edit-Spec to override
				// �A730/MIZ/29.08.02
				reply << "  function " << strBoxName << "_OnChangeEditField(field, column, changeable, datatype)\n";
				reply << "  {\n";
				reply << "    listbox=field.form.elements[\"fld_" << strBoxName << "\"];\n";
				reply << "    for (var i = 0; i < listbox.length; i++) \n";
				reply << "    {";
				reply << "        if (listbox.options[i].selected == true)\n";
				reply << "        { \n";

				reply << "            row = listbox.options[i].index;\n";
				reply << "            var bDoChange = " << (bBoxEditable ? "true" : "false") << ";\n";

				reply << "            if (bDoChange == true && changeable == 0) \n";
				reply << "            { \n";
				reply << "    	          bDoChange = false; \n";
				reply << "            } \n";

				reply << "            if ((bDoChange == true && changeable == 1) && (row == -1 || listbox.options[row].value.length == 0)) \n";
				reply << "            { \n";
				reply << "    	          bDoChange = false; \n";
				reply << "            } \n";

				if (config.IsDefined("OnChangeEditFieldScript")) {
					Render(reply, c, config["OnChangeEditFieldScript"]);
				}
				reply << "            if (bDoChange)\n";
				reply << "            {\n";

				String strOnChangeValidation;
				strOnChangeValidation << "                ChangeField( " << strBoxName << "Properties, row, column, field.value );\n";
				strOnChangeValidation << "                UpdateField( " << strBoxName << "Properties, row, column, field.value, listbox);\n";
				reply << strOnChangeValidation;
				reply << "            }\n";

				reply << "            listbox.options[i].selected = true;\n";
				reply << "        }\n";
				reply << "    }\n";
				reply << "    return true;\n";
				reply << "  }\n\n";
			}
			if (bFoundPulldownFields) {
				// common for all pulldownmenus, use OnChange in Pulldown-Spec to override
				//�A730/MIZ/30.08.02
				reply << "  function " << strBoxName << "_OnChangePulldown(field, column, changeable)\n";
				reply << "  {\n";
				reply << "    listbox=field.form.elements[\"fld_" << strBoxName << "\"];\n";
				reply << "    for (var i = 0; i < listbox.length; i++) \n";
				reply << "    {";
				reply << "        if (listbox.options[i].selected == true)\n";
				reply << "        { \n";
				reply << "            row = listbox.options[i].index;\n";
				reply << "            selectedPulldown=field.selectedIndex;\n";

				reply << "            var bDoChange = " << (bBoxEditable ? "true" : "false") << ";\n";
				reply << "            if (bDoChange == true && changeable == 0) \n";
				reply << "            { \n";
				reply << "                bDoChange = false; \n";
				reply << "            } \n";

				reply << "            if ((bDoChange == true && changeable == 1) && (row == -1 || listbox.options[row].value.length == 0)) \n";
				reply << "            { \n";
				reply << "                bDoChange = false; \n";
				reply << "            } \n";

				if (config.IsDefined("OnChangePulldownScript")) {
					Render(reply, c, config["OnChangePulldownScript"]);
				}
				reply << "            if (bDoChange)\n";
				reply << "            {\n";
				reply << "                ChangeField( " << strBoxName << "Properties, row, column, field.options[selectedPulldown].value );\n";
				reply << "                UpdateField( " << strBoxName << "Properties, row, column, field.options[selectedPulldown].value, listbox);\n";
				reply << "            }\n";
				reply << "            listbox.options[i].selected = true;\n";
				reply << "        }\n";
				reply << "    }\n";
				reply << "    return true;\n";
				reply << "  }\n\n";
			}

			if ((bBoxEditable) || (bHasPrintButton) || (bHasExportButton)) {
				// create box-variable for storing box properties
				reply << "  // create property variable for [" << strBoxName << "]\n";
				reply << "  var " << strBoxName << "Properties = new BoxProperties();\n";
				reply << "  // now fill in the boxes data\n";

				// render the print title
				ROAnything roaTitleSpec;
				reply << "  " << strBoxName << "Properties.SetPrintTitle(\"";
				if (!config.LookupPath(roaTitleSpec, "PrintTitle")) {
					c.Lookup("PageTitle", roaTitleSpec);
				}
				Render(reply, c, roaTitleSpec);
				reply << "\");\n";

				static Renderer *pListRenderer = FindRenderer("ListRenderer");
				if (pListRenderer) {
					Anything anyListSpec;
					anyListSpec["ListName"] = config["ListName"].DeepClone();
					anyListSpec["EntryStore"] = "SelectBoxOption";
					anyListSpec["IndexSlot"] = "SelectBoxOptionIndex";

					// render the value separator info
					reply << "  " << strBoxName << "Properties.SetValueSep(\"";
					if (config.IsDefined("ClientSideValSep")) {
						Render(reply, c, config["ClientSideValSep"]);
					} else {
						reply << ";";
					}
					reply << "\");\n";

					// render the value rows
					if (config.IsDefined("ValueRenderer")) {
						anyListSpec["EntryHeader"] = String("  ") << strBoxName << "Properties.AddRow(\"";
						anyListSpec["EntryRenderer"] = config["ValueRenderer"].DeepClone();
						anyListSpec["EntryFooter"] = "\");\n";
						TraceAny(anyListSpec, "AddRow renderer specification");
						pListRenderer->RenderAll(reply, c, anyListSpec);
					}
					// set column size, align and IntName info
					reply << "  " << strBoxName << "Properties.SetColumns(\"" << strColumnSize << "\"";
					reply << ", \"" << strColumnAlign << "\"";
					reply << ", \"" << strColumnIntName << "\"";
					reply << ", \"" << strColumnKeys << "\"";
					reply << ", \"" << strColumnHidden << "\"";
					reply << ", \"" << strColumnName << "\");\n";
					// render the column separator info
					reply << "  " << strBoxName << "Properties.SetColumnSep(\"";
					if (config.IsDefined("ClientSideColSep")) {
						Render(reply, c, config["ClientSideColSep"]);
					} else {
						reply << "| ";
					}
					reply << "\");\n";
				}
			}
			if (bCreatePrintviewOnClientsite) {
				RenderPrintScripts(reply, c, config);
			}
			reply << "  //-->\n";
			reply << "</script>\n";
		}
	}
}

void MultifunctionListBoxRenderer::RenderPrintScripts(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderPrintScripts);

	String strBoxName, strFormName;
	MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
	MultifunctionListBoxRenderer::GetFormName(strFormName, c);
	//
	reply << "\nfunction " << strBoxName << "_DoPrintNow(myform)\n";
	reply << "{\n";
	String strPrintWindowHeader;
	strPrintWindowHeader << "	var winCommOpts = 'locationbar=no,menubar=yes,status=yes,scrollbars=yes,resizable=yes';\n";
	strPrintWindowHeader << "	winCommOpts = winCommOpts+',screenX='+screen.availLeft+',screenY='+screen.availTop;\n";
	strPrintWindowHeader << "	/* open window for printing */\n";
	strPrintWindowHeader << "	var winOptions = winCommOpts;\n";
	strPrintWindowHeader << "	var winPrintPreview = window.open('','Print', winOptions);\n";
	strPrintWindowHeader << "	winPrintPreview.document.open('text/html');\n";
	strPrintWindowHeader << "	winPrintPreview.document.writeln('<HTML><HEAD><TITLE>Drucken...<\\/TITLE><\\/HEAD>');\n";
	strPrintWindowHeader << "	winPrintPreview.document.writeln('<BODY>');\n";
	reply << strPrintWindowHeader;
	//
	if (config.IsDefined("PrintTableStyle")) {
		Render(reply, c, config["PrintTableStyle"]);
	} else {
		reply << "  RenderPrintTableStyle(winPrintPreview.document);\n";
	}
	// print title
	reply << "  RenderPrintTitle(" << strBoxName << "Properties, winPrintPreview.document);\n";
	// print header with db and print date
	Anything theList;
	if (config.IsDefined("PrintHeaderList") && Lookup(config["PrintHeaderList"], c, config, theList) ) {
		TraceAny(theList, "theList");
		reply << "	winPrintPreview.document.writeln('<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" bgcolor=\"#000000\"><tr><td>');\n";
		reply << "	winPrintPreview.document.writeln('<table border=\"0\" cellspacing=\"1\" cellpadding=\"3\" bgcolor=\"#FFFF99\" class=\"ListingHeaderBorder\"><tr>');\n";
		for (long lx = 0, sz = theList.GetSize(); lx < sz; ++lx) {
			String strName;
			RenderOnString(strName, c, theList[lx]["Name"]);
			String strClass = (0 == lx) ? "\"ListingHeaderTableCellName\"" : "\"ListingHeaderTableCellNameLR\"";
			reply << "	winPrintPreview.document.writeln('   <td class=" << strClass << ">" << strName  << "<\\/td>');\n";
			String strDesc;
			RenderOnString(strDesc, c, theList[lx]["Desc"]);
			reply << "	winPrintPreview.document.writeln('   <td class=\"ListingHeaderTableDetail\">" << strDesc  << "<\\/td>');\n";
		}
		reply << "	winPrintPreview.document.writeln('<\\/tr><\\/table>');\n";
		reply << "	winPrintPreview.document.writeln('<\\/td><\\/tr><\\/table>');\n";
	}

	reply << "	winPrintPreview.document.writeln('<TABLE BORDER=1>');\n";
	if (config.IsDefined("PrintTableRows")) {
		Render(reply, c, config["PrintTableRows"]);
	} else {
		reply << "  RenderPrintTableRows(" << strBoxName << "Properties, winPrintPreview.document);\n";
	}
	// end printingtable
	reply << "	winPrintPreview.document.writeln('<\\/TABLE>');\n";

	// render navigation
	String strPrintWindowNavigation;
	strPrintWindowNavigation << "	winPrintPreview.document.writeln('<FORM ACTION=\"\" METHOD=\"POST\" Name=\"" << strBoxName << "_PrintForm\">');\n";
	strPrintWindowNavigation << "	winPrintPreview.document.writeln('<INPUT TYPE=\"HIDDEN\" NAME=\"fld_dummy\" VALUE=\"\">');\n";
	strPrintWindowNavigation << "	winPrintPreview.document.writeln('<INPUT TYPE=\"SUBMIT\" NAME=\"" << strBoxName << "_Print\" VALUE=\"Drucken\" OnClick=\"print();return false;\" class=\"FormButton\">');\n";
	strPrintWindowNavigation << "	winPrintPreview.document.writeln('<INPUT TYPE=\"BUTTON\" VALUE=\"Schliessen\" OnClick=\"self.close();return false;\" class=\"FormButton\">');\n";
	strPrintWindowNavigation << "	winPrintPreview.document.writeln('<\\/FORM>');\n";
	reply << strPrintWindowNavigation;

	reply << "	winPrintPreview.document.writeln('<\\/BODY>');\n";
	reply << "	winPrintPreview.document.writeln('<\\/HTML>');\n";
	reply << "	winPrintPreview.document.close();\n";
	reply << "}\n";
}

void MultifunctionListBoxRenderer::RenderHiddenFieldsForEdit(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultifunctionListBoxRenderer.RenderHiddenFieldsForEdit);

	if (RenderToString(c, config["EditableList"]).AsLong(0L) == 1L) {
		// add fields to submit added, deleted or changed fields
		static Renderer *pRenderer = FindRenderer("HiddenFieldRenderer");
		if (pRenderer) {
			Anything anyHidden;
			String strBoxName;
			MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
			anyHidden["Name"] = String("edt") << strBoxName << "_Added";
			pRenderer->RenderAll(reply, c, anyHidden);
			reply << "\n";
			anyHidden["Name"] = String("edt") << strBoxName << "_Changed";
			pRenderer->RenderAll(reply, c, anyHidden);
			reply << "\n";
			anyHidden["Name"] = String("edt") << strBoxName << "_Deleted";
			pRenderer->RenderAll(reply, c, anyHidden);
			reply << "\n";
		}
	}
}

//---- MultiOptionListRenderer -----------------------------------------------------------
class MultiOptionListRenderer : public OptionListRenderer
{
public:
	MultiOptionListRenderer(const char *name) : OptionListRenderer(name) {};
	~MultiOptionListRenderer() {};

protected:
	virtual void RenderText(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &textConfig, const ROAnything &listItem);
};

RegisterRenderer(MultiOptionListRenderer);

void MultiOptionListRenderer::RenderText(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &textConfig, const ROAnything &listItem)
{
	StartTrace(MultiOptionListRenderer.RenderText);
	static Renderer *pRenderer = Renderer::FindRenderer("MultiColumnRenderer");
	if (pRenderer) {
		TraceAny(textConfig, "MultiColumnRenderer-Config");
		pRenderer->RenderAll(reply, c, textConfig);
	}
}

//---- MultiColumnRenderer -----------------------------------------------------------
class MultiColumnRenderer : public ListRenderer
{
public:
	MultiColumnRenderer(const char *name) : ListRenderer(name) {};
	~MultiColumnRenderer() {};

protected:
	void RenderEntry(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState);
	void RenderEntryFooter(std::ostream &reply, Context &c, const ROAnything &entryFooter, const ROAnything &listItem, Anything &anyRenderState);
};

RegisterRenderer(MultiColumnRenderer);

static void CopyWithDefault(const ROAnything &source, Anything &destination, const char *slot, const String &deflt = "")
{
	ROAnything valueAny;
	if (source.LookupPath(valueAny, slot)) {
		destination[slot] = valueAny.DeepClone();
	} else if (deflt.Length() > 0) {
		destination[slot] = deflt;
	}
}

void MultiColumnRenderer::RenderEntry(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(MultiColumnRenderer.RenderEntry);
	static Renderer *pRenderer = Renderer::FindRenderer("FormattedStringRenderer");

	bool boHide = MultifunctionListBoxRenderer::IsHiddenField(c, listItem);
	if (!boHide) {
		if (pRenderer) {
			ROAnything valueAny;
			if (listItem.LookupPath(valueAny, "Value")) {
				Anything formatConfig;
				formatConfig["Value"] = valueAny.DeepClone();
				CopyWithDefault(listItem, formatConfig, "Align", "left");
				CopyWithDefault(listItem, formatConfig, "Width", "");
				CopyWithDefault(listItem, formatConfig, "Filler", "&nbsp;");
				TraceAny(formatConfig, "formatConfig");
				pRenderer->RenderAll(reply, c, formatConfig);
			}
		}
	}
}

void MultiColumnRenderer::RenderEntryFooter(std::ostream &reply, Context &c, const ROAnything &entryFooter, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(MultiColumnRenderer.RenderEntryFooter);
	bool boHide = MultifunctionListBoxRenderer::IsHiddenField(c, listItem);
	if (!boHide) {
		TraceAny(entryFooter, "entryFooter");
		TraceAny(listItem, "listItem");
		reply << entryFooter["ColSep"].AsString("&nbsp;");
	}
}

//---- ColumnInputFieldRenderer -----------------------------------------------------------
class ColumnInputFieldRenderer : public ListRenderer
{
public:
	ColumnInputFieldRenderer(const char *name) : ListRenderer(name) {};
	~ColumnInputFieldRenderer() {};

protected:
	void RenderEntry(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState);
	void RenderListHeader(std::ostream &reply, Context &c, const ROAnything &listHeader);

private:
	void RenderBgColor(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem);
	void RenderCellOptions(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem);
	void RenderCellBegin(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem);
	void RenderCellEnd(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem);
	void RenderEditField(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem, const long &lColumnIndex);
	void RenderPulldownField(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem, const long &lColumnIndex);
	void AppendAny(const ROAnything &roaSource, Anything &anyDest);
};

RegisterRenderer(ColumnInputFieldRenderer);

void ColumnInputFieldRenderer::RenderListHeader(std::ostream &reply, Context &c, const ROAnything &listHeader)
{
	StartTrace(ColumnInputFieldRenderer.RenderListHeader);
	ListRenderer::RenderListHeader(reply, c, listHeader);
}

void ColumnInputFieldRenderer::RenderBgColor(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem)
{
	StartTrace(ColumnInputFieldRenderer.RenderBgColor);
	ROAnything anyBgColor;
	String strBgColor;
	if (listItem.LookupPath(anyBgColor, "BgColor")) {
		strBgColor << " bgcolor='";
		RenderOnString(strBgColor, c, anyBgColor);
		strBgColor << "'";
	}
	reply << strBgColor;
}

void ColumnInputFieldRenderer::RenderCellOptions(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem)
{
	StartTrace(ColumnInputFieldRenderer.RenderCellOptions);
	ROAnything roOptions;
	Anything anyOptions;
	if (config.LookupPath(roOptions, "CellOptions")) {
		anyOptions["Options"] = roOptions.DeepClone();
	} else {
		anyOptions["Options"]["class"] = "SelectBoxFooter";
	}
	PrintOptions3(reply, c, anyOptions);
}

void ColumnInputFieldRenderer::RenderCellBegin(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem)
{
	StartTrace(ColumnInputFieldRenderer.RenderCellBegin);

	if ( listItem.IsDefined("EditfieldTitle") ) {
		reply << "<td title=\'";
		Render(reply, c, listItem["EditfieldTitle"]);
		reply << "\' align=";
	} else {
		reply << "<td align=";
	}

	if (listItem.IsDefined("EditfieldAlign")) {
		Trace("alignment is [" << listItem["EditfieldAlign"].AsString() << "]");
		Render(reply, c, listItem["EditfieldAlign"]);
	} else if (listItem.IsDefined("Align")) {
		Trace("alignment is [" << listItem["Align"].AsString() << "]");
		Render(reply, c, listItem["Align"]);
	} else {
		Trace("alignment is [left]");
		reply << "left";
	}
	// add BgColor if any
	RenderBgColor(reply, c, config, listItem);
	// render options
	RenderCellOptions(reply, c, config, listItem);
	reply << ">";
	// add also span to be very compatible
	reply << "<span";
	RenderCellOptions(reply, c, config, listItem);
	reply << ">";
}

void ColumnInputFieldRenderer::RenderCellEnd(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem)
{
	StartTrace(ColumnInputFieldRenderer.RenderCellEnd);
	reply << "</span>";
	reply << "</td>\n";
}

void ColumnInputFieldRenderer::RenderEditField(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem, const long &lColumnIndex)
{
	StartTrace(ColumnInputFieldRenderer.RenderEditField);
	static Renderer *pRenderer = Renderer::FindRenderer("TextFieldRenderer");
	if (pRenderer) {
		Anything textFieldConfig;
		String strCellName, strBoxName, strFormName;
		MultifunctionListBoxRenderer::RenderCellName(strCellName, c, listItem);
		MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
		MultifunctionListBoxRenderer::GetFormName(strFormName, c);
		Trace("Formname [" << strFormName << "]");

		textFieldConfig["Name"] = String("edt") << strBoxName << "_" << strCellName;
		textFieldConfig["Size"] = listItem["Width"].DeepClone();
		textFieldConfig["Maxlength"] = listItem["Width"].DeepClone();

		// check if we can re-use the value from the last request as default
		ROAnything roField;
		String strQueryField("query.fields.edt");
		strQueryField << strBoxName << "_" << strCellName;
		Trace("looking up:[" << strQueryField << "]");
		if (c.Lookup(strQueryField, roField)) {
			// field could be looked up in context and is not the empty string
			strQueryField = "";
			RenderOnString(strQueryField, c, roField);
			Trace("Value is:[" << strQueryField << "]");
			textFieldConfig["Value"] = strQueryField;
		} else {
			// field was not found
			Trace("Value not found");
			textFieldConfig["Value"] = Anything();
		}
		// add the options slot
		TraceAny(config, "config is ");
		TraceAny(listItem, "listItem is ");
		bool bChangeable = MultifunctionListBoxRenderer::IsChangeableField(c, listItem);

		textFieldConfig["Options"]["OnChange"][0L] = String(strBoxName) << "_OnChangeEditField(this";
		textFieldConfig["Options"]["OnChange"][1L] = String(", ") << lColumnIndex;

		String strEventHandler;
		if (listItem.IsDefined("DataType")) {
			ROAnything roDataType;
			String strDataType("Float");
			if (listItem.LookupPath(roDataType, "DataType")) {
				// if anything is an array, e.g. when Validate slot is also defined
				// we assume the first entry is the type
				if (roDataType.IsDefined("Type")) {
					strDataType = "";
					RenderOnString(strDataType, c, roDataType["Type"]);
				}
			}

			textFieldConfig["Options"]["OnChange"][2L] = String(", ") << (bChangeable ? 1L : 0L);
			textFieldConfig["Options"]["OnChange"][3L] = String(", '") << strDataType << "')";

			Trace("DataType is:[" << strDataType << "]");

			String strFieldName, strKeyDownHandler;
			strFieldName << "fld_edt" << String(strBoxName) << "_" << strCellName;
			strKeyDownHandler << "	document.forms";
			strKeyDownHandler << "[" << strFormName << "]";
			strKeyDownHandler << "." << strFieldName << ".onkeydown=";

			// render script for register and handle the keydownevent
			strEventHandler << "\n<script language=\"JavaScript1.2\" type=\"text/javascript\">\n";
			if (roDataType.IsDefined("Validate")) {
				strEventHandler << "function Event_" << strFieldName <<  "(e)\n";
				strEventHandler << "{\n";

				strEventHandler << "	debugE('Event_" << strFieldName << "');\n";
				strEventHandler << "	debug('type[' + e.type + '] key[' + e.which + '] target [' + e.target.name + ']');\n";

				strEventHandler << "	retCode = ";

				ROAnything roValidate;
				String strValidate;
				if (roDataType.LookupPath(roValidate, "Validate")) {
					RenderOnString(strValidate, c, roValidate);
				}

				strEventHandler << strValidate << "(e);\n";

				strEventHandler << "	debugL('Event_" << strFieldName << "');\n";
				strEventHandler << "	return retCode;\n";
				strEventHandler << "}\n";

				strKeyDownHandler << "Event_" << strFieldName << ";\n";
			} else {
				strKeyDownHandler << "only" << strDataType << ";\n";
			}

			strEventHandler << strKeyDownHandler;
			strEventHandler << "</script>\n";
		} else {
			textFieldConfig["Options"]["OnChange"][2L] = String(", ") << (bChangeable ? 1L : 0L) << ")";
		}

		ROAnything roOptions;
		if (listItem.LookupPath(roOptions, "Options")) {
			TraceAny(roOptions, "column specific Options");
			AppendAny(roOptions, textFieldConfig["Options"]);
		}

		if (config.LookupPath(roOptions, "CellOptions")) {
			if (!bChangeable && config.LookupPath(roOptions, "CellInputOptions")) {
				AppendAny(roOptions, textFieldConfig["Options"]);
			} else {
				AppendAny(roOptions, textFieldConfig["Options"]);
			}
		} else {
			textFieldConfig["Options"]["class"] = "SelectBoxFooter";
		}

		TraceAny(textFieldConfig, "textFieldConfig");
		pRenderer->RenderAll(reply, c, textFieldConfig);

		if (strEventHandler.Length() > 0) {
			reply << strEventHandler;
		}
	} else {
		Trace("TextFieldRenderer not found:" << (long)pRenderer);
	}
}

void ColumnInputFieldRenderer::AppendAny(const ROAnything &roaSource, Anything &anyDest)
{
	StartTrace(ColumnInputFieldRenderer.AppendAny);
	TraceAny(anyDest, "anything before");
	for (long i = 0, sz = roaSource.GetSize(); i < sz; ++i) {
		Trace("appending at [" << roaSource.SlotName(i) << "]");
		TraceAny(roaSource[i], "content:");
		anyDest[roaSource.SlotName(i)] = roaSource[i].DeepClone();
	}
	TraceAny(anyDest, "anything after");
}

void ColumnInputFieldRenderer::RenderPulldownField(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem, const long &lColumnIndex)
{
	StartTrace(ColumnInputFieldRenderer.RenderPulldownField);
	ROAnything roaPulldown;

	static Renderer *pRenderer = Renderer::FindRenderer("PulldownMenuRenderer");
	if (pRenderer && listItem.LookupPath(roaPulldown, "Pulldown")) {
		TraceAny(roaPulldown, "roaPulldown");
		Anything pulldownConfig;
		String strCellName, strBoxName;
		MultifunctionListBoxRenderer::RenderCellName(strCellName, c, listItem);
		MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
		pulldownConfig["Name"] = String("cbo") << strBoxName << "_" << strCellName;
		if (roaPulldown.IsDefined("ListName")) {
			pulldownConfig["ListName"] = roaPulldown["ListName"].DeepClone();
		}
		if (roaPulldown.IsDefined("PrependListName")) {
			pulldownConfig["PrependListName"] = roaPulldown["PrependListName"].DeepClone();
		}
		if (roaPulldown.IsDefined("AppendListName")) {
			pulldownConfig["AppendListName"] = roaPulldown["AppendListName"].DeepClone();
		}
		if (roaPulldown.IsDefined("TextRenderer")) {
			pulldownConfig["TextRenderer"] = roaPulldown["TextRenderer"].DeepClone();
		}
		if (roaPulldown.IsDefined("ValueRenderer")) {
			pulldownConfig["ValueRenderer"] = roaPulldown["ValueRenderer"].DeepClone();
		}
		if (roaPulldown.IsDefined("SelectedRenderer")) {
			pulldownConfig["SelectedRenderer"] = roaPulldown["SelectedRenderer"].DeepClone();
		}
		// add the options slot
		bool bChangeable = MultifunctionListBoxRenderer::IsChangeableField(c, listItem);

		if (roaPulldown.IsDefined("OnChangeScript")) {
			String strOnChangeScript;
			RenderOnString(strOnChangeScript, c, roaPulldown["OnChangeScript"]);
			pulldownConfig["Options"]["OnChange"] = String("") << strOnChangeScript;
		} else {
			pulldownConfig["Options"]["OnChange"][0L] = String(strBoxName) << "_OnChangePulldown(this";
			pulldownConfig["Options"]["OnChange"][1L] = String(", ") << lColumnIndex;
			pulldownConfig["Options"]["OnChange"][2L] = String(", ") << (bChangeable ? 1L : 0L) << ")";
		}

		ROAnything roOptions;
		if (config.LookupPath(roOptions, "CellOptions")) {
			if (!bChangeable && config.LookupPath(roOptions, "CellInputOptions")) {
				AppendAny(roOptions, pulldownConfig["Options"]);
			} else {
				AppendAny(roOptions, pulldownConfig["Options"]);
			}
		} else {
			pulldownConfig["Options"]["class"] = "SelectBoxFooter";
		}

		TraceAny(pulldownConfig, "pulldownConfig");
		pRenderer->RenderAll(reply, c, pulldownConfig);
	} else {
		Trace("PulldownMenuRenderer not found:" << (long)pRenderer);
	}
}

void ColumnInputFieldRenderer::RenderEntry(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(ColumnInputFieldRenderer.RenderEntry);
	bool boHide = MultifunctionListBoxRenderer::IsHiddenField(c, listItem);
	if (!boHide) {
		long lColumnIndex = c.Lookup("ColumnIndex", 0L);
		TraceAny(config, "config");
		TraceAny(entryRendererConfig, "entryRendererConfig");
		String strBoxName;
		MultifunctionListBoxRenderer::GetBoxName(strBoxName, c);
		TraceAny(listItem, "listItem");

		RenderCellBegin(reply, c, config, listItem);

		if ( MultifunctionListBoxRenderer::IsEditableField(c, listItem) ) {
			RenderEditField(reply, c, config, listItem, lColumnIndex);
		} else if ( MultifunctionListBoxRenderer::IsPulldownField(c, listItem) ) {
			RenderPulldownField(reply, c, config, listItem, lColumnIndex);
		} else {
			reply << "&nbsp;";
		}
		RenderCellEnd(reply, c, config, listItem);
	}
}
