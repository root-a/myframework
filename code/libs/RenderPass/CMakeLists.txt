#--------------------------------------------------------------------------
# render_pass project
#--------------------------------------------------------------------------

PROJECT(render_pass)
FILE(GLOB render_pass_headers *.h)
FILE(GLOB render_pass_sources *.cpp)

SET(files_render_pass
	${render_pass_headers} 
	${render_pass_sources})

SOURCE_GROUP("render_pass" FILES ${files_render_pass})

ADD_LIBRARY(render_pass STATIC ${files_render_pass})
TARGET_LINK_LIBRARIES(render_pass fbo_manager script object_profile)
SET_TARGET_PROPERTIES(render_pass PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(render_pass PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(render_pass PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})