#--------------------------------------------------------------------------
# fbo_manager project
#--------------------------------------------------------------------------

PROJECT(fbo_manager)
FILE(GLOB fbo_manager_headers *.h)
FILE(GLOB fbo_manager_sources *.cpp)

SET(files_fbo_manager
	${fbo_manager_headers} 
	${fbo_manager_sources})

SOURCE_GROUP("fbo_manager" FILES ${files_fbo_manager})

ADD_LIBRARY(fbo_manager STATIC ${files_fbo_manager})
TARGET_LINK_LIBRARIES(fbo_manager gl_core mymathlib texture render_buffer)
SET_TARGET_PROPERTIES(fbo_manager PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(fbo_manager PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(fbo_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})