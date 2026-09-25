cmake_minimum_required(VERSION 2.6)
project(RakNetStatic)

FILE(GLOB ALL_HEADER_SRCS ${RakNet_SOURCE_DIR}/Source/*.h)
FILE(GLOB ALL_CPP_SRCS ${RakNet_SOURCE_DIR}/Source/*.cpp)

include_directories(${RakNet_SOURCE_DIR}/Source)
IF(WIN32 AND NOT UNIX)
	add_library(RakNetStatic STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS} readme.txt)
ELSE(WIN32 AND NOT UNIX)
	add_library(RakNetStatic STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS} readme.txt)
ENDIF(WIN32 AND NOT UNIX)

IF(NOT WIN32)
	# Old RakNet4 source (e.g. OFFLINE_MESSAGE_DATA_ID in RakPeer.cpp) initializes
	# signed char arrays with values >127, which modern GCC/Clang treat as a hard
	# narrowing-conversion error instead of the old implicit-wrap warning.
	target_compile_options(RakNetStatic PRIVATE -Wno-narrowing -fpermissive)
ENDIF(NOT WIN32)

IF(WIN32 AND NOT UNIX)
	SET(CMAKE_CXX_FLAGS "-D WIN32 -D _RAKNET_LIB -D _CRT_NONSTDC_NO_DEPRECATE -D _CRT_SECURE_NO_DEPRECATE")
ENDIF(WIN32 AND NOT UNIX)

IF(WIN32 AND NOT UNIX)
	target_link_libraries (RakNetStatic ${RAKNET_LIBRARY_LIBS})

	IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
			set_target_properties(RakNetStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:\"LIBCD.lib LIBCMTD.lib MSVCRT.lib\"")
		ELSE(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
			set_target_properties(RakNetStatic PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:&quot;LIBCD.lib LIBCMTD.lib MSVCRT.lib&quot;")
		ENDIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
	ENDIF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

ELSE(WIN32 AND NOT UNIX)
	target_link_libraries (RakNetStatic ${RAKNET_LIBRARY_LIBS})
	INSTALL(TARGETS RakNetStatic DESTINATION ${RakNet_SOURCE_DIR}/Lib/LibStatic)
	INSTALL(FILES ${ALL_HEADER_SRCS} DESTINATION ${RakNet_SOURCE_DIR}/include/raknet)
ENDIF(WIN32 AND NOT UNIX)
