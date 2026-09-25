##################################################################################################
##------------------------------------------FILE START------------------------------------------##
##################################################################################################


# Set project details.
cmake_minimum_required(VERSION 3.10)
project(OPNHRTSRV CXX)

# Set the primary directories.
set(SHARE "${CMAKE_CURRENT_SOURCE_DIR}/../.Dependencies/Shared")
set(SRV "${CMAKE_CURRENT_SOURCE_DIR}/../.Dependencies/Server")

# Telling CMake to send everything to the RELEASE folder.
foreach(_cfg "" "_DEBUG" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${_cfg} "${CMAKE_CURRENT_SOURCE_DIR}/RELEASE")
endforeach()

# Instructions for RakNet.
set(DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(DISABLE_DLL ON CACHE BOOL "" FORCE)
set(DISABLE_STATICLIB OFF CACHE BOOL "" FORCE)

# Find MYSQL.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
	pkg_check_modules(MYSQLPP_PC QUIET mysql++)
endif()

# Find paths for MYSQL.
find_path(MYSQLPP_INCLUDE_DIR NAMES mysql++.h HINTS ${MYSQLPP_PC_INCLUDE_DIRS} PATHS /usr/include/mysql++ /usr/local/include/mysql++ /usr/include /usr/local/include)
find_path(MYSQL_INCLUDE_DIR NAMES mysql.h PATH_SUFFIXES mysql mariadb PATHS /usr/include /usr/local/include)

# Find libraries for MYSQL.
find_library(MYSQLPP_LIBRARY NAMES mysqlpp HINTS ${MYSQLPP_PC_LIBRARY_DIRS} PATHS "${SRV}/Libraries")
find_library(MYSQL_LIBRARY NAMES mysqlclient mariadbclient mariadb)

# Errors that stop compilation if MYSQL is missing.
if(NOT MYSQLPP_INCLUDE_DIR OR NOT MYSQLPP_LIBRARY)
	message(FATAL_ERROR "mysql++ not found. Install it first, e.g. on "
		"Debian/Ubuntu: apt install libmysql++-dev "
		"(or set MYSQLPP_INCLUDE_DIR / MYSQLPP_LIBRARY manually).")
endif()
if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
	message(FATAL_ERROR "libmysqlclient (or libmariadbclient) not found. "
		"Install a MySQL/MariaDB client dev package, e.g. "
		"apt install default-libmysqlclient-dev.")
endif()

# Set subdirectories.
add_subdirectory("${SHARE}/Libraries/RakNet4" "${CMAKE_CURRENT_BINARY_DIR}/RakNet4")
add_subdirectory(FHFrontServer)
add_subdirectory(FHMainServer)
add_subdirectory(FHMapServer)
add_subdirectory(FHMapOfficializer)


##################################################################################################
##-------------------------------------------FILE END-------------------------------------------##
##################################################################################################
##-------------------------------------------MODIFIED-------------------------------------------##
## DATE: 9/25/2026                                                                              ##
## TIME: 2:27 PM (UTC-5:00)                                                                     ##
## BY: radioactivedark                                                                          ##
##################################################################################################