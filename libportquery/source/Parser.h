#pragma once

#include <string>

#include "Lexer.h"


class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // This should return something in the future
        void parse();


    private:

        Lexer m_lexer;
};
