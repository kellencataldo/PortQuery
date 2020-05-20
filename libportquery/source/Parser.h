#pragma once

#include <string>

#include "Lexer.h"


struct ASTNodeBase {

    // public:
    //      void accept(SOSQL visitor)

};

struct ASTSelectNode : ASTNodeBase {


};


class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // This should return something in the future
        // SELECT Only SQL
        void parseSOSQLStatement();


    private:

        void parserSelectList();
        void parseTableExpression();

        ASTSelectNode SOSQLStatement;
        Lexer m_lexer;

        // m_environment, maps port num -> status, can submit port for inspection.
};
