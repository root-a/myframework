#--------------------------------------------------------------------------
# editor project
#--------------------------------------------------------------------------

PROJECT(editor)
FILE(GLOB editor_headers *.h)
FILE(GLOB editor_sources *.cpp)

SET(files_editor
	${editor_headers} 
	${editor_sources})

SOURCE_GROUP("editor" FILES ${files_editor})

ADD_LIBRARY(editor STATIC ${files_editor})
TARGET_LINK_LIBRARIES(editor glm material object texture script imgui_wrapper imguizmo scene_graph light rigidbody bounds graphics_storage graphics_manager lua_tools vao material_profile texture_profile render_profile render_pass fbo_manager times render debug_draw physics_manager glfw camera_manager camera)
SET_TARGET_PROPERTIES(editor PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(editor PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})