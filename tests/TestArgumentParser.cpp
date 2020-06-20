#include "gtest/gtest.h"
#include "../cliportquery/ArgumentParser.h"


// Help message found and counts as a fail
TEST(ParseHelpCommand, ReturnsFalse) {
    
    // No arguments supplied counts as a fail
    const int argc_T1 = 1;
    const char* args_T1[argc_T1] = {"./cli"};
    EXPECT_FALSE(ArgumentParser<emptyOutput>(argc_T1, args_T1).parse());

    // Usage flag alone counts as a fail
    const int argc_T2 = 2;
    const char * args_T2[argc_T2] = {"./cli", "--usage"};
    EXPECT_FALSE(ArgumentParser<emptyOutput>(argc_T2, args_T2).parse());

    // Usage flag specified anywhere in command counts as a fail
    const int argc_T3 = 4;
    const char * args_T3[argc_T3] = {"./cli", "--valid", "--usage", "potential_query_string"};
    ArgumentParser<emptyOutput> parser(argc_T3, args_T3);
    parser.addCommandFlag("--valid", "dummy command");
    EXPECT_FALSE(parser.parse());
}


// Positional arguments can be correctly parsed
TEST(ParseArguments, PositionalArguments) {
    const int argc_T1 = 6;
    const char * args_T1[argc_T1] = {"./cli", "--debug", "5", "--term", "search", "query_string_here"};
    ArgumentParser<emptyOutput> parser(argc_T1, args_T1);
    parser.addCommand<int>("--debug", "help text", 1);
    parser.addCommand<std::string>("--term", "help text", std::string("default"));
    parser.addCommand<std::string>("--file", "help text", std::string("/some/file"));

    ASSERT_TRUE(parser.parse());
    ASSERT_EQ(5, parser.getCommand<int>("--debug"));
    EXPECT_STREQ("search", parser.getCommand<std::string>("--term").c_str());
    EXPECT_STREQ("/some/file", parser.getCommand<std::string>("--file").c_str());

    EXPECT_THROW(parser.getCommand<std::string>("--nocommand"), std::invalid_argument);
    EXPECT_STREQ("query_string_here", parser.getQueryString().c_str());

    // Command argument which cannot be converted counts as a fail
    const int argc_T2 = 4;
    const char * args_T2[argc_T2] = {"./cli", "--intcommand", "alphabeta", "query_string_here"};
    ArgumentParser<emptyOutput> parser2(argc_T2, args_T2);

    parser2.addCommand<int>("--intcommand", "help text", 1);
    EXPECT_FALSE(parser2.parse());

    // Command with no argument supplied counts as a fail
    const int argc_T3 = 3;
    const char * args_T3[argc_T3] = {"./cli", "--command", "query_string_here"};
    ArgumentParser<emptyOutput> parser3(argc_T3, args_T3);
    EXPECT_FALSE(parser3.parse());
}


// flag arguments can be correctly parsed
TEST(ParseArguments, FlagArguments) {
    const int argc_T1 = 4;
    const char * args_T1[argc_T1] = {"./cli", "--first", "--second", "potential_query_string"};

    ArgumentParser<emptyOutput> parser(argc_T1, args_T1);
    parser.addCommandFlag("--first", "help text");
    parser.addCommandFlag("--second", "help text");
    parser.addCommandFlag("--third", "help text");
    ASSERT_TRUE(parser.parse());
    EXPECT_TRUE(parser.getCommandFlag("--first"));
    EXPECT_TRUE(parser.getCommandFlag("--second"));
    EXPECT_FALSE(parser.getCommandFlag("--third"));

    EXPECT_THROW(parser.getCommandFlag("--noflag"), std::invalid_argument);
    EXPECT_STREQ("potential_query_string", parser.getQueryString().c_str());
}


// List arguments can be correctly parsed
TEST(ParseArguments, ListArguments) {

    const int argc_T1 = 6;
    const char * args_T1[argc_T1] = {"./cli", "--list", "1", "2", "3", "query_string_here"};
    ArgumentParser<emptyOutput> parser(argc_T1, args_T1);

    parser.addCommandList<int>("--list", "help text");
    ASSERT_TRUE(parser.parse());
    std::vector<int> intList = parser.getCommandList<int>("--list");
    ASSERT_EQ(3, intList.size());
    EXPECT_TRUE((1 == intList.at(0) && 2 == intList.at(1) && 3 == intList.at(2))); 
    EXPECT_STREQ("query_string_here", parser.getQueryString().c_str());

    // List population stops after another flag is found
    const int argc_T2 = 6;
    const char * args_T2[argc_T2] = {"./cli", "--list", "arg1", "arg2", "--unrelated", "query_string_here"};
    ArgumentParser<emptyOutput> parser2(argc_T2, args_T2);

    parser2.addCommandList<std::string>("--list", "help text");
    parser2.addCommandFlag("--unrelated", "help text");
    ASSERT_TRUE(parser2.parse());
    
    std::vector<std::string> stringList = parser2.getCommandList<std::string>("--list");
    ASSERT_EQ(2, stringList.size());
    EXPECT_TRUE(("arg1" == stringList.at(0) && "arg2" == stringList.at(1)));
    EXPECT_TRUE(parser2.getCommandFlag("--unrelated"));
    EXPECT_STREQ("query_string_here", parser2.getQueryString().c_str());
}




