/***********************************************************************************************************************
*                                                                                                                      *
* logtools                                                                                                             *
*                                                                                                                      *
* Copyright (c) 2016-2024 Andrew D. Zonenberg and contributors                                                         *
* All rights reserved.                                                                                                 *
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
***********************************************************************************************************************/

/**
	@file
	@brief		Implementation of base LogSink class and some global helper functions
	@ingroup	liblog
 */

#include "log.h"
#include <cstdarg>
#include <cstdlib>
#include <string>

using namespace std;

/**
	@brief		Mutex for serializing access to global logging state
	@ingroup	logtools
 */
mutex g_log_mutex;

/**
	@brief		The current indentation level
	@ingroup	logtools
 */
__thread unsigned int g_logIndentLevel = 0;

/**
	@brief		The set of log sink objects logtools knows about.

	When a log message is printed, it is sent to every sink in this list for filtering and display.

	@ingroup	logtools
 */
vector<unique_ptr<LogSink>> g_log_sinks;

/**
	@brief		If set, STDLogSink will only write to stdout even for error/warning severity and never use stderr

	@ingroup	logtools
 */
bool g_logToStdoutAlways = false;

/**
	@brief		Set of classes or class::function for high verbosity trace messages

	@ingroup	logtools
 */
set<string> g_trace_filters;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// String formatting

/**
	@brief Like sprintf, but self-managing a buffer with a std::string
 */
string LogSink::vstrprintf(const char* format, va_list va)
{
	//Figure out how much space we need
	va_list va_tmp;
	va_copy(va_tmp, va);

	int len = vsnprintf(NULL, 0, format, va_tmp);
	va_end(va_tmp);

	if(len < 0)
		return "";

	//and then format the real string
	string ret;
	ret.resize(len+1);
	vsnprintf(&ret[0], len+1, format, va);
	ret.resize(len);
	return ret;
}

string LogSink::GetIndentString()
	{ return std::string(m_indentSize * g_logIndentLevel, ' '); }

/**
	@brief Wraps long lines and adds indentation as needed
 */
string LogSink::WrapString(string str)
{
	string ret = "";

	//Cache the indent string so we don't have to re-generate it each time
	string indent = GetIndentString();

	//Split the string into lines
	string tmp;
	bool firstLine = true;
	for(size_t i=0; i<str.length(); i++)
	{
		//Append it
		char ch = str[i];
		tmp += ch;

		//Unless line is overly long, or done, nothing to do
		if( ( (tmp.length() + indent.length() )  < m_termWidth) && (ch != '\n') )
			continue;

		//We're ending this line
		//Only indent the first line if the previous message ended in \n
		if( (firstLine && m_lastMessageWasNewline) || !firstLine )
			ret += indent;
		firstLine = false;

		//Add the line after preprocessing as needed
		PreprocessLine(tmp);
		ret += tmp;

		//If we're wrapping due to a long line, add a \n to force it
		if(ch != '\n')
			ret += "\n";

		//Either way, we're done with the current line
		tmp = "";
	}

	//If we have any remaining stuff, append it
	if(tmp != "")
		ret += tmp;

	//Done
	return ret;
}

/**
	@brief Do any processing required to a line before printing it. Nothing in the base class.
 */
void LogSink::PreprocessLine(string& /*line*/)
{
}


LogIndenter::LogIndenter()
{
	//no mutexing needed b/c thread local
	g_logIndentLevel ++;
}

LogIndenter::~LogIndenter()
{
	g_logIndentLevel --;
}

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
		if(i+1 < argc)
		{
			FILE *log = fopen(argv[++i], "wt");
			g_log_sinks.emplace_back(new FILELogSink(log, line_buffered, console_verbosity));
		}
		else
		{
			printf("%s requires an argument\n", s.c_str());
		}
	}
	else if(s == "--trace")
	{
		if(i+1 < argc)
		{
			string sfilter = argv[++i];
			if(sfilter == "::")
				sfilter = "";
			g_trace_filters.emplace(sfilter);
		}
		else
		{
			printf("%s requires an argument\n", s.c_str());
		}
	}
	else if(s == "--stdout-only")
		g_logToStdoutAlways = true;

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
	lock_guard<mutex> lock(g_log_mutex);

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
	lock_guard<mutex> lock(g_log_mutex);

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
	lock_guard<mutex> lock(g_log_mutex);

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
	lock_guard<mutex> lock(g_log_mutex);

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
	lock_guard<mutex> lock(g_log_mutex);

	va_list va;
	for(auto &sink : g_log_sinks) {
		va_start(va, format);
		sink->Log(Severity::VERBOSE, format, va);
		va_end(va);
	}
}

void LogDebug(const char *format, ...)
{
	lock_guard<mutex> lock(g_log_mutex);

	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(Severity::DEBUG, format, va);
		va_end(va);
	}
}

void LogDebugTrace(const char* function, const char *format, ...)
{
	lock_guard<mutex> lock(g_log_mutex);

	//Early out (for performance) if we don't have any debug-level sinks
	bool has_debug_sinks = false;
	for(auto &sink : g_log_sinks)
	{
		if(sink->GetSeverity() >= Severity::DEBUG)
		{
			has_debug_sinks = true;
			break;
		}
	}
	if(!has_debug_sinks)
		return;

	string sfunc(function);

	//Class and function names
	string cls;
	string name = sfunc;

	//Member function?
	//Parse out "class::function" from PRETTY_FUNCTION which includes the return type and full arg list
	//This normally gives us zillions of templates we dont need to see!
	size_t poff = sfunc.rfind("(");
	size_t colpos = sfunc.rfind("::", poff);
	if( (colpos != string::npos) && (poff != string::npos))
	{
		//Get the function name
		size_t namelen = poff - colpos - 2;
		name = sfunc.substr(colpos+2, namelen);

		//Member function
		size_t coff = name.find(" ");
		if(coff == string::npos)
		{
			//Get the class name
			size_t clen = colpos - coff - 1;
			cls = sfunc.substr(coff + 1, clen);

			//Remove any leading space-delimited values in the class name (return types etc)
			coff = cls.rfind(" ");
			cls = cls.substr(coff+1);
		}

		//Global function returning a namespaced type
		else
			name = name.substr(coff + 1);
	}

	//Global function
	else
	{
		size_t soff = sfunc.find(" ");
		poff = sfunc.find("(", soff);
		if( (soff != string::npos) && (poff != string::npos) )
		{
			size_t namelen = poff - soff - 1;
			name = sfunc.substr(soff+1, namelen);
		}
	}

	//Format final function name
	sfunc = cls + "::" + name;

	//Check if class or function name is in the "to log" list
	if(cls == "")
	{
		if(g_trace_filters.find(sfunc) == g_trace_filters.end())
			return;
	}
	else if(g_trace_filters.find(cls) == g_trace_filters.end())
		return;

	va_list va;
	for(auto &sink : g_log_sinks)
	{
		//First, print the function name prefix
		sink->Log(Severity::DEBUG, string("[") + sfunc + "] " + sink->GetIndentString());

		//then the message
		va_start(va, format);
		sink->Log(Severity::DEBUG, format, va);
		va_end(va);
	}
}

void Log(Severity severity, const char *format, ...)
{
	lock_guard<mutex> lock(g_log_mutex);

	va_list va;
	for(auto &sink : g_log_sinks)
	{
		va_start(va, format);
		sink->Log(severity, format, va);
		va_end(va);
	}
}
