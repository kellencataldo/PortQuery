message("STARTING CLI CMAKELISTST.TXT")


add_executable(cli main.cpp)
target_link_libraries(cli ${CMAKE_BINARY_DIR}/libportquery/libPortQuery.a)
