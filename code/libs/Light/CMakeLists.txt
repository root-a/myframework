#--------------------------------------------------------------------------
# light project
#--------------------------------------------------------------------------

PROJECT(light)
FILE(GLOB light_headers *.h)
FILE(GLOB light_sources *.cpp)

SET(files_light
	${light_headers} 
	${light_sources})

SOURCE_GROUP("light" FILES ${files_light})

ADD_LIBRARY(light STATIC ${files_light})
TARGET_LINK_LIBRARIES(light mymathlib component object fbo_manager)
SET_TARGET_PROPERTIES(light PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(light PROPERTIES FOLDER "MyLibs/Components")
TARGET_INCLUDE_DIRECTORIES(light PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})