set(CMAKE_SYSTEM_NAME Windows)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
set(BUNDLED_MINGW "${PROJECT_ROOT}/../.Dependencies/Client/MinGW/bin")

set(CMAKE_C_COMPILER
    "${BUNDLED_MINGW}/gcc.exe"
    CACHE FILEPATH "Bundled MinGW C compiler"
)

set(CMAKE_CXX_COMPILER
    "${BUNDLED_MINGW}/mingw32-g++.exe"
    CACHE FILEPATH "Bundled MinGW C++ compiler"
)

set(CMAKE_RC_COMPILER
    "${BUNDLED_MINGW}/windres.exe"
    CACHE FILEPATH "Bundled MinGW resource compiler"
)

set(CMAKE_MAKE_PROGRAM
    "${BUNDLED_MINGW}/mingw32-make.exe"
    CACHE FILEPATH "Bundled MinGW Make program"
)

set(CMAKE_FIND_ROOT_PATH
    "${PROJECT_ROOT}/../.Dependencies/Client"
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
