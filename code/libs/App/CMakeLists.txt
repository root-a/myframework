#--------------------------------------------------------------------------
# app
#--------------------------------------------------------------------------

SET(files_app
	app.h
	app.cc)
SOURCE_GROUP("app" FILES ${files_app})
	

ADD_LIBRARY(app STATIC ${files_app})
SET_TARGET_PROPERTIES(app PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})