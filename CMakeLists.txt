cmake_minimum_required (VERSION 3.10.2)
project (portquery)
message(STATUS "STARTING TOP LEVEL CMAKELISTS.TXT")

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include_directories(libportquery/include)
get_property(directories DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(directory ${directories})
    message(STATUS "INCLUDE DIRECTORY: ${directory}")
endforeach()

add_subdirectory(libportquery)
add_subdirectory(cliportquery)

option(BUILD_TEST_BINARIES "Setting this will download and build the googletest project so that tests can be run" ON)
if (BUILD_TEST_BINARIES)
    add_subdirectory(tests)
endif()
