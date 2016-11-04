#--------------------------------------------------------------------------
# gl_window
#--------------------------------------------------------------------------

SET(files_gl_window
	gl_window.h
	gl_window.cc
	config.h
	config.cc)
SOURCE_GROUP("gl_window" FILES ${files_gl_window})

ADD_LIBRARY(gl_window STATIC ${files_gl_window})
TARGET_LINK_LIBRARIES(gl_window gl_core glfw)
SET_TARGET_PROPERTIES(gl_window PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(gl_window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})