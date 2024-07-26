#--------------------------------------------------------------------------
# render project
#--------------------------------------------------------------------------

PROJECT(render)
FILE(GLOB render_headers *.h)
FILE(GLOB render_sources *.cpp)

SET(files_render
	${render_headers} 
	${render_sources})

SOURCE_GROUP("render" FILES ${files_render})

ADD_LIBRARY(render STATIC ${files_render})
TARGET_LINK_LIBRARIES(render gl_windowd frustum fbo_manager camera_manager scene_graph shader render_profile imgui_wrapper render_pass particle)
SET_TARGET_PROPERTIES(render PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(render PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})