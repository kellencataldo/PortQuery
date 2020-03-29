#include <exception>
#include <cctype>

#include "Lexer.h"


Token Lexer::nextToken() {

    // This is an optimization for the scenario that the next token has been scanned and identified
    // but the input hasn't been advanced. This occurs when a token is "peeked"

    // First, check to see if the next token has already been peeked
    if (std::nullopt != m_peekToken) {
        // move the value that was in the peek token to the return token
        Token outToken(std::move(m_peekToken.value()));
        // Reset the peek token to an empty state, will be set again next time peek() is called
        m_peekToken.reset();
        // Return the token to the user
        return outToken;
    }

    // The peek token was empty, parse the next token the usual way.
    
    return scanNextToken();
}


Token Lexer::scanErrorToken() {
    // Grab everything that isn't a white space character. This is considered the error lexeme;
    // This method assumes that the caller detected an invalid character somewhere and m_currentChar
    // is NOT pointing at the end of the query string
    while(!reachedTokenEnd()) { m_currentChar++; };
    return ErrorToken{std::string(m_tokenStart, ++m_currentChar)};
}


Token Lexer::scanAlphaToken() {

}


Token Lexer::scanIntegerToken() {

}


Token Lexer::scanComparisonToken() {

    static std::map<std::string, std::function<uint16_t(const uint16_t, const uint16_t)>> operatorMap{
        {"=" , [](const uint16_t a, const uint16_t b) { return a == b; } },
        {">" , [](const uint16_t a, const uint16_t b) { return a >  b; } },
        {"<" , [](const uint16_t a, const uint16_t b) { return a <  b; } },
        {">=", [](const uint16_t a, const uint16_t b) { return a >= b; } },
        {"<=", [](const uint16_t a, const uint16_t b) { return a <= b; } },
        {"<>", [](const uint16_t a, const uint16_t b) { return a != b; } }
    };

    // This is super hacky and innefficient (lol), start by scanning to the end of the potential token
    while (!reachedTokenEnd()) { m_currentChar++; };
    
    // use whatever this token is as a key to check if it is present in the operatorMap
    std::string key(m_tokenStart, ++m_currentChar);
    if(operatorMap.end() != operatorMap.find(key)) {

        // If they key is present, construct a ComparisonToken and return that
        return ComparisonToken{operatorMap[key]};
    }

    // If the key is not present, return whatever hte heck was scanned as an error token. 
    // Two birds with one stone I guess.
    return ErrorToken{key};
}


Token Lexer::scanNextToken() {

    // The assumption here is that the next time this is called, it will be either at the start of the string,
    // or one character past the termination of the last token.
    
    while(m_queryString.end() != m_currentChar && std::isspace(*m_currentChar)) { m_currentChar++; }

    // The end of the string has been reached, return the EOF token
    if (m_queryString.end() == m_currentChar) { return EOFToken{}; }

    // and now... the real fun begins.
    // Set the token start to wherever the whitespace ended
    m_tokenStart = m_currentChar;
    
    switch(*m_currentChar) {
        case '*': return PunctuationToken<'*'>{};
        case '(': return PunctuationToken<'('>{};
        case ')': return PunctuationToken<')'>{};
        case ',': return PunctuationToken<','>{};
        case ';': return PunctuationToken<';'>{}; // return EOF here? this isn't being handled correctly.
    }
   

    // next, lets try scanning some comparison operators
    if (*m_currentChar == isCharAnyOf{'=', '>', '<'}) {
        // The next character is potentially the beginning of a comparison operator
        return scanComparisonToken();
    }


    
    // nothing else matches, return error case.
    return scanErrorToken();
}
