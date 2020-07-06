message("STARTING TESTS CMAKELISTS.TXT")
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)
add_executable(tests 
    ${CMAKE_SOURCE_DIR}/libportquery/source/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/libportquery/source/Network.cpp
    ${CMAKE_SOURCE_DIR}/libportquery/source/Parser.cpp
    ${CMAKE_SOURCE_DIR}/libportquery/source/Statement.cpp
    ${CMAKE_SOURCE_DIR}/libportquery/source/PortQuery.cpp
    TestArgumentParser.cpp
    TestLexer.cpp
    TestParser.cpp
    TestPortQuery.cpp
    )

set_target_properties(tests PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
    PREFIX ""
)

target_link_libraries(tests gtest_main)
add_test(NAME unit_tests COMMAND tests)
