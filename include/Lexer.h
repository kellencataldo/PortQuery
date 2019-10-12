#pragma once
#include <string>

class token { };


class lexer {
    public:
        explicit lexer(const std::string queryString) : m_queryString(queryString) { };
        token getNextToken(void) const;

    private:
        std::string m_queryString;
};
