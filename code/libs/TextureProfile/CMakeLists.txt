#--------------------------------------------------------------------------
# texture_profile project
#--------------------------------------------------------------------------

PROJECT(texture_profile)
FILE(GLOB texture_profile_headers *.h)
FILE(GLOB texture_profile_sources *.cpp)

SET(files_texture_profile
	${texture_profile_headers} 
	${texture_profile_sources})

SOURCE_GROUP("texture_profile" FILES ${files_texture_profile})

ADD_LIBRARY(texture_profile STATIC ${files_texture_profile})
TARGET_LINK_LIBRARIES(texture_profile render_element texture)
SET_TARGET_PROPERTIES(texture_profile PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(texture_profile PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(texture_profile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})