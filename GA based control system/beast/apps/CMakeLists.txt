SET(wxWidgets_USE_LIBS base core gl net)
FIND_PACKAGE(wxWidgets)

INCLUDE(FindOpenGL FindGLUT)          

if(wxWidgets_FOUND)

	INCLUDE(${wxWidgets_USE_FILE})
	include_directories (${BEAST_SOURCE_DIR}/libs ${wxWidgets_INCLUDE_DIRS} )
	link_directories (${BEAST_BINARY_DIR}/libs)

    	add_executable(beast wxbeast.cc screengrab.cc beastframe.cc worldglcanvas.cc)

    	target_link_libraries(beast BEASTLIB ${wxWidgets_LIBRARIES} ${OPENGL_gl_LIBRARY} ${GLUT_glut_LIBRARY} ${GLUT_cocoa_LIBRARY} glut GLU)

else(wxWidgets_FOUND)
	Message("wxWidgets not found!")
endif(wxWidgets_FOUND)
