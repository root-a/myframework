#--------------------------------------------------------------------------
# imgui project
#--------------------------------------------------------------------------

PROJECT(imgui)
INCLUDE_DIRECTORIES("misc/cpp/")

FILE(GLOB imgui_headers "*.h")
FILE(GLOB imgui_sources "*.cpp")

SET(files_imgui
	${imgui_headers} 
	${imgui_sources})

SOURCE_GROUP("imgui" FILES ${files_imgui})

FILE(GLOB imgui_headers_misc "misc/cpp/*.h")
FILE(GLOB imgui_sources_misc "misc/cpp/*.cpp")

SET(files_imgui_misc
	${imgui_headers_misc} 
	${imgui_sources_misc})

SOURCE_GROUP("imgui\\misc" FILES ${files_imgui_misc})

LIST(APPEND ALLSRCS ${files_imgui} ${files_imgui_misc})

ADD_LIBRARY(imgui STATIC ${ALLSRCS})
SET_TARGET_PROPERTIES(imgui PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(imgui PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
