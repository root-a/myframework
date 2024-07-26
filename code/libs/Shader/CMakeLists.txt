#--------------------------------------------------------------------------
# shader project
#--------------------------------------------------------------------------

PROJECT(shader)
FILE(GLOB shader_headers *.h)
FILE(GLOB shader_sources *.cpp)

SET(files_shader
	${shader_headers} 
	${shader_sources})

SOURCE_GROUP("shader" FILES ${files_shader})

ADD_LIBRARY(shader STATIC ${files_shader})
TARGET_LINK_LIBRARIES(shader gl_core render_element mymathlib shader_block vao)
SET_TARGET_PROPERTIES(shader PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(shader PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(shader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})