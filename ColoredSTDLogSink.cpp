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

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

ColoredSTDLogSink::ColoredSTDLogSink(Severity min_severity)
	: STDLogSink(min_severity)
{

}

ColoredSTDLogSink::~ColoredSTDLogSink()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging

void ColoredSTDLogSink::PreprocessLine(std::string& line)
{
	//strings we make red
	static string red_strings[] =
	{
		"INTERNAL ERROR",
		"ERROR",
		"Error",
		"error",
	};

	//ANSI escape codes
	static string red = "\033[31;1m";
	static string yellow = "\033[33;1m";
	static string clear = "\033[0m";

	//Bold red errors. If there's a colon after the message, do that too
	for(auto s : red_strings)
	{
		line = replace(s + ":", red, clear, line);
		//line = replace(s, red + s + clear, line);
	}

	//strings we make yellow
	static string yellow_strings[] =
	{
		"WARNING",
		"Warning",
		"warning"
	};

	//Bold yellow warnings. If there's a colon after the message, do that too
	for(auto s : yellow_strings)
	{
		line = replace(s + ":", yellow, clear, line);
		//line = replace(s, yellow + s + clear, line);
	}
}

//TODO: less copying
string ColoredSTDLogSink::replace(
	const string& search,
	const string& before,
	const string& after,
	string subject)
{
	//If not found, return unchanged
	size_t pos = subject.find(search);
	if(pos == string::npos)
		return subject;

	//If found, change color starting beginning of the line until end of the string
	size_t end = pos + search.length();
	string ret = before;
	ret += subject.substr(0, end);
	ret += after;
	ret += subject.substr(end);
	return ret;
}
