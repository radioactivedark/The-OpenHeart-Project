##################################################################################################
##------------------------------------------FILE START------------------------------------------##
##################################################################################################


# Set the target system.
set(CMAKE_SYSTEM_NAME Windows)

# Set the primary directories.
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
set(MINGW "${PROJECT_ROOT}/../.Dependencies/Client/MinGW/bin")

# Set MinGW as compiler.
set(CMAKE_C_COMPILER
	"${MINGW}/gcc.exe"
	CACHE FILEPATH "MinGW C compiler"
)
set(CMAKE_CXX_COMPILER
	"${MINGW}/mingw32-g++.exe"
	CACHE FILEPATH "MinGW C++ compiler"
)

# Set Windows resource compiler.
set(CMAKE_RC_COMPILER
	"${MINGW}/windres.exe"
	CACHE FILEPATH "MinGW resource compiler"
)

# Set MinGW build program.
set(CMAKE_MAKE_PROGRAM
	"${MINGW}/mingw32-make.exe"
	CACHE FILEPATH "MinGW Make program"
)

# Set dep search directory.
set(CMAKE_FIND_ROOT_PATH
	"${PROJECT_ROOT}/../.Dependencies/Client"
)

# Set dep search behavior.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


##################################################################################################
##-------------------------------------------FILE END-------------------------------------------##
##################################################################################################
##-------------------------------------------MODIFIED-------------------------------------------##
## DATE: 9/25/2026                                                                              ##
## TIME: 2:38 PM (UTC-5:00)                                                                     ##
## BY: radioactivedark                                                                          ##
##################################################################################################