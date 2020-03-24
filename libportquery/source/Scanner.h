#pragma once

#include <string>
#include <string_view>
#include <map>

/*
enum class TokenType {
    // KEY WORDS
    
    ALL,
    AND_BETWEEN,
    AND_OP,
    ANY,
    AS,
    ASC,
    BETWEEN,
    BY,
    CASE,
    COUNT,
    CROSS,
    DESC,
    DESCRIBE,  keep this? 
    DISTINCT,
    DISTINCTROW,
    ELSE,
    ELSEIF,
    EXISTS,
    FROM,
    GROUP,
    HAVING,
    IF,
    IN,
    INNER,
    IS,
    JOIN,
    LEFT,
    LIKE,
    LIMIT,
    MATCH,
    NATURAL,
    NOT,
    ON,
    OR,
    ORDER,
    OUTER,
    RIGHT,
    SELECT,
    STRAIGHT_JOIN,  keep this too? 
    THEN,  keep conditionals 
    TO,
    UNION,
    UNIQUE,
    USING,
    WHEN,
    WHERE,
    WITH,

    // SYMBOLS

    SEMICOLON,
    ASTERISK,
    COMMA,
    LEFT_PAREN,
    RIGHT_PAREN,
    GT,
    GT_OR_EQ,
    LT,
    LT_OR_EQ,
    EQ,
    NOT_EQ,
};
*/

enum class TokenType {
    KEYWORD,
    OPERATOR,
    NUMBER,
    COLUMN,
    URL
};


class Token {
    public:
        Token(const TokenType type, std::string_view tokenView) : m_type(type), m_tokenView(tokenView) { }

        const TokenType getTokenType(void) const {
            return m_type;
        }


    private:
        TokenType m_type;
        std::string_view m_tokenView;
};


class KeywordToken {
    public:
        enum Keyword {
            ALL,
            AND_BETWEEN,
            ANY,
            BETWEEN,
            COUNT,
            FROM,
            // GROUP_BY,
            // HAVING,
            IF,
            IN,
            IS,
            LIKE,
            LIMIT,
            NOT,
            OR,
            ORDER,
            SELECT,
            WHERE,
        };

    private:
        static std::map<std::string, Keyword> m_keywordMap;
};

std::map<std::string, KeywordToken::Keyword> KeywordToken::m_keywordMap = {



};

class Scanner {
    public:
        Scanner(const std::string queryString) : m_queryString(queryString), m_errorSet(false) {
            m_start = m_queryString.cbegin();
            m_nextChar = m_queryString.cbegin();

        }


    private:
        std::string m_queryString;
        std::string::const_iterator m_start;
        std::string::const_iterator m_nextChar;
        bool m_errorSet;
};

