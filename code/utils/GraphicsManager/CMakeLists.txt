#--------------------------------------------------------------------------
# graphics_storage project
#--------------------------------------------------------------------------

PROJECT(graphics_manager)
FILE(GLOB graphics_manager_headers *.h)
FILE(GLOB graphics_manager_sources *.cpp)

SET(files_graphics_manager
	${graphics_manager_headers} 
	${graphics_manager_sources})

SOURCE_GROUP("graphics_manager" FILES ${files_graphics_manager})
#if (MSVC)
#	ADD_DEFINITIONS(/bigobj)
#endif (MSVC)
ADD_LIBRARY(graphics_manager STATIC ${files_graphics_manager})
TARGET_LINK_LIBRARIES(graphics_manager graphics_storage shader_manager fbo_manager material gl_core stb_soil2 vao shader lua_tools)
SET_TARGET_PROPERTIES(graphics_manager PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(graphics_manager PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(graphics_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})