#--------------------------------------------------------------------------
# asset_registry project
#--------------------------------------------------------------------------

PROJECT(asset_registry)
FILE(GLOB asset_registry_headers *.h)
FILE(GLOB asset_registry_sources *.cpp)

SET(files_asset_registry
	${asset_registry_headers} 
	${asset_registry_sources})

SOURCE_GROUP("asset_registry" FILES ${files_asset_registry})

ADD_LIBRARY(asset_registry STATIC ${files_asset_registry})
TARGET_LINK_LIBRARIES(asset_registry poolparty stduuid xoshiro-cpp shader_block)
SET_TARGET_PROPERTIES(asset_registry PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(asset_registry PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(asset_registry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})