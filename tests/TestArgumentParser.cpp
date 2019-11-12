#include "catch.hpp"
#include "../cli/ArgumentParser.h"
#include <iostream>


TEST_CASE("Help message found and counts as a fail", "[ArgumentParser]") {
    
    // No arguments supplied counts as a fail
    const int argc_T1 = 1;
    const char* args_T1[argc_T1] = {"./cli"};
    REQUIRE(argumentParser(argc_T1, args_T1).parse() == false);

    // Usage flag alone counts as a fail
    const int argc_T2 = 2;
    const char * args_T2[argc_T2] = {"./cli", "--usage"};
    REQUIRE(argumentParser(argc_T2, args_T2).parse() == false);

    // Usage flag specified anywhere in command counts as a fail
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

    REQUIRE_THROWS(parser.getCommand<std::string>("--nocommand"));
    REQUIRE("query_string_here" == parser.getQueryString());

    // Command argument which cannot be converted counts as a fail
    const int argc_T2 = 4;
    const char * args_T2[argc_T2] = {"./cli", "--intcommand", "alphabeta", "query_string_here"};
    argumentParser parser2(argc_T2, args_T2);

    parser2.addCommand<int>("--intcommand", "help text", 1);
    REQUIRE(false == parser2.parse());

    // Command with no argument supplied counts as a fail
    const int argc_T3 = 3;
    const char * args_T3[argc_T3] = {"./cli", "--command", "query_string_here"};
    argumentParser parser3(argc_T3, args_T3);
    REQUIRE(false == parser3.parse());
}


TEST_CASE("Flag arguments can be correctly parsed", "[ArgumentParser] [Flag]") {
    const int argc_T1 = 4;
    const char * args_T1[argc_T1] = {"./cli", "--first", "--second", "potential_query_string"};

    argumentParser parser(argc_T1, args_T1);
    parser.addCommandFlag("--first", "help text");
    parser.addCommandFlag("--second", "help text");
    parser.addCommandFlag("--third", "help text");
    REQUIRE(true == parser.parse());
    REQUIRE(true == parser.getCommandFlag("--first"));
    REQUIRE(true == parser.getCommandFlag("--second"));
    REQUIRE(false == parser.getCommandFlag("--third"));

    REQUIRE_THROWS(parser.getCommandFlag("--noflag"));
    REQUIRE("potential_query_string" == parser.getQueryString());
}


TEST_CASE("List arguments can be correctly parsed" "[ArgumentParser] [List]") {

    const int argc_T1 = 6;
    const char * args_T1[argc_T1] = {"./cli", "--list", "1", "2", "3", "query_string_here"};
    argumentParser parser(argc_T1, args_T1);

    parser.addCommandList<int>("--list", "help text");
    REQUIRE(true == parser.parse());
    std::vector<int> intList = parser.getCommandList<int>("--list");
    REQUIRE(intList.size() == 3);
    REQUIRE((1 == intList.at(0) && 2 == intList.at(1) && 3 == intList.at(2))); 
    REQUIRE("query_string_here" == parser.getQueryString());

    // List population stops after another flag is found
    const int argc_T2 = 6;
    const char * args_T2[argc_T2] = {"./cli", "--list", "arg1", "arg2", "--unrelated", "query_string_here"};
    argumentParser parser2(argc_T2, args_T2);

    parser2.addCommandList<std::string>("--list", "help text");
    parser2.addCommandFlag("--unrelated", "help text");
    REQUIRE(true == parser2.parse());
    
    std::vector<std::string> stringList = parser2.getCommandList<std::string>("--list");
    REQUIRE(2 == stringList.size());
    REQUIRE(("arg1" == stringList.at(0) && "arg2" == stringList.at(1)));
    REQUIRE(true == parser2.getCommandFlag("--unrelated"));
    REQUIRE("query_string_here" == parser2.getQueryString());
}




