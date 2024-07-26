#--------------------------------------------------------------------------
# scripts_component project
#--------------------------------------------------------------------------

PROJECT(scripts_component)
FILE(GLOB scripts_component_headers *.h)
FILE(GLOB scripts_component_sources *.cpp)

SET(files_scripts_component
	${scripts_component_headers} 
	${scripts_component_sources})

SOURCE_GROUP("scripts_component" FILES ${files_scripts_component})

ADD_LIBRARY(scripts_component STATIC ${files_scripts_component})
TARGET_LINK_LIBRARIES(scripts_component component script)
SET_TARGET_PROPERTIES(scripts_component PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(scripts_component PROPERTIES FOLDER "MyLibs/Components")
TARGET_INCLUDE_DIRECTORIES(scripts_component PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})