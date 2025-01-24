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

#ifndef log_h
#define log_h

/**
	@file
	@brief		Main logging framework header
	@ingroup	liblog
 */

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <mutex>

#if defined(__MINGW32__)
#undef ERROR
#endif

#if defined(_MSC_VER)
// MSVC
#include <thread>
#undef ERROR
#define __thread __declspec(thread)
#endif

/**
	@brief		Severity of a logging message
	@ingroup	liblog
 */
enum class Severity
{
	///State is totally unusable, must exit right now. Aborts the program after printing.
	FATAL	= 1,

	///@brief Something went very wrong, an operation may be aborted or state may be confused
	ERROR	= 2,

	///@brief Something went wrong, but we'll attempt to proceed
	WARNING	= 3,

	///@brief Useful information about progress printed by default
	NOTICE	= 4,

	///@brief Detailed information end users may sometimes need, but not often
	VERBOSE	= 5,

	///@brief Extremely detailed information only useful to people working on application internals
	DEBUG = 6
};

extern __thread unsigned int g_logIndentLevel;

/**
	@brief		Base class for all log sinks
	@ingroup	liblog
 */
class LogSink
{
public:
	LogSink(Severity min_severity = Severity::VERBOSE)
	: m_indentSize(4)
	, m_termWidth(120)	//default if not using ioctls to check
	, m_lastMessageWasNewline(true)
	, m_min_severity(min_severity)
	{}

	virtual ~LogSink() {}

	///@brief Returns the current severity / verbosity level
	Severity GetSeverity()
	{ return m_min_severity; }

	/**
		@brief Gets the indent string (for now, only used by STDLogSink)

		Each log message printed is prefixed with (indentLevel * indentSize) space characters.
		No parsing of newline etc characters is performed.
	 */
	std::string GetIndentString();

	virtual void Log(Severity severity, const std::string &msg) = 0;
	virtual void Log(Severity severity, const char *format, va_list va) = 0;

	std::string vstrprintf(const char* format, va_list va);

protected:

	std::string WrapString(std::string str);
	virtual void PreprocessLine(std::string& line);

	/// @brief Number of spaces in one indentation
	unsigned int m_indentSize;

	/// @brief Width of the console we're printing to, in characters
	unsigned int m_termWidth;

	/// @brief True if the last message ended in a \n character
	bool m_lastMessageWasNewline;

	/// @brief Minimum severity of messages to be printed
	Severity m_min_severity;
};

/**
	@brief A log sink writing to stdout/stderr depending on severity
	@ingroup	liblog
 */
class STDLogSink : public LogSink
{
public:
	STDLogSink(Severity min_severity = Severity::VERBOSE);
	~STDLogSink() override;

	void Log(Severity severity, const std::string &msg) override;
	void Log(Severity severity, const char *format, va_list va) override;

protected:
	void Flush();
};

/**
	@brief		A STDLogSink that colorizes "warning" or "error" keywords
	@ingroup	liblog
 */
class ColoredSTDLogSink : public STDLogSink
{
public:
	ColoredSTDLogSink(Severity min_severity = Severity::VERBOSE);
	~ColoredSTDLogSink() override;

protected:
	void PreprocessLine(std::string& line) override;
	std::string replace(
		const std::string& search,
		const std::string& before,
		const std::string& after,
		std::string subject);
};

/**
	@brief 		A log sink writing to a FILE* file handle
	@ingroup	liblog
 */
class FILELogSink : public LogSink
{
public:
	FILELogSink(FILE *f, bool line_buffered = false, Severity min_severity = Severity::VERBOSE);
	~FILELogSink() override;

	void Log(Severity severity, const std::string &msg) override;
	void Log(Severity severity, const char *format, va_list va) override;

protected:
	FILE		*m_file;
};

extern std::mutex g_log_mutex;
extern std::vector<std::unique_ptr<LogSink>> g_log_sinks;
extern std::set<std::string> g_trace_filters;

/**
	@brief		RAII wrapper for log indentation
	@ingroup	liblog

	Log messages are indented once for each LogIndenter object in the call stack.
 */
class LogIndenter
{
public:
	LogIndenter();

	~LogIndenter();
};

/**
	@brief		Helper function for parsing arguments that use common syntax
	@ingroup	liblog
 */
bool ParseLoggerArguments(
	int& i,
	int argc,
	char* argv[],
	Severity& console_verbosity);


#ifdef __GNUC__
#if defined(__MINGW32__) && !defined(__clang__)
#define ATTR_FORMAT(n, m) __attribute__((__format__ (gnu_printf, n, m)))
#else
#define ATTR_FORMAT(n, m) __attribute__((__format__ (__printf__, n, m)))
#endif
#define ATTR_NORETURN     __attribute__((noreturn))
#else
//FIXME on MSVC/Windows
#define ATTR_FORMAT(n, m)
#define ATTR_NORETURN
#endif

/**
	\def(LogTrace(...)
	@ingroup	liblog

	Helper for logging "trace" messages with the class::function name prepended to the message.

	Only printed if at debug level verbosity, plus explicitly turned on for this class or function
	(usually by --trace command line argument)
 */
#ifdef __GNUC__
#define LogTrace(...) LogDebugTrace(__PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#define LogTrace(...) LogDebugTrace(__func__, __VA_ARGS__)
#endif

ATTR_FORMAT(1, 2) void LogVerbose(const char *format, ...);
ATTR_FORMAT(1, 2) void LogNotice(const char *format, ...);
ATTR_FORMAT(1, 2) void LogWarning(const char *format, ...);
ATTR_FORMAT(1, 2) void LogError(const char *format, ...);
ATTR_FORMAT(1, 2) void LogDebug(const char *format, ...);
ATTR_FORMAT(2, 3) void LogDebugTrace(const char* function, const char *format, ...);
ATTR_FORMAT(1, 2) ATTR_NORETURN void LogFatal(const char *format, ...);

///Just print the message at given log level, don't do anything special for warnings or errors
ATTR_FORMAT(2, 3) void Log(Severity severity, const char *format, ...);

#undef ATTR_FORMAT
#undef ATTR_NORETURN

std::string LogHexDump(const unsigned char* data, size_t len);

#endif

