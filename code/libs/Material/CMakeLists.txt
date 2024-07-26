#--------------------------------------------------------------------------
# material project
#--------------------------------------------------------------------------

PROJECT(material)
FILE(GLOB material_headers *.h)
FILE(GLOB material_sources *.cpp)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# expected behaviour
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++lastest")

SET(files_material
	${material_headers} 
	${material_sources})

SOURCE_GROUP("material" FILES ${files_material})

ADD_LIBRARY(material STATIC ${files_material})
TARGET_LINK_LIBRARIES(material material_profile shader graphics_storage render_pass)
SET_TARGET_PROPERTIES(material PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(material PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(material PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})