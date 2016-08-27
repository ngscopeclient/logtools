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

#ifndef log_h
#define log_h

#include <memory>
#include <vector>

/**
	@brief The message severity
 */
enum class Severity
{
	FATAL	= 1,	//State is totally unusable, must exit right now
	ERROR	= 2,	//Design is unroutable, cannot continue
	WARNING	= 3,	//Design may have an error, but we'll attempt to proceed at your own risk
	NOTICE	= 4,	//Useful information about progress
	VERBOSE	= 5,	//Detailed information end users may sometimes need, but not often
	DEBUG = 6		//Extremely detailed information only useful to people working on the toolchain internals
};

/**
	@brief The log sink
 */
class LogSink
{
public:
	virtual ~LogSink() {}

	virtual void Log(Severity severity, const std::string &msg) = 0;
	virtual void Log(Severity severity, const char *format, va_list va) = 0;
};

/**
	@brief The log sink writing to stdout/stderr
 */
class STDLogSink : public LogSink
{
public:
	STDLogSink(Severity min_severity = Severity::VERBOSE);
	~STDLogSink() override;

	void Log(Severity severity, const std::string &msg) override;
	void Log(Severity severity, const char *format, va_list va) override;

protected:
	Severity	m_min_severity;

};
/**
	@brief The log sink writing to a FILE* file handle
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
	Severity	m_min_severity;

};

extern std::vector<std::unique_ptr<LogSink>> g_log_sinks;

/**
	@brief Helper function for parsing arguments that use common syntax
 */
bool ParseLoggerArguments(
	int& i,
	int argc,
	char* argv[],
	Severity& console_verbosity);
	

#ifdef __GNUC__
#define ATTR_FORMAT(n, m) __attribute__((__format__ (__printf__, n, m)))
#define ATTR_NORETURN     __attribute__((noreturn))
#else
#define ATTR_FORMAT(n, m)
#define ATTR_NORETURN
#endif

ATTR_FORMAT(1, 2) void LogVerbose(const char *format, ...);
ATTR_FORMAT(1, 2) void LogNotice(const char *format, ...);
ATTR_FORMAT(1, 2) void LogWarning(const char *format, ...);
ATTR_FORMAT(1, 2) void LogError(const char *format, ...);
ATTR_FORMAT(1, 2) void LogDebug(const char *format, ...);
ATTR_FORMAT(1, 2) ATTR_NORETURN void LogFatal(const char *format, ...);

///Just print the message at given log level, don't do anything special for warnings or errors
ATTR_FORMAT(2, 3) void Log(Severity severity, const char *format, ...);

#undef ATTR_FORMAT
#undef ATTR_NORETURN

#endif

