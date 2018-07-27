#--------------------------------------------------------------------------
# rigidbody project
#--------------------------------------------------------------------------

PROJECT(rigidbody)
FILE(GLOB rigidbody_headers *.h)
FILE(GLOB rigidbody_sources *.cpp)

SET(files_rigidbody
	${rigidbody_headers} 
	${rigidbody_sources})

SOURCE_GROUP("rigidbody" FILES ${files_rigidbody})

ADD_LIBRARY(rigidbody STATIC ${files_rigidbody})
TARGET_LINK_LIBRARIES(rigidbody mymathlib object component physics_manager times)
SET_TARGET_PROPERTIES(rigidbody PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(rigidbody PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(rigidbody PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})