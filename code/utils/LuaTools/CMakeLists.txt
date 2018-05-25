#--------------------------------------------------------------------------
# lua_tools project
#--------------------------------------------------------------------------

PROJECT(lua_tools)
FILE(GLOB lua_tools_headers *.h)
FILE(GLOB lua_tools_sources *.cpp)

SET(files_lua_tools
	${lua_tools_headers} 
	${lua_tools_sources})

SOURCE_GROUP("lua_tools" FILES ${files_lua_tools})

ADD_LIBRARY(lua_tools STATIC ${files_lua_tools})
TARGET_LINK_LIBRARIES(lua_tools luajit)
SET_TARGET_PROPERTIES(lua_tools PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(lua_tools PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(lua_tools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})