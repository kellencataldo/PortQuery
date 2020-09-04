#pragma once

#include <string>
#include <map>
#include <variant>
#include <optional>
#include <functional>
#include <algorithm>

#include "PortQuery.h"


namespace PortQuery {
    /* Other tokens that could be supported in the future

        AS, ASC, ANY, CASE, COUNT, CROSS, DESC, DESCRIBE, DISTINCT, EXISTS, GROUP_BY, HAVING, IF, IN, INNER, 
        JOIN, LEFT, LIMIT, MATCH, NATURAL, ON, ORDER, OUTER, RIGHT, TO, UNION, UNIQUE, USING, WHEN, WITH,
    */


    // Handy little helper class which performs a comparison on a collection of objects of varying lengths. 
    // Used mostly when comparing a single char against a collection of characters to determine how to scan a 
    // certain token. For example: if (someChar == isElementPresent<char>{'<', '>', '='}) parse it as a comparison operator

    // Inherit from vector so we get not only the collection, but also the convenient constructors that come with it.
    // Note the wierd syntax in the example above, this is because you are looking a braces initialized constructor
    template <typename T> struct isElementPresent: private std::vector<T> {
        using std::vector<T>::vector;
        bool operator==(const T& c) const {
            // Upcast to so that we can perform std::any_of on our vector
            const std::vector<T>& collection = static_cast<const std::vector<T>&>(*this);
            return std::any_of(collection.cbegin(), collection.cend(), [&c](const T& other) {return c == other;});
        }

        // Add this is so we can support both comparisons from both sides
        friend bool operator==(const T& lhs, const isElementPresent rhs) { return rhs == lhs; }
    };


    // This token represents any numeric value.
    // For right now, only values which can be stored in an unsigned short
    // without truncation are supported
    // This could change I guess IDC.
    struct NumericToken { 

        uint16_t m_value; 
    };

    // This token represents a binary comparison operator such as !=
    struct ComparisonToken {

        // This enum represents all the different comparison operations supported in SOSQL
        // Occasionally, other tokens will make use of this enum as well, such as an IS
        // keyword that appears in a WHERE clause (it gets converted to EQ
        enum OpType {
            OP_EQ,  // ==, or IS
            OP_GT,  // >
            OP_LT,  // <
            OP_GTE, // >=
            OP_LTE, // <=
            OP_NE   // <>, or IS NOT
        };

        OpType m_opType;
    };

    struct KeywordToken {

        enum Keyword {
            AND,
            BETWEEN,
            FROM,
            IS,
            NOT,
            OR,
            SELECT,
            WHERE
        };

        Keyword m_keyword;

    };

    // This token represents a User string which contains valid characters
    // This could either be a table alias, a URL, a column alias, etc
    struct UserToken { 

        std::string m_UserToken; 
    };

    // These tokens represents query results that can be compared to the columns
    // Currently, there are only three result, OPEN, CLOSED, and REJECTED
    
    struct QueryResultToken {

        PQ_QUERY_RESULT m_queryResult;
    };
    
    // This token represents all the column types in the port query "database"
    // For now, there are only three: PORT, TCP, UDP
    
    struct ColumnToken {

        enum Column {

            PORT,
            TCP,
            UDP
        };

        Column m_column;
    };

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
        NumericToken,
        ComparisonToken,
        UserToken,
        ColumnToken,
        QueryResultToken,
        KeywordToken,
        EOFToken,
        // All the supported punctuation types below, maybe more to come?
        PunctuationToken<'*'>,
        PunctuationToken<'('>,
        PunctuationToken<')'>,
        PunctuationToken<','>,
        PunctuationToken<';'>,

        // token for when nothing else matches
        ErrorToken>;

    // helper class for std::visit. Constructs a callable which accepts various types based on deduction
    template<typename ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<typename ... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    template<typename ... Ts> bool MATCH(const Token t) { return (std::holds_alternative<Ts>(t) || ...); }

    inline bool MATCH_TERMINAL(const Token t) {

        return MATCH<NumericToken, QueryResultToken, ColumnToken>(t);
    }

