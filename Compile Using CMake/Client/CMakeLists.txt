##################################################################################################
##------------------------------------------FILE START------------------------------------------##
##################################################################################################


# Set project details.
cmake_minimum_required(VERSION 3.2)
project(OPNHRTCLI CXX)
if(WIN32)
	enable_language(RC)
endif()

# Set the primary directories.
set(CLI "${CMAKE_CURRENT_SOURCE_DIR}/../../../.Dependencies/Client")
set(SHARE "${CMAKE_CURRENT_SOURCE_DIR}/../../../.Dependencies/Shared")

# Set the dependency directories.
set(ZLIB "${CLI}/Libraries/zlib")
set(OGRE "${CLI}/OgreSDK")
set(SKYXHYDRAX "${CLI}/Libraries/SkyxHydrax")
set(IRRKLANG "${CLI}/Libraries/irrKlang-1.3.0b/irrKlang-1.3.0")
add_subdirectory("${SHARE}/Libraries/RakNet4" "${CMAKE_CURRENT_BINARY_DIR}/RakNet4")

# Find paths for specific deps.
find_path(ZLIB_INCLUDE_DIR NAMES zlib.h PATHS "${ZLIB}" "${ZLIB}/include" "${CLI}/MinGW/include")
find_path(OGRE_INCLUDE_DIR NAMES Ogre.h PATHS "${OGRE}/include" "${OGRE}/include/OGRE")
find_path(OIS_INCLUDE_DIR NAMES OIS.h PATHS "${OGRE}/include" "${OGRE}/include/OIS")
find_path(IRRKLANG_INCLUDE_DIR NAMES irrKlang.h PATHS "${IRRKLANG}/include")
find_path(SKYX_INCLUDE_DIR NAMES SkyX/SkyX.h PATHS "${SKYXHYDRAX}/include")

# Find libraries.
find_library(ZLIB_LIBRARY NAMES z libz zdll PATHS "${ZLIB}/lib" "${CLI}/MinGW/lib")
find_library(OGRE_LIBRARY NAMES OgreMain libOgreMain PATHS "${OGRE}/lib" "${OGRE}/bin/Release" "${OGRE}/lib/Release")
find_library(OIS_LIBRARY NAMES OIS libOIS PATHS "${OGRE}/lib" "${OGRE}/bin/Release" "${OGRE}/lib/Release")
find_library(IRRKLANG_LIBRARY NAMES irrKlang libirrKlang PATHS "${IRRKLANG}/lib/Win32-gcc")
find_library(SKYX_LIBRARY NAMES SkyX libSkyX PATHS "${SKYXHYDRAX}/lib")
find_library(HYDRAX_LIBRARY NAMES Hydrax libHydrax PATHS "${SKYXHYDRAX}/lib")

