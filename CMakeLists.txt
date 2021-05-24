# CMake build script for logtools.
# Intended to be integrated into a larger project, not built standalone.

if(WIN32)
add_library(log SHARED
	log.cpp
	ColoredSTDLogSink.cpp
	STDLogSink.cpp
	FILELogSink.cpp)
install(TARGETS log LIBRARY)
else()
add_library(log STATIC
	log.cpp
	ColoredSTDLogSink.cpp
	STDLogSink.cpp
	FILELogSink.cpp)
endif()

target_include_directories(log
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
