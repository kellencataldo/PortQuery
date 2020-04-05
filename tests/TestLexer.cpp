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
    Lexer lexer_T1{"SELE#CT   ^1234A =! !==    FROM"};

    // Almost corret keyword: SELECT
    Token token_T1{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T1));

    ErrorToken error_T1 = std::get<ErrorToken>(token_T1);
    EXPECT_STREQ("SELE#CT", error_T1.m_errorLexeme.c_str());

    // Malformed integer value: ^1234A
    Token token_T2{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<ErrorToken>(token_T2));

    ErrorToken error_T2 = std::get<ErrorToken>(token_T2);
    EXPECT_STREQ("^1234A", error_T2.m_errorLexeme.c_str());

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
    EXPECT_TRUE(KeywordToken::FROM == keyword_T6.m_keyword);

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
    Token token_T13{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T13));
}

TEST(RecognizeTokens, KeywordTokens) {

    Lexer lexer_T1{"ALL AND ANY BETWEEN COUNT FROM IF IN IS LIKE LIMIT NOT OR ORDER SELECT WHERE PORT TCP UDP"};

    Token token_T1{lexer_T1.nextToken()}; // ALL
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T1));
    KeywordToken keyword_T1 = std::get<KeywordToken>(token_T1);
    EXPECT_TRUE(KeywordToken::ALL == keyword_T1.m_keyword);

    Token token_T2{lexer_T1.nextToken()}; // AND
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T2));
    KeywordToken keyword_T2 = std::get<KeywordToken>(token_T2);
    EXPECT_TRUE(KeywordToken::AND == keyword_T2.m_keyword);

    Token token_T3{lexer_T1.nextToken()}; // ANY
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T3));
    KeywordToken keyword_T3 = std::get<KeywordToken>(token_T3);
    EXPECT_TRUE(KeywordToken::ANY == keyword_T3.m_keyword);

    Token token_T4{lexer_T1.nextToken()}; // BETWEEN
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T4));
    KeywordToken keyword_T4 = std::get<KeywordToken>(token_T4);
    EXPECT_TRUE(KeywordToken::BETWEEN == keyword_T4.m_keyword);

    Token token_T5{lexer_T1.nextToken()}; // COUNT
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T5));
    KeywordToken keyword_T5 = std::get<KeywordToken>(token_T5);
    EXPECT_TRUE(KeywordToken::COUNT == keyword_T5.m_keyword);

    Token token_T6{lexer_T1.nextToken()}; // FROM
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T6));
    KeywordToken keyword_T6 = std::get<KeywordToken>(token_T6);
    EXPECT_TRUE(KeywordToken::FROM == keyword_T6.m_keyword);

    Token token_T7{lexer_T1.nextToken()}; // IF
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T7));
    KeywordToken keyword_T7 = std::get<KeywordToken>(token_T7);
    EXPECT_TRUE(KeywordToken::IF == keyword_T7.m_keyword);

    Token token_T8{lexer_T1.nextToken()}; // IN
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T8));
    KeywordToken keyword_T8 = std::get<KeywordToken>(token_T8);
    EXPECT_TRUE(KeywordToken::IN == keyword_T8.m_keyword);

    Token token_T9{lexer_T1.nextToken()}; // IS
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T9));
    KeywordToken keyword_T9 = std::get<KeywordToken>(token_T9);
    EXPECT_TRUE(KeywordToken::IS == keyword_T9.m_keyword);

    Token token_T10{lexer_T1.nextToken()}; // LIKE
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T10));
    KeywordToken keyword_T10 = std::get<KeywordToken>(token_T10);
    EXPECT_TRUE(KeywordToken::LIKE == keyword_T10.m_keyword);

    Token token_T11{lexer_T1.nextToken()}; // LIMIT
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T11));
    KeywordToken keyword_T11 = std::get<KeywordToken>(token_T11);
    EXPECT_TRUE(KeywordToken::LIMIT == keyword_T11.m_keyword);

    Token token_T12{lexer_T1.nextToken()}; // NOT
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T12));
    KeywordToken keyword_T12 = std::get<KeywordToken>(token_T12);
    EXPECT_TRUE(KeywordToken::NOT == keyword_T12.m_keyword);

    Token token_T13{lexer_T1.nextToken()}; // OR
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T13));
    KeywordToken keyword_T13 = std::get<KeywordToken>(token_T13);
    EXPECT_TRUE(KeywordToken::OR == keyword_T13.m_keyword);

    Token token_T14{lexer_T1.nextToken()}; // ORDER 
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T14));
    KeywordToken keyword_T14 = std::get<KeywordToken>(token_T14);
    EXPECT_TRUE(KeywordToken::ORDER == keyword_T14.m_keyword);

    Token token_T15{lexer_T1.nextToken()}; // SELECT
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T15));
    KeywordToken keyword_T15 = std::get<KeywordToken>(token_T15);
    EXPECT_TRUE(KeywordToken::SELECT == keyword_T15.m_keyword);

    Token token_T16{lexer_T1.nextToken()}; // WHERE
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T16));
    KeywordToken keyword_T16 = std::get<KeywordToken>(token_T16);
    EXPECT_TRUE(KeywordToken::WHERE == keyword_T16.m_keyword);

    Token token_T17{lexer_T1.nextToken()}; // PORT
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T17));
    KeywordToken keyword_T17 = std::get<KeywordToken>(token_T17);
    EXPECT_TRUE(KeywordToken::PORT == keyword_T17.m_keyword);

    Token token_T18{lexer_T1.nextToken()}; // TCP
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T18));
    KeywordToken keyword_T18 = std::get<KeywordToken>(token_T18);
    EXPECT_TRUE(KeywordToken::TCP == keyword_T18.m_keyword);

    Token token_T19{lexer_T1.nextToken()}; // UDP
    ASSERT_TRUE(std::holds_alternative<KeywordToken>(token_T19));
    KeywordToken keyword_T19 = std::get<KeywordToken>(token_T19);
    EXPECT_TRUE(KeywordToken::UDP == keyword_T19.m_keyword);

    Token token_T20{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T20));

    // Bad inputs
    Lexer lexer_T2{"ALLANY ALL.ANY ALLL .ALL ALL. ALL1"};
    
    Token token_T21{lexer_T2.nextToken()}; // ALLANY
    ASSERT_FALSE(std::holds_alternative<KeywordToken>(token_T21));

    Token token_T22{lexer_T2.nextToken()}; // ALL.ANY
    ASSERT_FALSE(std::holds_alternative<KeywordToken>(token_T22));

    Token token_T23{lexer_T2.nextToken()}; // ALLL
    ASSERT_FALSE(std::holds_alternative<KeywordToken>(token_T23));

    Token token_T24{lexer_T2.nextToken()}; // .ALL
    ASSERT_FALSE(std::holds_alternative<KeywordToken>(token_T24));

    Token token_T25{lexer_T2.nextToken()}; // ALL.
    ASSERT_FALSE(std::holds_alternative<KeywordToken>(token_T25));

    Token token_T26{lexer_T2.nextToken()}; // ALL1
    ASSERT_FALSE(std::holds_alternative<KeywordToken>(token_T24));

    Token token_T27{lexer_T2.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T27));
};
