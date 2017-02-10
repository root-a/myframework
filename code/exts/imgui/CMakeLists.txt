#--------------------------------------------------------------------------
# imgui project
#--------------------------------------------------------------------------

PROJECT(imgui)
FILE(GLOB imgui_headers "*.h")
FILE(GLOB imgui_sources "*.cpp")

SET(files_imgui
	${imgui_headers} 
	${imgui_sources})

SOURCE_GROUP("imgui" FILES ${files_imgui})

ADD_LIBRARY(imgui STATIC ${files_imgui})
SET_TARGET_PROPERTIES(imgui PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(imgui PROPERTIES FOLDER "Libs")
TARGET_INCLUDE_DIRECTORIES(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})