#--------------------------------------------------------------------------
# shader_block project
#--------------------------------------------------------------------------

PROJECT(shader_block)
FILE(GLOB shader_block_headers *.h)
FILE(GLOB shader_block_sources *.cpp)

SET(files_shader_block
	${shader_block_headers} 
	${shader_block_sources})

SOURCE_GROUP("shader_block" FILES ${files_shader_block})

ADD_LIBRARY(shader_block STATIC ${files_shader_block})
TARGET_LINK_LIBRARIES(shader_block gl_core)
SET_TARGET_PROPERTIES(shader_block PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(shader_block PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(shader_block PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})