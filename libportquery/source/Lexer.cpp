#include <exception>
#include <cctype>
#include <sstream>

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
    return ErrorToken{std::string(m_tokenStart, m_currentChar)};
}


Token Lexer::scanURLToken() {

    return scanErrorToken();
}


Token Lexer::scanAlphaToken() {

    // Scan everything until we reach something that is not an alphabetical character or we have
    // reached the end of the token
    while (m_queryString.end() != ++m_currentChar && std::isalpha(*m_currentChar));

    // If we stopped at the end of the token, check if this is a keyword
    if (reachedTokenEnd()) {

        static const std::map<std::string, KeywordToken::Keyword> keywordMap{ 
            {"ALL",     KeywordToken::ALL},
            {"AND",     KeywordToken::AND},
            {"ANY",     KeywordToken::ANY},
            {"BETWEEN", KeywordToken::BETWEEN},
            {"COUNT",   KeywordToken::COUNT},
            {"FROM",    KeywordToken::FROM},
            {"IF",      KeywordToken::IF},
            {"IN",      KeywordToken::IN},
            {"IS",      KeywordToken::IS},
            {"LIKE",    KeywordToken::LIKE},
            {"LIMIT",   KeywordToken::LIMIT},
            {"NOT",     KeywordToken::NOT},
            {"OR",      KeywordToken::OR},
            {"ORDER",   KeywordToken::ORDER},
            {"SELECT",  KeywordToken::SELECT},
            {"WHERE",   KeywordToken::WHERE},
            {"PORT",    KeywordToken::PORT},
            {"TCP",     KeywordToken::TCP},
            {"UDP",     KeywordToken::UDP}
        };
       
        // Check to see if we have a mapping to a keyword token
        std::string key(m_tokenStart, m_currentChar);
        auto keywordMapIter = keywordMap.find(key);
        if (keywordMap.end() != keywordMapIter) {
            // if not, fall through and scan whatever we found as an error token
            return KeywordToken{keywordMapIter->second};
        }

        // if it is not in the map, then its a malformed comparison token
        return ErrorToken{key};
    } 
    // Check to see if we can transition to a URL token
    // is there a better heuristic? almost certainly.
    else if (*m_currentChar == isCharAnyOf{':', '/', '.', '@', '-', '?', '=', '&', '+', ';', '$', '#'}) { 

        // this miiiiiight be a URL token
        return scanURLToken();
    }

    return scanErrorToken();
}


Token Lexer::scanNumericToken() {

    // Increment the current character and start scanning until we encournter either the end of the token
    // or some character that is not a digit
    while (m_queryString.end() != ++m_currentChar && std::isdigit(*m_currentChar));

    // If we stopped at the end of the token, this is an integer value
    if (reachedTokenEnd()) {

        // there is no stous method in c or c++ so this little half step method is necessary because 
        // I don't want to have to deal with either exceptions or truncations
        
        const std::string potentialNumeric(m_tokenStart, m_currentChar);
        uint16_t conversionDest(0);
        std::stringstream conversionStream(potentialNumeric);
        if(conversionStream >> conversionDest && !conversionStream.fail()) {

            // We have successfully converted the token
            return NumericToken{conversionDest};
        }

        // Fall through error path, this could happen if we attempt to convert a number which gets truncted
        return ErrorToken{potentialNumeric};
    }

    // This could potentially be a URL token if a dot separater is encountered, hand things off to the 
    // scanURLToken method. Note: This is a safe dereference bceause we already know m_currentChar is not at
    // m_queryString.end() because of hte call to reachedTokenEnd()
    else if ('.' == *m_currentChar) {

        return scanURLToken();
    }

    return scanErrorToken();
    
}


Token Lexer::scanComparisonToken() {

    // Scan forwards one character and check to see if we have reached the end of our token. Also
    // check to see if we possibly have a two character comparison token, if not, it won't be present in our map
    m_currentChar++;
    if(reachedTokenEnd() || (*(m_currentChar++) == isCharAnyOf{'=', '>'} && reachedTokenEnd())) {

        static std::map<std::string, std::function<uint16_t(const uint16_t, const uint16_t)>> operatorMap{
            {"=" , [](const uint16_t a, const uint16_t b) { return a == b; } },
            {">" , [](const uint16_t a, const uint16_t b) { return a >  b; } },
            {"<" , [](const uint16_t a, const uint16_t b) { return a <  b; } },
            {">=", [](const uint16_t a, const uint16_t b) { return a >= b; } },
            {"<=", [](const uint16_t a, const uint16_t b) { return a <= b; } },
            {"<>", [](const uint16_t a, const uint16_t b) { return a != b; } }
        };


        // We have found something that looks like a key
        std::string key(m_tokenStart, m_currentChar);

        auto operatorMapIter = operatorMap.find(key);
        // if it is present in the map, we have found the operator token
        if (operatorMap.end() != operatorMapIter) {
            // if not, fall through and scan whatever we found as an error token
            return ComparisonToken{operatorMapIter->second};
        }

        // if it is not in the map, then its a malformed comparison token
        return ErrorToken{key};
    }

    // Keep scanning whatever we found as an error token
    return scanErrorToken();
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
    
    // Check to see if we have any punctuation tokens first.
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

    // next, is this a digit
    else if (std::isdigit(*m_currentChar)) {
        // this could return a number, an error token, or a URL possibly
        return scanNumericToken();
    }

    // next is this a letter
    else if (std::isalpha(*m_currentChar)) {
        // this could return a keyword, a column, a URL, or an error token
        return scanAlphaToken();
    }
    //
    
    // nothing else matches, return error case.
    return scanErrorToken();
}
