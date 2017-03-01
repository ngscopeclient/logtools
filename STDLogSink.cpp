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
#include <cstdio>
#include <cstdarg>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

using namespace std;

extern bool g_logToStdoutAlways;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

STDLogSink::STDLogSink(Severity min_severity)
	: m_min_severity(min_severity)
{
	//Get the current display terminal width
#ifndef _WIN32
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	m_termWidth = w.ws_col;
#else
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	m_termWidth = csbi.dwSize.X;
#endif
}

STDLogSink::~STDLogSink()
{
	Flush();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging

void STDLogSink::Flush()
{
	fflush(stdout);
	fflush(stderr);
}

void STDLogSink::Log(Severity severity, const string &msg)
{
	//Skip messages which aren't important enough
	if(severity > m_min_severity)
		return;

	//Prevent newer messages on stderr from appearing before older messages on stdout
	if(severity <= Severity::WARNING)
		Flush();

	//Wrap/print it
	string wrapped = WrapString(msg);
	if( (severity <= Severity::WARNING) && !g_logToStdoutAlways )
		fputs(wrapped.c_str(), stderr);
	else
		fputs(wrapped.c_str(), stdout);

	//Ensure that this message is displayed immediately even if we print lower severity stuff later
	if(severity <= Severity::WARNING)
		Flush();

	//See if we printed a \n
	if(wrapped.length() && (wrapped[wrapped.length() - 1] == '\n'))
		m_lastMessageWasNewline = true;
	else if(wrapped != "")
		m_lastMessageWasNewline = false;
}

void STDLogSink::Log(Severity severity, const char *format, va_list va)
{
	//Skip messages which aren't important enough
	if(severity > m_min_severity)
		return;

	//Prevent newer messages on stderr from appearing before older messages on stdout
	if(severity <= Severity::WARNING)
		Flush();

	//Wrap/print it
	string wrapped = WrapString(vstrprintf(format, va));
	if( (severity <= Severity::WARNING) && !g_logToStdoutAlways )
		fputs(wrapped.c_str(), stderr);
	else
		fputs(wrapped.c_str(), stdout);

	//Ensure that this message is displayed immediately even if we print lower severity stuff later
	if(severity <= Severity::WARNING)
		Flush();

	//See if we printed a \n
	if(wrapped.length() && (wrapped[wrapped.length() - 1] == '\n'))
		m_lastMessageWasNewline = true;
	else if(wrapped != "")
		m_lastMessageWasNewline = false;
}
