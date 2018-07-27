#--------------------------------------------------------------------------
# times project
#--------------------------------------------------------------------------

PROJECT(times)
FILE(GLOB times_headers *.h)
FILE(GLOB times_sources *.cpp)

SET(files_times
	${times_headers} 
	${times_sources})

SOURCE_GROUP("times" FILES ${files_times})

ADD_LIBRARY(times STATIC ${files_times})
TARGET_LINK_LIBRARIES(times)
SET_TARGET_PROPERTIES(times PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(times PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(times PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})