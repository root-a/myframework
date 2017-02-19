#--------------------------------------------------------------------------
# halfedgemesh2dsquared project
#--------------------------------------------------------------------------

PROJECT(halfedgemesh2dsquared)
FILE(GLOB halfedgemesh2dsquared_headers *.h)
FILE(GLOB halfedgemesh2dsquared_sources *.cpp)

SET(files_halfedgemesh2dsquared
	${halfedgemesh2dsquared_headers} 
	${halfedgemesh2dsquared_sources})

SOURCE_GROUP("halfedgemesh2dsquared" FILES ${files_halfedgemesh2dsquared})

ADD_LIBRARY(halfedgemesh2dsquared STATIC ${files_halfedgemesh2dsquared})
TARGET_LINK_LIBRARIES(halfedgemesh2dsquared mesh poolparty vector obj)
SET_TARGET_PROPERTIES(halfedgemesh2dsquared PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(halfedgemesh2dsquared PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(halfedgemesh2dsquared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})