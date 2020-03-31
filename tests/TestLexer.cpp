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
//    Token token_T6{lexer_T1.nextToken()};
 //   ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T6));

 //   KeywordToken keyword_T6 = std::get<KeywordToken>(token_T6);
  //  EXPECT_TRUE(Keyword::SELECT == keyword_T6.m_keyword);

    // And last but not least is the EOF token
    lexer_T1.nextToken();
    ASSERT_TRUE(std::holds_alternative<EOFToken>(lexer_T1.nextToken()));
}


TEST(RecognizeTokens, ComparisonTokens) {

    Lexer lexer_T1{"= > < >= <= <>       == >< => =< << >>"};
    
    Token token_T1{lexer_T1.nextToken()}; // =
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T1));
    ComparisonToken comparison_T1 = std::get<ComparisonToken>(token_T1);
    EXPECT_TRUE(comparison_T1.m_compareFunc(1, 1));

    Token token_T2{lexer_T1.nextToken()}; // >
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T2));
    ComparisonToken comparison_T2 = std::get<ComparisonToken>(token_T2);
    EXPECT_TRUE(comparison_T2.m_compareFunc(2, 1));

    Token token_T3{lexer_T1.nextToken()}; // <
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T3));
    ComparisonToken comparison_T3 = std::get<ComparisonToken>(token_T3);
    EXPECT_TRUE(comparison_T3.m_compareFunc(1, 2));

    Token token_T4{lexer_T1.nextToken()}; // >=
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T4));
    ComparisonToken comparison_T4 = std::get<ComparisonToken>(token_T4);
    EXPECT_TRUE(comparison_T4.m_compareFunc(2, 1));
    EXPECT_TRUE(comparison_T4.m_compareFunc(1, 1));

    Token token_T5{lexer_T1.nextToken()}; // <=
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T5));
    ComparisonToken comparison_T5 = std::get<ComparisonToken>(token_T5);
    EXPECT_TRUE(comparison_T5.m_compareFunc(1, 2));
    EXPECT_TRUE(comparison_T5.m_compareFunc(1, 1));

    Token token_T6{lexer_T1.nextToken()}; // <>
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T6));
    ComparisonToken comparison_T6 = std::get<ComparisonToken>(token_T6);
    EXPECT_TRUE(comparison_T6.m_compareFunc(2, 1));

    Token token_T7{lexer_T1.nextToken()}; // == 
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T7));

    Token token_T8{lexer_T1.nextToken()}; // ><
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T8));

    Token token_T9{lexer_T1.nextToken()}; // =>
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T9));

    Token token_T10{lexer_T1.nextToken()}; // =< 
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T10));

    Token token_T11{lexer_T1.nextToken()}; // << 
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T11));

    Token token_T12{lexer_T1.nextToken()}; // >>
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T12));

    // And last but not least is the EOF token
    Token token_T13{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T13));
}


TEST(RecognizeTokens, NumericTokens) {

    Lexer lexer_T1{"1 12 345 06789 65535     12345A 0x123 123.456 12A34 -1 65536"};
    
    Token token_T1{lexer_T1.nextToken()}; // 1
    ASSERT_TRUE(std::holds_alternative<NumericToken>(token_T1));
    NumericToken numeric_T1 = std::get<NumericToken>(token_T1);
    EXPECT_TRUE(1 == numeric_T1.m_value);

    Token token_T2{lexer_T1.nextToken()}; // 12
    ASSERT_TRUE(std::holds_alternative<NumericToken>(token_T2));
    NumericToken numeric_T2 = std::get<NumericToken>(token_T2);
    EXPECT_TRUE(12 == numeric_T2.m_value);


    Token token_T3{lexer_T1.nextToken()}; // 345
    ASSERT_TRUE(std::holds_alternative<NumericToken>(token_T3));
    NumericToken numeric_T3 = std::get<NumericToken>(token_T3);
    EXPECT_TRUE(345 == numeric_T3.m_value);


    Token token_T4{lexer_T1.nextToken()}; // 06789 
    ASSERT_TRUE(std::holds_alternative<NumericToken>(token_T4));
    NumericToken numeric_T4 = std::get<NumericToken>(token_T4);
    EXPECT_TRUE(6789 == numeric_T4.m_value);

    Token token_T5{lexer_T1.nextToken()}; // 65535
    ASSERT_TRUE(std::holds_alternative<NumericToken>(token_T5));
    NumericToken numeric_T5 = std::get<NumericToken>(token_T5);
    EXPECT_TRUE(65535 == numeric_T5.m_value);

    Token token_T6{lexer_T1.nextToken()}; // 12345A
    EXPECT_FALSE(std::holds_alternative<NumericToken>(token_T6));

    Token token_T7{lexer_T1.nextToken()}; // 0x123
    ASSERT_FALSE(std::holds_alternative<NumericToken>(token_T7));

    Token token_T8{lexer_T1.nextToken()}; // 123.456
    ASSERT_FALSE(std::holds_alternative<NumericToken>(token_T8));

    Token token_T9{lexer_T1.nextToken()}; // 12A34
    ASSERT_FALSE(std::holds_alternative<NumericToken>(token_T9));

    Token token_T10{lexer_T1.nextToken()}; // -1
    ASSERT_FALSE(std::holds_alternative<NumericToken>(token_T10));

    Token token_T11{lexer_T1.nextToken()}; // 65536
    ASSERT_FALSE(std::holds_alternative<NumericToken>(token_T11));
    // And last but not least is the EOF token
    //
    Token token_T13{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T13));
}
