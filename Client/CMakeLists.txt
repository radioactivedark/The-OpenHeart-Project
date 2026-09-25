cmake_minimum_required(VERSION 3.5)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
project(OpenHeartClient CXX)

if(WIN32)
	enable_language(RC)
endif()

set(DEP_CLIENT "${CMAKE_CURRENT_SOURCE_DIR}/../.Dependencies/Client")
set(DEP_SHARED "${CMAKE_CURRENT_SOURCE_DIR}/../.Dependencies/Shared")

if(WIN32 AND NOT CMAKE_CXX_COMPILER)
	set(BUNDLED_MINGW_BIN "${DEP_CLIENT}/MinGW/bin")
	if(EXISTS "${BUNDLED_MINGW_BIN}/mingw32-g++.exe")
		set(CMAKE_C_COMPILER "${BUNDLED_MINGW_BIN}/gcc.exe" CACHE FILEPATH "" FORCE)
		set(CMAKE_CXX_COMPILER "${BUNDLED_MINGW_BIN}/mingw32-g++.exe" CACHE FILEPATH "" FORCE)
		set(CMAKE_MAKE_PROGRAM "${BUNDLED_MINGW_BIN}/mingw32-make.exe" CACHE FILEPATH "" FORCE)
	endif()
endif()

foreach(_cfg "" "_DEBUG" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${_cfg} "${CMAKE_CURRENT_SOURCE_DIR}/RELEASE")
endforeach()

