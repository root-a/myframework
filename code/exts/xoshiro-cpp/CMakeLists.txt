#--------------------------------------------------------------------------
# xoshiro-cpp project
#--------------------------------------------------------------------------

PROJECT(xoshiro-cpp)
FILE(GLOB xoshiro-cpp_headers "*.hpp")
FILE(GLOB xoshiro-cpp_sources "*.cpp")

SET(files_xoshiro-cpp
	${xoshiro-cpp_headers} 
	${xoshiro-cpp_sources})

SOURCE_GROUP("xoshiro-cpp" FILES ${files_xoshiro-cpp})

ADD_LIBRARY(xoshiro-cpp STATIC ${files_xoshiro-cpp})
SET_TARGET_PROPERTIES(xoshiro-cpp PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(xoshiro-cpp PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(xoshiro-cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})