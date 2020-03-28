#pragma once

#include <string>
#include <string_view>
#include <map>
#include <variant>
#include <optional>
#include <functional>

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


// Enum of all supported keywords
enum class Keyword {
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

// This token represents all keywords from the above enum.
// No other information except which keyword is needed
struct KeywordToken { Keyword m_keyword; };

enum class Column {
    PORT,
    TCP,
    UDP
};

// This token represents the names of all the columes that can be queried.
// Maybe this could change or be expanded in the future to be more dynamic, but for right now
// this is as future proof as it gets.
struct ColumnToken { Column m_column; };

// This token represents any numeric value.
// For right now, only values which can be stored in an unsigned short
// without truncation are supported
// This could change I guess IDC.
struct NumericToken { uint16_t m_value; };

// This token represents a binary comparison operator such as !=
struct ComparisonToken { std::function<uint16_t(const uint16_t, const uint16_t)> m_compareFunc; };

// This token represents a URL upon which to make queries
struct URLToken { std::string_view m_URL; };

// This token should be pretty self explanatory.
struct EOFToken { };

// This token represents single character punctuation.
// This type of punctuation includes semicolon, asterisk, comma, etc
// This is distinctly separate from binary comparison tokens
template <char> struct PunctuationToken { };

// A special error token. When the Lexer encounters a lexeme that it cannot resolve, it stores the string 
// in this special error token struct. It is up to the parser to decide what to do with it

struct ErrorToken { std::string m_errorLexeme; };

using Token = std::variant<
    KeywordToken,
    ColumnToken,
    NumericToken,
    ComparisonToken,
    URLToken,
    EOFToken,

    // All the supported punctuation types below, maybe more to come?
    PunctuationToken<'*'>,
    PunctuationToken<'('>,
    PunctuationToken<')'>,
    PunctuationToken<','>,
    PunctuationToken<';'>,

    ErrorToken
    >;


class Lexer { 
    public:
        Lexer(const std::string queryString) : m_queryString{std::move(queryString)} {
            m_tokenStart = m_queryString.cbegin();
            m_currentChar = m_queryString.cbegin();
        
        }

        // The public facing Lex function, first checks if a token has already been scanned 
        // By someone calling "Peek", if so returns that and clears the peek token. If not,
        // calls the internal scanNextToken function to get get an output

        Token nextToken();

        // Token peek();

    private:

        // The function which holds the scanning logic, this can be called by both nextToken
        // and Peek
        Token scanNextToken();

        // This method is transitioned to when no other routine can resolve a token. This routine
        // Scans until the end of the unresolved token, and returns the lexeme to the caller
        Token scanErrorToken();
        
        // This query string represents the SQL query to be scanned
        std::string m_queryString;

        // This peek token is occasionally necessary when the parser wants to lookahead at
        // the next token without advancing the lexer
        // An example of this is when parsing "GROUP BY" because the keyword GROUP only
        // makes sense when followed by the keyword BY. This context can be necessary sometimes
        std::optional<Token> m_peekToken;

        std::string::const_iterator m_tokenStart;
        std::string::const_iterator m_currentChar;
};
