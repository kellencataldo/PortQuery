project (PortQuery)
message(STATUS "STARTING TOP LEVEL CMAKELISTS.TXT")

cmake_minimum_required (VERSION 3.10.2)
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_SOURCE_DIR}/bin")

# test this please
if(NOT DEFINED CATCH2_INCLUDE)
    set(CATCH2_INCLUDE "${CMAKE_SOURCE_DIR}/thirdparty/Catch2/single_include")
endif()

include_directories("${CMAKE_SOURCE_DIR}/include" "${CATCH2_INCLUDE}")
get_property(directories DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(directory ${directories})
    message(STATUS "INCLUDE DIRECTORY: ${directory}")
endforeach()

add_subdirectory(cli)
add_subdirectory(tests)

