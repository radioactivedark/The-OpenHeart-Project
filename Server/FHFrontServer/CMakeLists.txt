cmake_minimum_required(VERSION 3.10)

add_executable(FHFrontServer include/main.cpp)

target_include_directories(FHFrontServer PRIVATE
	include
	${DEP_SHARED}/Libraries/RakNet4/Source
	${MYSQLPP_INCLUDE_DIR}
	${MYSQL_INCLUDE_DIR}
)

target_link_libraries(FHFrontServer PRIVATE
	RakNetStatic
	${MYSQLPP_LIBRARY}
	${MYSQL_LIBRARY}
)
