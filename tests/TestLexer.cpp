#include <string>

#include "gtest/gtest.h"
#include "../libportquery/source/Lexer.h"
// #include "../libportquery/include/PortQuery.h"


using namespace PortQuery;

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
    EXPECT_TRUE(MATCH_KEYWORD<KeywordToken::FROM>(token_T6));

    // And last but not least is the EOF token
    lexer_T1.nextToken();
    ASSERT_TRUE(std::holds_alternative<EOFToken>(lexer_T1.nextToken()));
}


TEST(RecognizeTokens, ComparisonTokens) {

    Lexer lexer_T1{"= > < >= <= <>       == >< => =< << >>"};
    
    Token token_T1{lexer_T1.nextToken()}; // =
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T1));
    ComparisonToken comparison_T1 = std::get<ComparisonToken>(token_T1);
    EXPECT_TRUE(ComparisonToken::OP_EQ == comparison_T1.m_opType);

    Token token_T2{lexer_T1.nextToken()}; // >
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T2));
    ComparisonToken comparison_T2 = std::get<ComparisonToken>(token_T2);
    EXPECT_TRUE(ComparisonToken::OP_GT == comparison_T2.m_opType);

    Token token_T3{lexer_T1.nextToken()}; // <
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T3));
    ComparisonToken comparison_T3 = std::get<ComparisonToken>(token_T3);
    EXPECT_TRUE(ComparisonToken::OP_LT == comparison_T3.m_opType);

    Token token_T4{lexer_T1.nextToken()}; // >=
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T4));
    ComparisonToken comparison_T4 = std::get<ComparisonToken>(token_T4);
    EXPECT_TRUE(ComparisonToken::OP_GTE == comparison_T4.m_opType);

    Token token_T5{lexer_T1.nextToken()}; // <=
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T5));
    ComparisonToken comparison_T5 = std::get<ComparisonToken>(token_T5);
    EXPECT_TRUE(ComparisonToken::OP_LTE == comparison_T5.m_opType);

    Token token_T6{lexer_T1.nextToken()}; // <>
    ASSERT_TRUE(std::holds_alternative<ComparisonToken>(token_T6));
    ComparisonToken comparison_T6 = std::get<ComparisonToken>(token_T6);
    EXPECT_TRUE(ComparisonToken::OP_NE == comparison_T6.m_opType);

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

    Lexer lexer_T1{"ALL AND BETWEEN FROM IS NOT OR SELECT WHERE"};

    Token token_T1{lexer_T1.nextToken()}; // ALL
    EXPECT_TRUE(MATCH_KEYWORD<KeywordToken::ALL>(token_T1));

    Token token_T2{lexer_T1.nextToken()}; // AND
    EXPECT_TRUE(MATCH_KEYWORD<KeywordToken::AND>(token_T2));

    Token token_T4{lexer_T1.nextToken()}; // BETWEEN
    EXPECT_TRUE(MATCH_KEYWORD<KeywordToken::BETWEEN>(token_T4));

    Token token_T6{lexer_T1.nextToken()}; // FROM
    EXPECT_TRUE(MATCH_KEYWORD<KeywordToken::FROM>(token_T6));

    Token token_T9{lexer_T1.nextToken()}; // IS
    ASSERT_TRUE(MATCH_KEYWORD<KeywordToken::IS>(token_T9));

    Token token_T12{lexer_T1.nextToken()}; // NOT
    ASSERT_TRUE(MATCH_KEYWORD<KeywordToken::NOT>(token_T12));

    Token token_T13{lexer_T1.nextToken()}; // OR
    ASSERT_TRUE(MATCH_KEYWORD<KeywordToken::OR>(token_T13));

    Token token_T15{lexer_T1.nextToken()}; // SELECT
    ASSERT_TRUE(MATCH_KEYWORD<KeywordToken::SELECT>(token_T15));

    Token token_T16{lexer_T1.nextToken()}; // WHERE
    ASSERT_TRUE(MATCH_KEYWORD<KeywordToken::WHERE>(token_T16));

    Token token_T20{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T20));

    // Bad inputs (parsed as user identifiers)
    Lexer lexer_T2{"ALLANY ALL.ANY ALLL .ALL ALL. ALL1"};
    
    Token token_T21{lexer_T2.nextToken()}; // ALLANY
    ASSERT_TRUE(std::holds_alternative<UserToken>(token_T21));

    Token token_T22{lexer_T2.nextToken()}; // ALL.ANY
    ASSERT_TRUE(std::holds_alternative<UserToken>(token_T22));

    Token token_T23{lexer_T2.nextToken()}; // ALLL
    EXPECT_TRUE(std::holds_alternative<UserToken>(token_T23));

    Token token_T24{lexer_T2.nextToken()}; // .ALL
    EXPECT_TRUE(std::holds_alternative<ErrorToken>(token_T24));

    Token token_T25{lexer_T2.nextToken()}; // ALL.
    EXPECT_TRUE(std::holds_alternative<UserToken>(token_T25));

    Token token_T26{lexer_T2.nextToken()}; // ALL1
    EXPECT_TRUE(std::holds_alternative<UserToken>(token_T26));

    Token token_T27{lexer_T2.nextToken()};
    EXPECT_TRUE(std::holds_alternative<EOFToken>(token_T27));
};


TEST(RecognizeTokens, ColumnTokens) {

    Lexer lexer_T1{"PORT TCP UDP"};

    Token token_T1{lexer_T1.nextToken()}; // PORT
    ASSERT_TRUE(MATCH_COLUMN<ColumnToken::PORT>(token_T1));

    Token token_T2{lexer_T1.nextToken()}; // TCP
    ASSERT_TRUE(MATCH_COLUMN<ColumnToken::TCP>(token_T2));

    Token token_T3{lexer_T1.nextToken()}; // UDP
    ASSERT_TRUE(MATCH_COLUMN<ColumnToken::UDP>(token_T3));

    Token token_T20{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T20));
};


TEST(RecognizeTokens, QueryResultTokens) {

    Lexer lexer_T1{"OPEN CLOSED REJECTED"};

    Token token_T1{lexer_T1.nextToken()}; // OPEN
    EXPECT_TRUE(MATCH_QUERY_RESULT<PQ_QUERY_RESULT::OPEN>(token_T1));

    Token token_T2{lexer_T1.nextToken()}; // CLOSED
    ASSERT_TRUE(MATCH_QUERY_RESULT<PQ_QUERY_RESULT::CLOSED>(token_T2));

    Token token_T3{lexer_T1.nextToken()}; // REJECTED
    ASSERT_TRUE(MATCH_QUERY_RESULT<PQ_QUERY_RESULT::REJECTED>(token_T3));

    Token token_T20{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T20));
}


TEST(LexerFunctionality, Peek) {

    // query string holding one valid token with some space after, past that EOF is returned
    Lexer lexer_T1{"SELECT ,   "};

    Token token_T1{lexer_T1.peek()};
    EXPECT_TRUE(MATCH_KEYWORD<KeywordToken::SELECT>(token_T1));

    Token token_T2{lexer_T1.nextToken()};
    ASSERT_TRUE(MATCH_KEYWORD<KeywordToken::SELECT>(token_T2));

    Token token_T3{lexer_T1.peek()};
    EXPECT_TRUE(std::holds_alternative<PunctuationToken<','>>(token_T3));

    Token token_T4{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<PunctuationToken<','>>(token_T4));

    Token token_T5{lexer_T1.peek()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T5));

    Token token_T6{lexer_T1.nextToken()};
    ASSERT_TRUE(std::holds_alternative<EOFToken>(token_T6));
}
