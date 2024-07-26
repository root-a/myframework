#--------------------------------------------------------------------------
# script project
#--------------------------------------------------------------------------

PROJECT(script)
FILE(GLOB script_headers *.h)
FILE(GLOB script_sources *.cpp)

SET(files_script
	${script_headers} 
	${script_sources})

SOURCE_GROUP("script" FILES ${files_script})

ADD_LIBRARY(script STATIC ${files_script})
TARGET_LINK_LIBRARIES(script lua_tools)
SET_TARGET_PROPERTIES(script PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(script PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(script PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})