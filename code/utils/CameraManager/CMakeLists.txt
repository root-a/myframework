#--------------------------------------------------------------------------
# camera_manager project
#--------------------------------------------------------------------------

PROJECT(camera_manager)
FILE(GLOB camera_manager_headers *.h)
FILE(GLOB camera_manager_sources *.cpp)

SET(files_camera_manager
	${camera_manager_headers} 
	${camera_manager_sources})

SOURCE_GROUP("camera_manager" FILES ${files_camera_manager})

ADD_LIBRARY(camera_manager STATIC ${files_camera_manager})
TARGET_LINK_LIBRARIES(camera_manager mymathlib camera time)
SET_TARGET_PROPERTIES(camera_manager PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(camera_manager PROPERTIES FOLDER "MyUtils")
TARGET_INCLUDE_DIRECTORIES(camera_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})