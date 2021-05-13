#--------------------------------------------------------------------------
# render_element project
#--------------------------------------------------------------------------

PROJECT(render_element)
FILE(GLOB render_element_headers *.h)
FILE(GLOB render_element_sources *.cpp)

SET(files_render_element
	${render_element_headers} 
	${render_element_sources})

SOURCE_GROUP("render_element" FILES ${files_render_element})

ADD_LIBRARY(render_element STATIC ${files_render_element})
TARGET_LINK_LIBRARIES(render_element)
SET_TARGET_PROPERTIES(render_element PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(render_element PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(render_element PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})