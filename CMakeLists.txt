add_library(log STATIC
	log.cpp
	STDLogSink.cpp
	FILELogSink.cpp)

target_include_directories(log
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