# Instructions for RakNet.
set(DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(DISABLE_DLL ON CACHE BOOL "" FORCE)
set(DISABLE_STATICLIB OFF CACHE BOOL "" FORCE)

# Sets the MINGW Compiler.
if(WIN32 AND NOT CMAKE_CXX_COMPILER)
	set(MINGW "${CLI}/MinGW/bin")
	if(EXISTS "${MINGW}/mingw32-g++.exe")
		set(CMAKE_C_COMPILER "${MINGW}/gcc.exe" CACHE FILEPATH "" FORCE)
		set(CMAKE_CXX_COMPILER "${MINGW}/mingw32-g++.exe" CACHE FILEPATH "" FORCE)
		set(CMAKE_MAKE_PROGRAM "${MINGW}/mingw32-make.exe" CACHE FILEPATH "" FORCE)
	endif()
endif()

# Telling CMake to send everything to the RELEASE folder.
foreach(_cfg "" "_DEBUG" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${_cfg} "${CMAKE_CURRENT_SOURCE_DIR}/RELEASE")
endforeach()

# File errors that stop compilation if deps are missing.
if(NOT OGRE_INCLUDE_DIR OR NOT OGRE_LIBRARY)
	message(FATAL_ERROR "Ogre SDK not found under ${OGRE}")
endif()
if(NOT OIS_INCLUDE_DIR OR NOT OIS_LIBRARY)
	message(FATAL_ERROR "OIS not found under ${OGRE}")
endif()
if(EXISTS "${OGRE}/boost_1_47")
	set(BOOST_INCLUDE_DIR "${OGRE}/boost_1_47")
elseif(EXISTS "${OGRE}/boost_1_44")
	set(BOOST_INCLUDE_DIR "${OGRE}/boost_1_44")
endif()
if(NOT IRRKLANG_INCLUDE_DIR OR NOT IRRKLANG_LIBRARY)
	message(FATAL_ERROR "irrKlang not found under ${IRRKLANG}")
endif()
if(NOT SKYX_INCLUDE_DIR OR NOT SKYX_LIBRARY OR NOT HYDRAX_LIBRARY)
	message(FATAL_ERROR "SkyX Hydrax not found under ${SKYXHYDRAX}")
endif()

# Start building executable.
add_executable(OpenHeart
	main.cpp
	include/md5.cpp
)
if(WIN32)
	target_sources(OpenHeart PRIVATE include/resource.rc)
endif()

# Directories to include for building.
target_include_directories(OpenHeart PRIVATE
	include
	${OGRE_INCLUDE_DIR}
	${OIS_INCLUDE_DIR}
	${IRRKLANG_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${SKYX_INCLUDE_DIR}
	${SKYXHYDRAX}/include
	${SHARE}/Libraries/RakNet4/Source
	${CLI}/Libraries
)
if(BOOST_INCLUDE_DIR)
	target_include_directories(OpenHeart PRIVATE ${BOOST_INCLUDE_DIR})
endif()
target_link_libraries(OpenHeart PRIVATE
	${OGRE_LIBRARY}
	${OIS_LIBRARY}
	${IRRKLANG_LIBRARY}
	${SKYX_LIBRARY}
	${HYDRAX_LIBRARY}
	${ZLIB_LIBRARY}
	RakNetStatic
)

# Compile executable.
if(WIN32)
	target_compile_definitions(OpenHeart PRIVATE WIN32 _WINDOWS NDEBUG _GLIBCXX_HAVE_GTHR_DEFAULT)
	target_compile_options(OpenHeart PRIVATE -mthreads -fmessage-length=0 -fexceptions -fident -O2)
	target_link_libraries(OpenHeart PRIVATE ws2_32 iphlpapi)
	target_link_options(OpenHeart PRIVATE -Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -s)
endif()

# Add DLLs to EXE.
if(WIN32)
	file(GLOB OGRE_RUNTIME_DLLS "${OGRE}/bin/Release/*.dll")
	foreach(_dll ${OGRE_RUNTIME_DLLS})
		add_custom_command(TARGET OpenHeart POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "$<TARGET_FILE_DIR:OpenHeart>")
	endforeach()
	find_file(IRRKLANG_DLL NAMES irrKlang.dll
		PATHS "${IRRKLANG}/bin/win32-gcc" "${IRRKLANG}/bin")
	if(IRRKLANG_DLL)
		add_custom_command(TARGET OpenHeart POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IRRKLANG_DLL}" "$<TARGET_FILE_DIR:OpenHeart>")
	endif()
	find_file(ZLIB_RUNTIME_DLL NAMES libz-1.dll zlib1.dll
		PATHS "${ZLIB}" "${ZLIB}/bin" "${CLI}/MinGW/bin")
	if(ZLIB_RUNTIME_DLL)
		add_custom_command(TARGET OpenHeart POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ZLIB_RUNTIME_DLL}" "$<TARGET_FILE_DIR:OpenHeart>")
	endif()
	get_filename_component(MINGW_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
	foreach(_rtdll libgcc_s_dw2-1.dll libstdc++-6.dll mingwm10.dll)
		if(EXISTS "${MINGW_DIR}/${_rtdll}")
			add_custom_command(TARGET OpenHeart POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MINGW_DIR}/${_rtdll}" "$<TARGET_FILE_DIR:OpenHeart>")
		endif()
	endforeach()
	file(GLOB CLIENT_RES_ITEMS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/res" "${CMAKE_CURRENT_SOURCE_DIR}/res/*")
	foreach(_name ${CLIENT_RES_ITEMS})
		if(NOT _name STREQUAL "fh.ico")
			set(_src "${CMAKE_CURRENT_SOURCE_DIR}/res/${_name}")
			if(IS_DIRECTORY "${_src}")
				add_custom_command(TARGET OpenHeart POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory "${_src}" "$<TARGET_FILE_DIR:OpenHeart>/${_name}")
			else()
				add_custom_command(TARGET OpenHeart POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "$<TARGET_FILE_DIR:OpenHeart>")
			endif()
		endif()
	endforeach()
endif()


##################################################################################################
##-------------------------------------------FILE END-------------------------------------------##
##################################################################################################
##-------------------------------------------MODIFIED-------------------------------------------##
## DATE: 9/26/2026                                                                              ##
## TIME: 12:02 AM (UTC-5:00)                                                                    ##
## BY: radioactivedark                                                                          ##
##################################################################################################