message("STARTING CLIPORTQUERY CMAKELISTST.TXT")


add_executable(cliportquery main.cpp)
target_link_libraries(cliportquery ${CMAKE_BINARY_DIR}/libportquery/libPortQuery.a)
