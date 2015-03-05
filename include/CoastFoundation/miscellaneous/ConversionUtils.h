/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConversionUtils_H
#define _ConversionUtils_H

#include "foundation.h"
#include "ITOString.h"
#include "Tracer.h"
#include <iostream>

namespace ConversionUtils
{
	enum OrderType {
		eLSB2MSB,		// leftmost byte in buffer is LSB of value
		eMSB2LSB		// leftmost byte in buffer is MSB of value
	};

	template<typename valueType>
	inline void GetValueFromBuffer(const unsigned char *pBuf, valueType &lValue, OrderType order = eLSB2MSB)
	{
		StartTrace(ConversionUtils.GetValueFromBuffer);
		size_t lValSize = sizeof(valueType);
		Trace("value size:" <<  static_cast<long>(lValSize));
		lValue = 0;
		for (size_t lIdx = 0; lIdx < lValSize; ++lIdx) {
			switch ( order ) {
				case eMSB2LSB:
					Trace("MSB2LSB mode");
					lValue <<= 8;
					lValue |= (ul_long)pBuf[lIdx];
					break;

				case eLSB2MSB:
					Trace("LSB2MSB mode");
					lValue |= ( (ul_long)pBuf[lIdx] << (lIdx * 8) );
					break;
				default:
					;
			}
		}
	}

	template<typename valueType>
	inline void GetValueFromBuffer(const String &raBuffer, valueType &lValue, long lOffset = 0L, OrderType order = eLSB2MSB)
	{
		StartTrace1(ConversionUtils.GetValueFromBuffer, "Offset:" << lOffset << " order is:" << static_cast<long>(order));
		// ensure the buffer is big enough to prevent a buffer overflow
		const size_t lValSize = sizeof(valueType);
		if ( static_cast<size_t>(raBuffer.Length()) >= ( lOffset + lValSize ) ) {
			const char *pBuf = (const char *)raBuffer;
			Trace("initial address:" << reinterpret_cast<long>(pBuf));
			pBuf += lOffset;
			Trace("address with offset:" << reinterpret_cast<long>(pBuf));
			GetValueFromBuffer((unsigned char *)pBuf, lValue, order);
			Trace("value:" << (l_long)lValue);
		}
	}

	template<typename valueType>
	inline std::istream &GetValueFromStream(std::istream &stream, valueType &lValue, OrderType order = eLSB2MSB)
	{
		const size_t lValSize = sizeof(valueType);
		char pBuf[sizeof(valueType)];
		if ( !stream.read(pBuf, lValSize).eof() ) {
			GetValueFromBuffer((unsigned char *)pBuf, lValue, order);
		}
		return stream;
	}
}

#endif
