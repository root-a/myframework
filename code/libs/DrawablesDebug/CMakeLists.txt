#--------------------------------------------------------------------------
# drawables_debug project
#--------------------------------------------------------------------------

PROJECT(drawables_debug)
FILE(GLOB drawables_debug_headers *.h)
FILE(GLOB drawables_debug_sources *.cpp)

SET(files_drawables_debug
	${drawables_debug_headers} 
	${drawables_debug_sources})

SOURCE_GROUP("drawables_debug" FILES ${files_drawables_debug})

ADD_LIBRARY(drawables_debug STATIC ${files_drawables_debug})
TARGET_LINK_LIBRARIES(drawables_debug object glew frustum camera_manager scene vao graphics_storage)
SET_TARGET_PROPERTIES(drawables_debug PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(drawables_debug PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(drawables_debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})