    template<ColumnToken::Column... Cs> bool MATCH_COLUMN(const Token t) {

        return std::holds_alternative<ColumnToken>(t) && 
            std::get<ColumnToken>(t).m_column == isElementPresent<ColumnToken::Column>{Cs...,};
    }

    template<KeywordToken::Keyword... Ks> bool MATCH_KEYWORD(const Token t) {

        return std::holds_alternative<KeywordToken>(t) && 
            std::get<KeywordToken>(t).m_keyword == isElementPresent<KeywordToken::Keyword>{Ks...,};
    }

    template<PQ_QUERY_RESULT... Qs> bool MATCH_QUERY_RESULT(const Token t) {

        return std::holds_alternative<QueryResultToken>(t) && 
            std::get<QueryResultToken>(t).m_queryResult == isElementPresent<PQ_QUERY_RESULT>{Qs...,};
    }

    class Lexer { 
        public:
            Lexer(const std::string queryString) : m_queryString{std::move(queryString)} {
                std::for_each(m_queryString.begin(), m_queryString.end(), toupper);
                m_tokenStart = m_queryString.cbegin();
                m_currentChar = m_queryString.cbegin();
            }

            // The public facing Lex function, first checks if a token has already been scanned 
            // By someone calling "Peek", if so returns that and clears the peek token. If not,
            // calls the internal scanNextToken function to get get an output
            Token nextToken();

            Token peek();

        private:

            // The function which holds the scanning logic, this can be called by both nextToken
            // and Peek
            Token scanNextToken();

            // This method is transitioned to when no other routine can resolve a token. This routine
            // Scans until the end of the unresolved token, and returns the lexeme to the caller
            Token scanErrorToken();

            // This method is transitioned to when a token begins with an alphabetical character.
            // Tokens that could be returned by this routine are: KeywordTokens, ColumnTokens, URLSTokens, ErrorTokens
            // In the future this is where user variables would be parsed from. (not supported now lol)
            Token scanAlphaToken();

            // This method is transitioned to when a token begins with a numeric character.
            // Tokens that could be returned by this routine are: NumericTokens, possibly URLTokens, and ErrorTokens
            Token scanNumericToken();

            // This method is transitioned to when a token begins with a comparison character (!, =, <, >)
            // Tokens that could be returned by this routine are: ComparisonTokens, ErrorTokens
            Token scanComparisonToken();

            // This method is transitioned to when a potential user has been spotted. User provided URL's are VERY hard to match
            // against, so this can be transitioned to from both scanAlphaToken and scanNumericToken routines
            Token scanUserToken();

            // Some characters are not whitespace, but can also legitimately terminate a character
            // Essentially this includes all the punctuation tokens. This could be expanded in the future to include
            // comparison token characters
            bool reachedTokenEnd() const {

                // All of these are for checks that a token can be legitimately terminated
                return m_queryString.end() == m_currentChar || std::isspace(*m_currentChar) || 
                    *m_currentChar == isElementPresent<char>{'*', '(', ')', ',', ';'};
            }

            // This could be handled better. For right now, these are the valid URL characters. URL tokenbs could contain
            // any of the tokens above as well, but those are more rare. This should be changed in the future.
            static bool isValidUserCharacter(const char c) {
                return std::isalpha(c) || std::isdigit(c) || 
                    c == isElementPresent<char>{'$', '-', '_', '.', '+', '!', '\'', '/', '?', ':', '@', '=', '&'};

            }

            // This query string represents the SQL query to be scanned
            std::string m_queryString;

            // This peek token is occasionally necessary when the parser wants to lookahead at
            // the next token without advancing the lexer
            // An example of this is when parsing "GROUP BY" because the keyword GROUP only
            // makes sense when followed by the keyword BY. This context can be necessary sometimes
            std::optional<Token> m_peekToken;

            // Iterators are used here out of convenience, when generating a string using two iterators, the character
            // pointed at by the terminating iterator is not included, so therefore, you can increment that forward  
            // iterator, create a substring of the previous lexeme, and have the forward iterator be prepared to scan the 
            // next token all in one go.
            std::string::const_iterator m_tokenStart;
            std::string::const_iterator m_currentChar;
    };
}
