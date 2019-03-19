#--------------------------------------------------------------------------
# component project
#--------------------------------------------------------------------------

PROJECT(component)
FILE(GLOB component_headers *.h)
FILE(GLOB component_sources *.cpp)

SET(files_component
	${component_headers} 
	${component_sources})

SOURCE_GROUP("component" FILES ${files_component})

ADD_LIBRARY(component STATIC ${files_component})
TARGET_LINK_LIBRARIES(component object)
SET_TARGET_PROPERTIES(component PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(component PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(component PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})