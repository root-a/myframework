#--------------------------------------------------------------------------
# externals project
#--------------------------------------------------------------------------

PROJECT(externals)
FILE(GLOB externals_headers *.h)
FILE(GLOB externals_sources *.cpp)

SET(files_externals
	${externals_headers} 
	${externals_sources})

SOURCE_GROUP("externals" FILES ${files_externals})

ADD_LIBRARY(externals STATIC ${files_externals})
TARGET_LINK_LIBRARIES(externals gl_windowd mymathlib graphics_storage fbo_manager render_pass scene_graph shader)
SET_TARGET_PROPERTIES(externals PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(externals PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(externals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})