message("STARTING SOURCE CMAKELISTS.TXT")

add_library(libportquery STATIC 
    source/PortQuery.cpp
    source/Scanner.cpp
    )


set_target_properties(libportquery PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
    PREFIX ""
)
