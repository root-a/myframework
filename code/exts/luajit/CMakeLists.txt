#--------------------------------------------------------------------------
# LuaJIT project
#--------------------------------------------------------------------------

PROJECT(luajit)
INCLUDE_DIRECTORIES("src")

# Define the output files for the custom commands
if (WIN32)
    set(LUAJIT_OUTPUT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/luajit.lib)
    set(LUAJIT_OUTPUT_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/luajit.h)
    set(LUA51_OUTPUT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/lua51.lib)
else()
    set(LUAJIT_OUTPUT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/libluajit.a)
    set(LUAJIT_OUTPUT_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/luajit.h)
    set(LUA51_OUTPUT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/liblua51.a)
endif()

# Determine the build type and set the appropriate command options
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(MSVC_BUILD_OPTIONS "debug static")
else()
    set(MSVC_BUILD_OPTIONS "static")
endif()

# Custom command to build LuaJIT on Windows and Unix
if (WIN32)
    add_custom_command(
        OUTPUT ${LUAJIT_OUTPUT_HEADER} ${LUAJIT_OUTPUT_LIB} ${LUA51_OUTPUT_LIB}
        COMMAND cmd /c "msvcbuild.bat ${MSVC_BUILD_OPTIONS}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Building LuaJIT with MSVC and generating headers on Windows..."
    )
else()
    add_custom_command(
        OUTPUT ${LUAJIT_OUTPUT_HEADER} ${LUAJIT_OUTPUT_LIB} ${LUA51_OUTPUT_LIB}
        COMMAND make
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Building LuaJIT and generating headers on Unix..."
    )
endif()


# Custom target to build LuaJIT
add_custom_target(luajit-build ALL
    DEPENDS ${LUAJIT_OUTPUT_HEADER} ${LUAJIT_OUTPUT_LIB} ${LUA51_OUTPUT_LIB}
)
set_target_properties(luajit-build PROPERTIES FOLDER "exts")

# Create a static library for LuaJIT
add_library(luajit STATIC IMPORTED GLOBAL)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION ${LUAJIT_OUTPUT_LIB}
)
add_dependencies(luajit luajit-build)
set_target_properties(luajit PROPERTIES FOLDER "exts")

# Check for lua51.lib and link if necessary
if (WIN32)
    find_library(LUA51_LIBRARY NAMES lua51 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ NO_DEFAULT_PATH)
else()
    find_library(LUA51_LIBRARY NAMES lua51 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ NO_DEFAULT_PATH)
endif()

if (LUA51_LIBRARY)
    message(STATUS "Found lua51.lib: ${LUA51_LIBRARY}")
    target_link_libraries(luajit INTERFACE ${LUA51_LIBRARY})
endif()

# Set include directories for LuaJIT
target_include_directories(luajit INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

