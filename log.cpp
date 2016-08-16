/************************************************************************************************************************
 * Copyright (C) 2016 Andrew Zonenberg and contributors                                                                 *
 *                                                                                                                      *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
 * following conditions are met:                                                                                        *
 *                                                                                                                      *
 *    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
 *      following disclaimer.                                                                                           *
 *                                                                                                                      *
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
 *      following disclaimer in the documentation and/or other materials provided with the distribution.                *
 *                                                                                                                      *
 *    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
 *      derived from this software without specific prior written permission.                                           *
 *                                                                                                                      *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
 * THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 * POSSIBILITY OF SUCH DAMAGE.                                                                                          *
 *                                                                                                                      *
 ************************************************************************************************************************/

#include "log.h"
#include <cstdarg>
#include <cstdlib>
#include <string>

std::vector<std::unique_ptr<LogSink>> g_log_sinks;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convenience functions that log into all configured sinks

void LogFatal(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		sink->Log(LogSink::FATAL, "INTERNAL ERROR: ");

		va_start(va, format);
		sink->Log(LogSink::FATAL, format, va);
		va_end(va);

		sink->Log(LogSink::FATAL, 
			"    This indicates a bug in the program, please file a report via Github\n");
	}

	abort();
}

void LogError(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		sink->Log(LogSink::ERROR, "ERROR: ");

		va_start(va, format);
		sink->Log(LogSink::ERROR, format, va);
		va_end(va);
	}
}

void LogWarning(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		sink->Log(LogSink::WARNING, "Warning: ");

		va_start(va, format);
		sink->Log(LogSink::WARNING, format, va);
		va_end(va);
	}
}

void LogNotice(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(LogSink::NOTICE, format, va);
		va_end(va);
	}
}

void LogVerbose(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(LogSink::VERBOSE, format, va);
		va_end(va);
	}
}

void LogDebug(const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(LogSink::DEBUG, format, va);
		va_end(va);
	}
}

void Log(LogSink::Severity severity, const char *format, ...)
{
	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(severity, format, va);
		va_end(va);
	}
}
