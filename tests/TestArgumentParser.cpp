#include "catch.hpp"
#include "ArgumentParser.h"
#include <iostream>


TEST_CASE("Help message found and counts as a fail", "[ArgumentParser]") {
    const int argc_T1 = 1;
    const char* args_T1[argc_T1] = {"./cli"};
    REQUIRE(argumentParser(argc_T1, args_T1).parse() == false);

    const int argc_T2 = 2;
    const char * args_T2[argc_T2] = {"./cli", "--usage"};
    REQUIRE(argumentParser(argc_T2, args_T2).parse() == false);

    const int argc_T3 = 4;
    const char * args_T3[argc_T3] = {"./cli", "--valid", "--usage", "potential_query_string"};
    argumentParser parser(argc_T3, args_T3);
    parser.addCommandFlag("--valid", "dummy command");
    REQUIRE(parser.parse() == false);
}


TEST_CASE("Positional arguments can be correctly parsed", "[ArgumentParser] [Positional]") {
    const int argc_T1 = 6;
    const char * args_T1[argc_T1] = {"./cli", "--debug", "5", "--term", "search", "query_string_here"};

    argumentParser parser(argc_T1, args_T1);
    parser.addCommand<int>("--debug", "help text", 1);
    parser.addCommand<std::string>("--term", "help text", std::string("default"));
    parser.addCommand<std::string>("--file", "help text", std::string("/some/file"));

    REQUIRE(parser.parse() == true);
    REQUIRE(5 == parser.getCommand<int>("--debug"));
    REQUIRE("search" == parser.getCommand<std::string>("--term"));
    REQUIRE("/some/file" == parser.getCommand<std::string>("--file"));
}