set(DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(DISABLE_DLL ON CACHE BOOL "" FORCE)
set(DISABLE_STATICLIB OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEP_SHARED}/Libraries/RakNet4" "${CMAKE_CURRENT_BINARY_DIR}/RakNet4")

set(ZLIB_ROOT "${DEP_CLIENT}/Libraries/zlib")
find_path(ZLIB_INCLUDE_DIR NAMES zlib.h
	PATHS "${ZLIB_ROOT}" "${ZLIB_ROOT}/include" "${DEP_CLIENT}/MinGW/include")
find_library(ZLIB_LIBRARY NAMES z libz zdll
	PATHS "${ZLIB_ROOT}/lib" "${DEP_CLIENT}/MinGW/lib")
find_package(ZLIB REQUIRED)

set(OGRE_SDK "${DEP_CLIENT}/OgreSDK")

find_path(OGRE_INCLUDE_DIR NAMES Ogre.h
	PATHS "${OGRE_SDK}/include" "${OGRE_SDK}/include/OGRE")
find_path(OIS_INCLUDE_DIR NAMES OIS.h
	PATHS "${OGRE_SDK}/include" "${OGRE_SDK}/include/OIS")

find_library(OGRE_LIBRARY NAMES OgreMain libOgreMain
	PATHS "${OGRE_SDK}/lib" "${OGRE_SDK}/bin/Release" "${OGRE_SDK}/lib/Release")
find_library(OIS_LIBRARY NAMES OIS libOIS
	PATHS "${OGRE_SDK}/lib" "${OGRE_SDK}/bin/Release" "${OGRE_SDK}/lib/Release")

if(NOT OGRE_INCLUDE_DIR OR NOT OGRE_LIBRARY)
	message(FATAL_ERROR "Ogre 1.7.x SDK not found under ${OGRE_SDK} - drop the "
		"MinGW Ogre 1.7.x (Cthugha) SDK there (replacing the placeholder "
		"version) before configuring.")
endif()
if(NOT OIS_INCLUDE_DIR OR NOT OIS_LIBRARY)
	message(FATAL_ERROR "OIS not found under ${OGRE_SDK} - it ships bundled "
		"with the Ogre MinGW SDK.")
endif()

if(EXISTS "${OGRE_SDK}/boost_1_47")
	set(BOOST_INCLUDE_DIR "${OGRE_SDK}/boost_1_47")
elseif(EXISTS "${OGRE_SDK}/boost_1_44")
	set(BOOST_INCLUDE_DIR "${OGRE_SDK}/boost_1_44")
endif()

set(IRRKLANG_ROOT "${DEP_CLIENT}/Libraries/irrKlang-1.3.0b/irrKlang-1.3.0")
find_path(IRRKLANG_INCLUDE_DIR NAMES irrKlang.h PATHS "${IRRKLANG_ROOT}/include")
find_library(IRRKLANG_LIBRARY NAMES irrKlang libirrKlang
	PATHS "${IRRKLANG_ROOT}/lib/Win32-gcc")

if(NOT IRRKLANG_INCLUDE_DIR OR NOT IRRKLANG_LIBRARY)
	message(FATAL_ERROR "irrKlang not found under ${IRRKLANG_ROOT}")
endif()

set(SKYXHYDRAX_ROOT "${DEP_CLIENT}/Libraries/SkyxHydrax")
find_path(SKYX_INCLUDE_DIR NAMES SkyX/SkyX.h PATHS "${SKYXHYDRAX_ROOT}/include")
find_library(SKYX_LIBRARY NAMES SkyX libSkyX PATHS "${SKYXHYDRAX_ROOT}/lib")
find_library(HYDRAX_LIBRARY NAMES Hydrax libHydrax PATHS "${SKYXHYDRAX_ROOT}/lib")

if(NOT SKYX_INCLUDE_DIR OR NOT SKYX_LIBRARY OR NOT HYDRAX_LIBRARY)
	message(FATAL_ERROR "SkyX/Hydrax not found under ${SKYXHYDRAX_ROOT}")
endif()

add_executable(OpenHeart
	main.cpp
	include/md5.cpp
)
if(WIN32)
	target_sources(OpenHeart PRIVATE include/resource.rc)
endif()

target_include_directories(OpenHeart PRIVATE
	include
	${OGRE_INCLUDE_DIR}
	${OIS_INCLUDE_DIR}
	${IRRKLANG_INCLUDE_DIR}
	${SKYX_INCLUDE_DIR}
	${SKYXHYDRAX_ROOT}/include
	${DEP_SHARED}/Libraries/RakNet4/Source
	${DEP_CLIENT}/Libraries
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
	RakNetStatic
	ZLIB::ZLIB
)

if(WIN32)
	target_compile_definitions(OpenHeart PRIVATE WIN32 _WINDOWS NDEBUG _GLIBCXX_HAVE_GTHR_DEFAULT)
	target_compile_options(OpenHeart PRIVATE -mthreads -fmessage-length=0 -fexceptions -fident -O2)
	target_link_libraries(OpenHeart PRIVATE ws2_32 iphlpapi)
	target_link_options(OpenHeart PRIVATE -Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -s)
endif()

if(WIN32)
	file(GLOB OGRE_RUNTIME_DLLS "${OGRE_SDK}/bin/Release/*.dll")
	foreach(_dll ${OGRE_RUNTIME_DLLS})
		add_custom_command(TARGET OpenHeart POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "$<TARGET_FILE_DIR:OpenHeart>")
	endforeach()

	find_file(IRRKLANG_DLL NAMES irrKlang.dll
		PATHS "${IRRKLANG_ROOT}/bin/win32-gcc" "${IRRKLANG_ROOT}/bin")
	if(IRRKLANG_DLL)
		add_custom_command(TARGET OpenHeart POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IRRKLANG_DLL}" "$<TARGET_FILE_DIR:OpenHeart>")
	endif()

	find_file(ZLIB_RUNTIME_DLL NAMES libz-1.dll zlib1.dll
		PATHS "${ZLIB_ROOT}" "${ZLIB_ROOT}/bin" "${DEP_CLIENT}/MinGW/bin")
	if(ZLIB_RUNTIME_DLL)
		add_custom_command(TARGET OpenHeart POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ZLIB_RUNTIME_DLL}" "$<TARGET_FILE_DIR:OpenHeart>")
	endif()

	get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
	foreach(_rtdll libgcc_s_dw2-1.dll libstdc++-6.dll mingwm10.dll)
		if(EXISTS "${MINGW_BIN_DIR}/${_rtdll}")
			add_custom_command(TARGET OpenHeart POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MINGW_BIN_DIR}/${_rtdll}" "$<TARGET_FILE_DIR:OpenHeart>")
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
