#--------------------------------------------------------------------------
# vao project
#--------------------------------------------------------------------------

PROJECT(vao)
FILE(GLOB vao_headers *.h)
FILE(GLOB vao_sources *.cpp)

SET(files_vao
	${vao_headers} 
	${vao_sources})

SOURCE_GROUP("vao" FILES ${files_vao})

ADD_LIBRARY(vao STATIC ${files_vao})
TARGET_LINK_LIBRARIES(vao mymathlib render_element gl_core shader_block)
SET_TARGET_PROPERTIES(vao PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(vao PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(vao PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})