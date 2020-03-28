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
    while(!reachedTokenEnd(++m_currentChar));
    return ErrorToken{std::string(m_tokenStart, m_currentChar)};
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

    // Scan forward one character
    m_currentChar++;

    // if the next char is a lexically valid, token terminating character, we know that the character we just 
    // examined must be in the operatorMap, as we matched against it in the call to scanNextToken()
    if (reachedTokenEnd(m_currentChar)) { 

        // Backup by one character, and lookup that character in the map
        // This little half step with string declaration is necessary because ALE kept losing its mind over all the 
        // brace initialization or something.
        std::string key{*(m_currentChar-1)}; 
        return ComparisonToken{operatorMap[key]};

    // Check if the next character is part of a valid comparison operator and past that is a valid
    // token terminating character.
    } else if (*m_currentChar == isCharAnyOf{'=', '>'} && reachedTokenEnd(++m_currentChar)) {

        // Another note: don't check presense here, if this throws an access error, then the scanning logic is wrong,
        // we should know exactly what we will find in the map when we go to lookup.
        return ComparisonToken{operatorMap[std::string(m_tokenStart, m_currentChar)]};
    }

    return scanErrorToken();
}


Token Lexer::scanNextToken() {

    // If m_currentChar is set to the EOF has already been returned, 
    // throw an error in the event that scan is called again
    // @TODO: possibly change this logic? maybe just keep returning EOF's?
    if (m_queryString.end() == m_currentChar) {
        // out of range error is used here, but logic error would serve just as well
        throw std::out_of_range("Cannot scan past EOF token");
    }

    // advance the character (it was stopped at the end of the last token)
    // and scan past any initial whitespace, using std::isspace instead of isspace because the standard library 
    // handles locales
    
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
