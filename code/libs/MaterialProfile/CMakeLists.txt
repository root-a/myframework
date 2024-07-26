#--------------------------------------------------------------------------
# material_profile project
#--------------------------------------------------------------------------

PROJECT(material_profile)
FILE(GLOB material_profile_headers *.h)
FILE(GLOB material_profile_sources *.cpp)

SET(files_material_profile
	${material_profile_headers} 
	${material_profile_sources})

SOURCE_GROUP("material_profile" FILES ${files_material_profile})

ADD_LIBRARY(material_profile STATIC ${files_material_profile})
TARGET_LINK_LIBRARIES(material_profile object_profile)
SET_TARGET_PROPERTIES(material_profile PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(material_profile PROPERTIES FOLDER "MyLibs/Rendering")
TARGET_INCLUDE_DIRECTORIES(material_profile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})