#--------------------------------------------------------------------------
# drawables_systems project
#--------------------------------------------------------------------------

PROJECT(drawables_systems)
FILE(GLOB drawables_systems_headers *.h)
FILE(GLOB drawables_systems_sources *.cpp)

SET(files_drawables_systems
	${drawables_systems_headers} 
	${drawables_systems_sources})

SOURCE_GROUP("drawables_systems" FILES ${files_drawables_systems})

ADD_LIBRARY(drawables_systems STATIC ${files_drawables_systems})
TARGET_LINK_LIBRARIES(drawables_systems object glew frustum camera_manager scene vao graphics_storage graphics_manager)
SET_TARGET_PROPERTIES(drawables_systems PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(drawables_systems PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(drawables_systems PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})