#--------------------------------------------------------------------------
# deps
#--------------------------------------------------------------------------

FILE(GLOB children RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
FOREACH(child ${children})
	IF(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${child})
		ADD_SUBDIRECTORY(${child})
	ENDIF()
ENDFOREACH()

ADD_LIBRARY(gl_core INTERFACE)
TARGET_LINK_LIBRARIES(gl_core INTERFACE glew ${OPENGL_LIBS})