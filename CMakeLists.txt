message(STATUS "STARTING TOP LEVEL CMAKELISTS.TXT")

project (portquery)
cmake_minimum_required (VERSION 3.10.2)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# test this please
if(NOT DEFINED CATCH2_INCLUDE)
    set(CATCH2_INCLUDE "${CMAKE_SOURCE_DIR}/thirdparty/Catch2/single_include")
endif()

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include_directories("${CMAKE_SOURCE_DIR}/libportquery/include" "${CATCH2_INCLUDE}")
get_property(directories DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(directory ${directories})
    message(STATUS "INCLUDE DIRECTORY: ${directory}")
endforeach()

add_subdirectory(libportquery)
add_subdirectory(cliportquery)
add_subdirectory(tests)

