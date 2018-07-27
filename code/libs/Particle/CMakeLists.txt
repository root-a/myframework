#--------------------------------------------------------------------------
# particle project
#--------------------------------------------------------------------------

PROJECT(particle)
FILE(GLOB particle_headers *.h)
FILE(GLOB particle_sources *.cpp)

SET(files_particle
	${particle_headers} 
	${particle_sources})

SOURCE_GROUP("particle" FILES ${files_particle})

ADD_LIBRARY(particle STATIC ${files_particle})
TARGET_LINK_LIBRARIES(particle mymathlib component object camera_manager gl_core)
SET_TARGET_PROPERTIES(particle PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(particle PROPERTIES FOLDER "MyLibs")
TARGET_INCLUDE_DIRECTORIES(particle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})