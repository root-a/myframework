#--------------------------------------------------------------------------
# bounds
#--------------------------------------------------------------------------

PROJECT(bounds)
FILE(GLOB bounds_headers *.h)
FILE(GLOB bounds_sources *.cpp)

SET(files_bounds
	${bounds_headers} 
	${bounds_sources})

SOURCE_GROUP("bounds" FILES ${files_bounds})

ADD_LIBRARY(bounds STATIC ${files_bounds})
TARGET_LINK_LIBRARIES(bounds object)
SET_TARGET_PROPERTIES(bounds PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(bounds PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(bounds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})