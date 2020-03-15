#pragma once

#include <string>


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
    DESCRIBE, /* keep this? */
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
    STRAIGHT_JOIN, /* keep this too? */
    THEN, /* keep conditionals */
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


class Token {
    public:
        Token(const TokenType type, const std::string lexeme, const size_t offset, const size_t location) :
            m_type(type), m_lexeme(lexeme), m_location(location) { }

    private:
        TokenType m_type;
        std::string m_lexeme;
        size_t m_location;
};

