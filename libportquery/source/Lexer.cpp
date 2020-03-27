#include <exception>

#include "Lexer.h"


Token Lexer::nextToken() {

    // This is an optimization for the scenario that the next token has been scanned and identified
    // but the input hasn't been advanced. This occurs when a token is "peeked"

    // First, check to see if we have already parsed the next token, just not advanced the input
    if (std::nullopt != m_peekToken) {
        // move the value that was in the peek token to the return token
        Token outToken(std::move(m_peekToken.value()));
        // Reset the peek token to an empty state, will be set again next time peek() is called
        m_peekToken.reset();
        // Return the token to the user
        return outToken;
    }

    // The peek token was empty, parse the next token the usual way.
    return EOFToken{};

}


