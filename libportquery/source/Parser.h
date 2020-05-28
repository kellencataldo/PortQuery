#pragma once

#include <string>
#include <memory>

#include "Lexer.h"


struct ASTNodeBase {

    // public:
    //      void accept(SOSQL visitor) = 0

};


// The three SelectNode structs form the base of the abstract syntax tree
// The parse function could return 

struct ASTColumnSelectNode : ASTNodeBase {


};


struct ASTCountSelectNode : ASTNodeBase {


};


using ASTNode = std::shared_ptr<ASTNodeBase>;

class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // SELECT Only SQL
        // maybe make this a unique_ptr? investigation required.
        ASTNode parseSOSQLStatement();


    private:
        ASTNode parseCountSelect();
        ASTNode parseColumnSelect();

        Lexer m_lexer;
};
