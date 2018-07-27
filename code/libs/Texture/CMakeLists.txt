#--------------------------------------------------------------------------
# texture project
#--------------------------------------------------------------------------

PROJECT(texture)
FILE(GLOB texture_headers *.h)
FILE(GLOB texture_sources *.cpp)

SET(files_texture
	${texture_headers} 
	${texture_sources})

SOURCE_GROUP("texture" FILES ${files_texture})

ADD_LIBRARY(texture STATIC ${files_texture})
TARGET_LINK_LIBRARIES(texture mymathlib gl_core)
SET_TARGET_PROPERTIES(texture PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(texture PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(texture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})