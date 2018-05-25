#--------------------------------------------------------------------------
# luaJIT project
#--------------------------------------------------------------------------

PROJECT(luajit)
INCLUDE_DIRECTORIES("include/")

#FILE(GLOB luaJIT_headers "src/*.h")
#FILE(GLOB luaJIT_sources "src/*.c")

#SET(files_luaJIT
#	${luaJIT_headers} 
#	${luaJIT_sources})

set(myluajit_HEAD 
	include/lua.h
	include/lauxlib.h
	include/lualib.h
	include/luajit.h
	include/luaconf.h
    include/luajit.hpp)
    
set(myluajit_SRC
	luaJIT.c
)
#LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib)
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    message(STATUS "Target is 64 bits")
    FIND_LIBRARY(lua51Lib lua51 HINT lib64/)
    FIND_LIBRARY(luaJitLib luajit HINT lib64/)
    FIND_LIBRARY(buildvmLib buildvm HINT lib64/)
    FIND_LIBRARY(miniluaLib minilua HINT lib64/)
else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    message(STATUS "Target is 32 bits")
    FIND_LIBRARY(lua51Lib lua51 HINT lib32/)
    FIND_LIBRARY(luaJitLib luajit HINT lib32/)
    FIND_LIBRARY(buildvmLib buildvm HINT lib32/)
    FIND_LIBRARY(miniluaLib minilua HINT lib32/)
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

ADD_LIBRARY(luajit STATIC ${myluajit_SRC} ${myluajit_HEAD})

TARGET_LINK_LIBRARIES(luajit ${lua51Lib} ${luaJitLib} ${buildvmLib} ${miniluaLib})
SET_TARGET_PROPERTIES(luajit PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(luajit PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(luajit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})