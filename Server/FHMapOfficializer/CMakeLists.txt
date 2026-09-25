cmake_minimum_required(VERSION 3.10)

add_executable(FHMapOfficializer
	main.cpp
	md5.cpp
)
target_include_directories(FHMapOfficializer PRIVATE .)
