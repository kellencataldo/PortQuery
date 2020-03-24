message("STARTING CLIPORTQUERY CMAKELISTST.TXT")


add_executable(portquery main.cpp)

set_target_properties(portquery PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

target_link_libraries(portquery ${CMAKE_BINARY_DIR}/libportquery/libportquery.a)
