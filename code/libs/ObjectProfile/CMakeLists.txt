#--------------------------------------------------------------------------
# object_profile project
#--------------------------------------------------------------------------

PROJECT(object_profile)
FILE(GLOB object_profile_headers *.h)
FILE(GLOB object_profile_sources *.cpp)

SET(files_object_profile
	${object_profile_headers} 
	${object_profile_sources})

SOURCE_GROUP("object_profile" FILES ${files_object_profile})

ADD_LIBRARY(object_profile STATIC ${files_object_profile})
TARGET_LINK_LIBRARIES(object_profile render_element shader_block component)
SET_TARGET_PROPERTIES(object_profile PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(object_profile PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(object_profile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})