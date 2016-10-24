/***********************************************************************************************************************
 * Copyright (C) 2016 Andrew Zonenberg and contributors                                                                *
 *                                                                                                                     *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the    *
 * following conditions are met:                                                                                       *
 *                                                                                                                     *
 *    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the        *
 *      following disclaimer.                                                                                          *
 *                                                                                                                     *
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the      *
 *      following disclaimer in the documentation and/or other materials provided with the distribution.               *
 *                                                                                                                     *
 *    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products    *
 *      derived from this software without specific prior written permission.                                          *
 *                                                                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL*
 * THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES       *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR      *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT*
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE      *
 * POSSIBILITY OF SUCH DAMAGE.                                                                                         *
 *                                                                                                                     *
 **********************************************************************************************************************/

#include "log.h"
#include <string>
#include <cstdio>
#include <cstdarg>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

FILELogSink::FILELogSink(FILE *f, bool line_buffered, Severity min_severity)
	: m_file(f)
	, m_min_severity(min_severity)
{
	if(line_buffered)
		setvbuf(f, NULL, _IOLBF, 0);
}

FILELogSink::~FILELogSink()
{
	fclose(m_file);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging

void FILELogSink::Log(Severity severity, const std::string &msg)
{
	if(severity > m_min_severity)
		return;

	//Wrap/print it
	string wrapped = WrapString(msg);
	fputs(wrapped.c_str(), m_file);

	//See if we printed a \n
	if(wrapped.length() && (wrapped[wrapped.length() - 1] == '\n'))
		m_lastMessageWasNewline = true;
	else if(wrapped != "")
		m_lastMessageWasNewline = false;
}

void FILELogSink::Log(Severity severity, const char *format, va_list va)
{
	if(severity > m_min_severity)
		return;

	//Wrap/print it
	string wrapped = WrapString(vstrprintf(format, va));
	fputs(wrapped.c_str(), m_file);

	//See if we printed a \n
	if(wrapped.length() && (wrapped[wrapped.length() - 1] == '\n'))
		m_lastMessageWasNewline = true;
	else if(wrapped != "")
		m_lastMessageWasNewline = false;
}
