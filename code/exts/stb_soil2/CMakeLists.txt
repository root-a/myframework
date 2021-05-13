#--------------------------------------------------------------------------
# stb_soil2 project
#--------------------------------------------------------------------------

PROJECT(stb_soil2)
FILE(GLOB stb_soil2_headers "*.h")
FILE(GLOB stb_soil2_sources "*.c")

SET(files_stb_soil2
	${stb_soil2_headers} 
	${stb_soil2_sources})

SOURCE_GROUP("stb_soil2" FILES ${files_stb_soil2})

ADD_LIBRARY(stb_soil2 STATIC ${files_stb_soil2})
SET_TARGET_PROPERTIES(stb_soil2 PROPERTIES LINKER_LANGUAGE C)
SET_TARGET_PROPERTIES(stb_soil2 PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(stb_soil2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})