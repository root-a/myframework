#--------------------------------------------------------------------------
# imgui_wrapper project
#--------------------------------------------------------------------------

PROJECT(imgui_wrapper)
FILE(GLOB imgui_wrapper_headers *.h)
FILE(GLOB imgui_wrapper_sources *.cpp)

SET(files_imgui_wrapper
	${imgui_wrapper_headers} 
	${imgui_wrapper_sources})

SOURCE_GROUP("imgui_wrapper" FILES ${files_imgui_wrapper})

ADD_LIBRARY(imgui_wrapper STATIC ${files_imgui_wrapper})
TARGET_LINK_LIBRARIES(imgui_wrapper imgui glfw gl_core gl_windowd)
SET_TARGET_PROPERTIES(imgui_wrapper PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(imgui_wrapper PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(imgui_wrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})