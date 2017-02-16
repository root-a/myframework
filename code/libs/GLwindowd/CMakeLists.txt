#--------------------------------------------------------------------------
# gl_windowd
#--------------------------------------------------------------------------

SET(files_gl_windowd
	gl_window.h
	gl_window.cc
	config.h
	config.cc)
SOURCE_GROUP("gl_windowd" FILES ${files_gl_windowd})

ADD_LIBRARY(gl_windowd STATIC ${files_gl_windowd})
TARGET_LINK_LIBRARIES(gl_windowd gl_core glfw)
SET_TARGET_PROPERTIES(gl_windowd PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(gl_windowd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})