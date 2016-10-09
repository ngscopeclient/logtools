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
#include <cstdarg>
#include <cstdlib>
#include <string>

using namespace std;

vector<unique_ptr<LogSink>> g_log_sinks;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convenience function for parsing command-line arguments

bool ParseLoggerArguments(
	int& i,
	int argc,
	char* argv[],
	Severity& console_verbosity)
{
	string s(argv[i]);

	if(s == "-q" || s == "--quiet")
	{
		if(console_verbosity == Severity::DEBUG)
			console_verbosity = Severity::VERBOSE;
		else if(console_verbosity == Severity::VERBOSE)
			console_verbosity = Severity::NOTICE;
		else if(console_verbosity == Severity::NOTICE)
			console_verbosity = Severity::WARNING;
		else if(console_verbosity == Severity::WARNING)
			console_verbosity = Severity::ERROR;
	}
	else if(s == "--verbose")
		console_verbosity = Severity::VERBOSE;
	else if(s == "--debug")
		console_verbosity = Severity::DEBUG;
	else if(s == "-l" || s == "--logfile" ||
			s == "-L" || s == "--logfile-lines")
	{
		bool line_buffered = (s == "-L" || s == "--logfile-lines");
		if(i+1 < argc) {
			FILE *log = fopen(argv[++i], "wt");
			g_log_sinks.emplace_back(new FILELogSink(log, line_buffered, console_verbosity));
		}
		else
		{
			printf("%s requires an argument\n", s.c_str());
		}
	}

	//Unrecognized argument
	else
		return false;

	//We parsed this arg, caller should ignore it
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convenience functions that log into all configured sinks and apply indentation

void LogFatal(const char *format, ...)
{
	string sformat("INTERNAL ERROR: ");
	sformat += format;

	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(Severity::FATAL, sformat.c_str(), va);
		va_end(va);

		sink->Log(Severity::FATAL,
			"    This indicates a bug in the program, please file a report via Github\n");
	}

	abort();
}

void LogError(const char *format, ...)
{
	string sformat("ERROR: ");
	sformat += format;

	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(Severity::ERROR, sformat.c_str(), va);
		va_end(va);
	}
}

void LogWarning(const char *format, ...)
{
	string sformat("Warning: ");
	sformat += format;

	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(Severity::WARNING, sformat.c_str(), va);
		va_end(va);
	}
}

void LogNotice(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(Severity::NOTICE, format, va);
		va_end(va);
	}
}

void LogVerbose(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(Severity::VERBOSE, format, va);
		va_end(va);
	}
}

void LogDebug(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(Severity::DEBUG, format, va);
		va_end(va);
	}
}

void Log(Severity severity, const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(severity, format, va);
		va_end(va);
	}
}
