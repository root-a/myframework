#--------------------------------------------------------------------------
# render_buffer project
#--------------------------------------------------------------------------

PROJECT(render_buffer)
FILE(GLOB render_buffer_headers *.h)
FILE(GLOB render_buffer_sources *.cpp)

SET(files_render_buffer
	${render_buffer_headers} 
	${render_buffer_sources})

SOURCE_GROUP("render_buffer" FILES ${files_render_buffer})

ADD_LIBRARY(render_buffer STATIC ${files_render_buffer})
TARGET_LINK_LIBRARIES(render_buffer mymathlib gl_core)
SET_TARGET_PROPERTIES(render_buffer PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(render_buffer PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(render_buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})