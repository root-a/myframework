#--------------------------------------------------------------------------
# imguizmo project
#--------------------------------------------------------------------------

PROJECT(imguizmo)
FILE(GLOB imguizmo_headers "*.h")
FILE(GLOB imguizmo_sources "*.cpp")

SET(files_imguizmo
	${imguizmo_headers} 
	${imguizmo_sources})

SOURCE_GROUP("imguizmo" FILES ${files_imguizmo})

ADD_LIBRARY(imguizmo STATIC ${files_imguizmo})
SET_TARGET_PROPERTIES(imguizmo PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(imguizmo PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(imguizmo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} imgui)
