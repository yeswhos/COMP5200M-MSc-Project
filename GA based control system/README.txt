1. beast is the file for BEAST source code. The setp for installing and compiling beast as follow steps.
	'cd beast/build'. If the build directory is not there, simply create it: 'mkdir build.'
	Call CMake: 'ccmake  ..'
	Configure: 'c'
	It will ask you for the 'build type': press 'enter' and type 'Release' in the relevant field , then press 'enter' again
	Again configure:'c'
	Generate Makefile: 'g'
	type 'make'
	in the 'build' directory, there now is a directory 'aps'. From this directory get the file called 'beast'.
	In the "build" directory you are currently in there are two new directories called "apps" and "projects". The "apps" directory contains the executable for the beast program called "beast" and the "projects" directory contain the required dynamic library needed to run the program called "libdemos.so".
 	From the build directory run the beast executable and pass the path to the dynamic library as a prameter as follows "./apps/beast ./projects/libdemos.so"
	When inside the program verify that you have a working build by going to "File" and then "Start simulation: Shrew"
The main code for this project is in 'beast/projects/shrew.cc'
2. Demo file store the python codes for plotting.           
