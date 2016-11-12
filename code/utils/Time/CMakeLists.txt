#--------------------------------------------------------------------------
# time project
#--------------------------------------------------------------------------

PROJECT(time)
FILE(GLOB time_headers *.h)
FILE(GLOB time_sources *.cpp)

SET(files_time
	${time_headers} 
	${time_sources})

SOURCE_GROUP("time" FILES ${files_time})

ADD_LIBRARY(time STATIC ${files_time})
TARGET_LINK_LIBRARIES(time)
SET_TARGET_PROPERTIES(time PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(time PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(time PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})