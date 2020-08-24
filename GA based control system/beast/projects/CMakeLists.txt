SET(wxWidgets_USE_LIBS base core gl net)
FIND_PACKAGE(wxWidgets)
if(wxWidgets_FOUND)
	INCLUDE(${wxWidgets_USE_FILE})
	include_directories (${BEAST_SOURCE_DIR}/libs ${wxWidgets_INCLUDE_DIRS} )
#	link_directories (${BEAST_BINARY_DIR}/libs)

#    	add_executable(beast wxbeast.cc screengrab.cc beastframe.cc worldglcanvas.cc)

#target_link_libraries(beast BEASTLIB ${wxWidgets_LIBRARIES} ${OPENGL_gl_LIBRARY} ${GLUT_glut_LIBRARY} ${GLUT_cocoa_LIBRARY} glut)

else(wxWidgets_FOUND)
	Message("wxWidgets not found!")
endif(wxWidgets_FOUND)

set( beast_SRC
#     ant1.cc           
#     bacteria.cc   
#     mouse.cc     
#     shrew.cc
#     ant.cc            
#     braitenberg.cc            
#     pacman.cc    
#     signal.cc
#     baccyexamples.cc  
#     chase.cc   
#     cluster01.cc     
    demos.cc        
#     poolbots.cc  
#     sigtest.cc
)

add_library (demos SHARED  demos.cc)
add_library (mouse SHARED  mouse.cc)
