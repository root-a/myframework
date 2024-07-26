#--------------------------------------------------------------------------
# exts
#--------------------------------------------------------------------------

ADD_SUBDIRECTORY(glew)
SET_TARGET_PROPERTIES(glew PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(glew INTERFACE glew/include)

SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
SET(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")		  

ADD_SUBDIRECTORY(glfw)
SET_TARGET_PROPERTIES(glfw PROPERTIES FOLDER "exts/glfw")
SET_TARGET_PROPERTIES(update_mappings PROPERTIES FOLDER "exts/glfw")
SET_TARGET_PROPERTIES(glfw PROPERTIES LINKER_LANGUAGE CXX)

TARGET_INCLUDE_DIRECTORIES(glfw INTERFACE glfw/include)

ADD_SUBDIRECTORY(glm)
ADD_SUBDIRECTORY(stb_soil2)
ADD_SUBDIRECTORY(imgui)
ADD_SUBDIRECTORY(imguizmo)
ADD_SUBDIRECTORY(luajit)
ADD_SUBDIRECTORY(stduuid)
ADD_SUBDIRECTORY(xoshiro-cpp)