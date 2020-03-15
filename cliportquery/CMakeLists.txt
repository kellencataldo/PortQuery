message("STARTING CLIPORTQUERY CMAKELISTST.TXT")


add_executable(portquery main.cpp)
target_link_libraries(portquery ${CMAKE_BINARY_DIR}/libportquery/libPortQuery.a)
