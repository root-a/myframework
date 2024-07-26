#--------------------------------------------------------------------------
# render_profile project
#--------------------------------------------------------------------------

PROJECT(render_profile)
FILE(GLOB render_profile_headers *.h)
FILE(GLOB render_profile_sources *.cpp)

SET(files_render_profile
	${render_profile_headers} 
	${render_profile_sources})

SOURCE_GROUP("render_profile" FILES ${files_render_profile})

ADD_LIBRARY(render_profile STATIC ${files_render_profile})
TARGET_LINK_LIBRARIES(render_profile script render_element)
SET_TARGET_PROPERTIES(render_profile PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(render_profile PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(render_profile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})