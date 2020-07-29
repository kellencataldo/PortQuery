message("STARTING SOURCE CMAKELISTS.TXT")

add_library(libportquery STATIC 
    source/Lexer.cpp
    source/Network.cpp
    source/Parser.cpp
    source/Statement.cpp
    source/ThreadPool.cpp
    source/PortQuery.cpp
 
    )


set_target_properties(libportquery PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
    PREFIX ""
)
