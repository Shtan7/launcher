cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(server LANGUAGES CXX)

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

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_BINARY_DIR}/../postgrespp/include)
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB postgrespp_src ${PROJECT_SOURCE_DIR}/postgrespp/src/*.cpp *.hpp)
file(GLOB server_source ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(server_app ${server_source} ${postgrespp_src})
target_link_libraries(server_app ${CONAN_LIBS})