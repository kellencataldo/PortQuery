#include <exception>
#include <cctype>
#include <sstream>
#include <algorithm>

#include "Lexer.h"


namespace PortQuery {

    Token getKeywordTokenFromString(const std::string lexeme) {

        static const std::map<std::string, Token> keywordMap { 
            {"ALL",      KeywordToken{ KeywordToken::ALL }},
            {"AND",      KeywordToken{ KeywordToken::AND }},
            {"BETWEEN",  KeywordToken{ KeywordToken::BETWEEN }},
            {"FROM",     KeywordToken{ KeywordToken::FROM }},
            {"IS",       KeywordToken{ KeywordToken::IS }},
            {"NOT",      KeywordToken{ KeywordToken::NOT }},
            {"OR",       KeywordToken{ KeywordToken::OR }},
            {"SELECT",   KeywordToken{ KeywordToken::SELECT }},
            {"WHERE",    KeywordToken{ KeywordToken::WHERE }},

            // Not technically tokens but the logic is the same
            {"OPEN",     QueryResultToken{ QueryResultToken::OPEN }},
            {"CLOSED",   QueryResultToken{ QueryResultToken::CLOSED }},
            {"REJECTED", QueryResultToken{ QueryResultToken::REJECTED }},
            {"PORT",     ColumnToken{ ColumnToken::PORT }},
            {"TCP",      ColumnToken{ ColumnToken::TCP }},
            {"UDP",      ColumnToken{ ColumnToken::UDP }},
        };

        auto keywordMapIter = keywordMap.find(lexeme);
        if (keywordMap.end() != keywordMapIter) {
            return keywordMapIter->second;
        }

        // EOFToken here signafies "not a keyword"
        return EOFToken{};
    }




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


    Token Lexer::peek() {

        // if the peek token is empty, set it to the next token
        if (std::nullopt == m_peekToken) { 

            m_peekToken = scanNextToken();
        }

        return *m_peekToken;
    }


    Token Lexer::scanErrorToken() {
        // Grab everything that isn't a white space character. This is considered the error lexeme;
        // This method assumes that the caller detected an invalid character somewhere and m_currentChar
        // is NOT pointing at the end of the query string
        while(!reachedTokenEnd()) { m_currentChar++; };
        return ErrorToken{std::string(m_tokenStart, m_currentChar)};
    }


    Token Lexer::scanUserToken() {

        // Very similar to scan error Token, except we first check to see if it can be converted to a binary address
        // first grab everything
        while(m_queryString.end() != ++m_currentChar && isValidUserCharacter(*m_currentChar));

        if (reachedTokenEnd()) {

            return UserToken{std::string(m_tokenStart, m_currentChar)};
        }

        // failure path, this is an error token
        return scanErrorToken();
    }


    Token Lexer::scanAlphaToken() {

        // Scan everything until we reach something that is not an alphabetical character or we have
        // reached the end of the token
        while (m_queryString.end() != ++m_currentChar && std::isalpha(*m_currentChar));

        // If we stopped at the end of the token, check if this is a keyword
        if (reachedTokenEnd()) {
          
            // Check to see if we have a mapping to a keyword token
            std::string lexeme(m_tokenStart, m_currentChar);
            Token keywordToken = getKeywordTokenFromString(lexeme);
            if (std::holds_alternative<EOFToken>(keywordToken)) {

                // this is not really an EOFToken, EOFToken is just used in this method to say "token not found"
                return UserToken{lexeme};
            }

            return keywordToken;
        } 
        // Check to see if we can transition to a User token
        // is there a better heuristic? almost certainly.
        else if (isValidUserCharacter(*m_currentChar)) { 

            // this miiiiiight be a User token
            return scanUserToken();
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

        // This could potentially be a User token if a dot separater is encountered, hand things off to the 
        // scanUserToken method. Note: This is a safe dereference bceause we already know m_currentChar is not at
        // m_queryString.end() because of hte call to reachedTokenEnd()
        else if (isValidUserCharacter(*m_currentChar)) {

            return scanUserToken();
        }

        return scanErrorToken();
        
    }


    Token Lexer::scanComparisonToken() {

        // Scan forwards one character and check to see if we have reached the end of our token. Also
        // check to see if we possibly have a two character comparison token, if not, it won't be present in our map
        m_currentChar++;
        if(reachedTokenEnd() || (*(m_currentChar++) == isElementPresent<char>{'=', '>'} && reachedTokenEnd())) {

            static std::map<std::string, ComparisonToken::OpType> operatorMap {
                {"=" , ComparisonToken::OP_EQ},
                {">" , ComparisonToken::OP_GT},
                {"<" , ComparisonToken::OP_LT},
                {">=", ComparisonToken::OP_GTE},
                {"<=", ComparisonToken::OP_LTE},
                {"<>", ComparisonToken::OP_NE}
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
            case '*': m_currentChar++; return PunctuationToken<'*'>{ };
            case '(': m_currentChar++; return PunctuationToken<'('>{ };
            case ')': m_currentChar++; return PunctuationToken<')'>{ };
            case ',': m_currentChar++; return PunctuationToken<','>{ };
            case ';': m_currentChar++; return PunctuationToken<';'>{ }; // return EOF here? this isn't being handled correctly.
        }
       

        // next, lets try scanning some comparison operators
        if (*m_currentChar == isElementPresent<char>{'=', '>', '<'}) {
            // The next character is potentially the beginning of a comparison operator
            return scanComparisonToken();
        }

        // next, is this a digit
        else if (std::isdigit(*m_currentChar)) {
            // this could return a number, an error token, or a User possibly
            return scanNumericToken();
        }

        // next is this a letter
        else if (std::isalpha(*m_currentChar)) {
            // this could return a keyword, a column, a User, or an error token
            return scanAlphaToken();
        }
        
        // nothing else matches, return error case.
        return scanErrorToken();
    }

}
