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
SET_TARGET_PROPERTIES(glfw PROPERTIES FOLDER "exts")
TARGET_INCLUDE_DIRECTORIES(glfw INTERFACE glfw/include)

ADD_SUBDIRECTORY(dirent)
ADD_SUBDIRECTORY(stb)
ADD_SUBDIRECTORY(vector)
ADD_SUBDIRECTORY(imgui)

ADD_SUBDIRECTORY(luajit)
TARGET_INCLUDE_DIRECTORIES(luajit INTERFACE luajit/include luajit/lib)
