#--------------------------------------------------------------------------
# scene_graph project
#--------------------------------------------------------------------------

PROJECT(scene_graph)
FILE(GLOB scene_graph_headers *.h)
FILE(GLOB scene_graph_sources *.cpp)

SET(files_scene_graph
	${scene_graph_headers} 
	${scene_graph_sources})

SOURCE_GROUP("scene_graph" FILES ${files_scene_graph})

ADD_LIBRARY(scene_graph STATIC ${files_scene_graph})
TARGET_LINK_LIBRARIES(scene_graph graphics_storage physics_manager light poolparty drawables_systems)
SET_TARGET_PROPERTIES(scene_graph PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(scene_graph PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(scene_graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})