#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Lexer.h"


TEST(RecognizeTokens, EOFTokens) {

    // empty query string, first scan should return EOF
    Lexer lexer_T1{std::string()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(lexer_T1.nextToken()));

    // query string holding one valid token, past that EOF is returned
    std::string queryString_T2("SELECT");
    Lexer lexer_T2(queryString_T2);
    // First call to nextToken should not be EOF token
    ASSERT_FALSE(std::holds_alternative<EOFToken>(lexer_T2.nextToken()));
    // Second call to nextToken should be EOF token
    ASSERT_TRUE(std::holds_alternative<EOFToken>(lexer_T2.nextToken()));
    // Parsing past the first recieved EOF should throw an error
    EXPECT_THROW(lexer_T2.nextToken(), std::out_of_range);
}
