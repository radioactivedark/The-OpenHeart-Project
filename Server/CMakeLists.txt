cmake_minimum_required(VERSION 3.10)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
project(OpenHeartServer CXX)

foreach(_cfg "" "_DEBUG" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${_cfg} "${CMAKE_CURRENT_SOURCE_DIR}/RELEASE")
endforeach()

set(DEP_SHARED "${CMAKE_CURRENT_SOURCE_DIR}/../.Dependencies/Shared")
set(DEP_SERVER "${CMAKE_CURRENT_SOURCE_DIR}/../.Dependencies/Server")

set(DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(DISABLE_DLL ON CACHE BOOL "" FORCE)
set(DISABLE_STATICLIB OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEP_SHARED}/Libraries/RakNet4" "${CMAKE_CURRENT_BINARY_DIR}/RakNet4")

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
	pkg_check_modules(MYSQLPP_PC QUIET mysql++)
endif()

find_path(MYSQLPP_INCLUDE_DIR NAMES mysql++.h
	HINTS ${MYSQLPP_PC_INCLUDE_DIRS}
	PATHS /usr/include/mysql++ /usr/local/include/mysql++ /usr/include /usr/local/include)
find_library(MYSQLPP_LIBRARY NAMES mysqlpp
	HINTS ${MYSQLPP_PC_LIBRARY_DIRS}
	PATHS "${DEP_SERVER}/Libraries")

find_path(MYSQL_INCLUDE_DIR NAMES mysql.h
	PATH_SUFFIXES mysql mariadb
	PATHS /usr/include /usr/local/include)
find_library(MYSQL_LIBRARY NAMES mysqlclient mariadbclient mariadb)

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

add_subdirectory(FHFrontServer)
add_subdirectory(FHMainServer)
add_subdirectory(FHMapServer)
add_subdirectory(FHMapOfficializer)
