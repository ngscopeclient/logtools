# CMake build script for logtools.
# Intended to be integrated into a larger project, not built standalone.

add_library(log STATIC
	log.cpp
	ColoredSTDLogSink.cpp
	STDLogSink.cpp
	FILELogSink.cpp)

target_include_directories(log
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
