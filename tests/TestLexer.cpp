#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Lexer.h"


TEST(RecognizeTokens, EOFTokens) {

    // query string holding one valid token with some space after, past that EOF is returned
    Lexer lexer_T2{"SELECT     "};
    // First call to nextToken should not be EOF token
    ASSERT_FALSE(std::holds_alternative<EOFToken>(lexer_T2.nextToken()));
    // Second call to nextToken should be EOF token
    ASSERT_TRUE(std::holds_alternative<EOFToken>(lexer_T2.nextToken()));
    // Parsing past the first recieved EOF should throw an error
    EXPECT_THROW(lexer_T2.nextToken(), std::out_of_range);
}


TEST(RecognizeTokens, ErrorTokens) {

    // A garbage query containing all error tokens and one valid token with some whitespace in between
    Lexer lexer_T1{"SELECTT   1234A A1234  =! !==    FROM"};

    // Almost corret keyword: SELECTT
    Token token_T1{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T1));

    ErrorToken error_T1 = std::get<ErrorToken>(token_T1);
    EXPECT_STREQ("SELECTT", error_T1.m_errorLexeme.c_str());

    // Malformed integer value: 1234A
    Token token_T2{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T2));

    ErrorToken error_T2 = std::get<ErrorToken>(token_T2);
    EXPECT_STREQ("1234A", error_T2.m_errorLexeme.c_str());

    // Malformed string value: A1234
    Token token_T3{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T3));

    ErrorToken error_T3 = std::get<ErrorToken>(token_T3);
    EXPECT_STREQ("A1234", error_T3.m_errorLexeme.c_str());

    // Backwords comparison token: =!
    Token token_T4{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T4));

    ErrorToken error_T4 = std::get<ErrorToken>(token_T4);
    EXPECT_STREQ("=!", error_T4.m_errorLexeme.c_str());

    // Malformed comparison token: !==
    Token token_T5{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T5));

    ErrorToken error_T5 = std::get<ErrorToken>(token_T5);
    EXPECT_STREQ("!==", error_T5.m_errorLexeme.c_str());

    // after all these errors we still recognize a valid token
    Token token_T6{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T6));

    KeywordToken keyword_T6 = std::get<KeywordToken>(token_T6);
    EXPECT_TRUE(Keyword::SELECT == keyword_T6.m_keyword);

    // And last but not least is the EOF token
    ASSERT_TRUE(std::holds_alternative<EOFToken>(lexer_T1.nextToken()));
 
}
