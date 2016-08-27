/***********************************************************************************************************************
 * Copyright (C) 2016 Andrew Zonenberg and contributors                                                                *
 *                                                                                                                     *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General   *
 * Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) *
 * any later version.                                                                                                  *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for     *
 * more details.                                                                                                       *
 *                                                                                                                     *
 * You should have received a copy of the GNU Lesser General Public License along with this program; if not, you may   *
 * find one here:                                                                                                      *
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt                                                              *
 * or you may search the http://www.gnu.org website for the version 2.1 license, or you may write to the Free Software *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA                                      *
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
			g_log_sinks.emplace_back(new FILELogSink(log, line_buffered));
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
// Convenience functions that log into all configured sinks

void LogFatal(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		sink->Log(Severity::FATAL, "INTERNAL ERROR: ");

		va_start(va, format);
		sink->Log(Severity::FATAL, format, va);
		va_end(va);

		sink->Log(Severity::FATAL,
			"    This indicates a bug in the program, please file a report via Github\n");
	}

	abort();
}

void LogError(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		sink->Log(Severity::ERROR, "ERROR: ");

		va_start(va, format);
		sink->Log(Severity::ERROR, format, va);
		va_end(va);
	}
}

void LogWarning(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		sink->Log(Severity::WARNING, "Warning: ");

		va_start(va, format);
		sink->Log(Severity::WARNING, format, va);
		va_end(va);
	}
}

void LogNotice(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
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
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(Severity::DEBUG, format, va);
		va_end(va);
	}
}

void Log(Severity severity, const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(severity, format, va);
		va_end(va);
	}
}
