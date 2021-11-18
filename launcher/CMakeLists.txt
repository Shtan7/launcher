cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(client LANGUAGES CXX)

if(MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(handler_path ${PROJECT_SOURCE_DIR}/../server)
list(APPEND file_handler_sources ${handler_path}/file_handler.cpp 
${handler_path}/file_handler.hpp ${handler_path}/common.hpp ${handler_path}/file_handler.hpp ${handler_path}/common.cpp)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB launcher_sources ${PROJECT_SOURCE_DIR}/*.cpp *.hpp)

add_executable(launcher ${launcher_sources} ${file_handler_sources})
target_link_libraries(launcher ${CONAN_LIBS})