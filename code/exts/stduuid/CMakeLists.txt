#--------------------------------------------------------------------------
# stduuid project
#--------------------------------------------------------------------------

option(UUID_BUILD_TESTS "Build the unit tests" OFF)
option(UUID_SYSTEM_GENERATOR "Enable operating system uuid generator" OFF)
option(UUID_TIME_GENERATOR "Enable experimental time-based uuid generator" OFF)
option(UUID_USING_CXX20_SPAN "Using span from std instead of gsl" ON)

PROJECT(stduuid)
FILE(GLOB stduuid_headers "${PROJECT_NAME}/include/*.h")
FILE(GLOB stduuid_sources "${PROJECT_NAME}/include/*.c")


SET(files_stduuid
	${stduuid_headers} 
	${stduuid_sources})

SOURCE_GROUP("stduuid" FILES ${files_stduuid})

ADD_LIBRARY(${PROJECT_NAME} STATIC ${files_stduuid})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES FOLDER "exts")

if (UUID_USING_CXX20_SPAN)
    LIST(APPEND includes ${PROJECT_NAME}/include ${PROJECT_SOURCE_DIR}/catch)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${includes}>)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
else ()
    LIST(APPEND includes ${PROJECT_NAME}/include ${PROJECT_NAME}/catch ${PROJECT_NAME}/gsl)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${includes}>)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
endif ()
if (WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /EHc)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _SCL_SECURE_NO_WARNINGS)
elseif (APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions -g -Wall)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions -g -Wall)
endif ()
get_target_property(CURRENT_COMPILE_OPTIONS ${PROJECT_NAME} COMPILE_OPTIONS)
message(STATUS "** ${CMAKE_CXX_COMPILER_ID} flags: ${CURRENT_COMPILE_OPTIONS}")