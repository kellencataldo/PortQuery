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


struct ASTDistinctColumnSelectNode : ASTNodeBase {


};


struct ASTCountSelectNode : ASTNodeBase {


};


class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // SELECT Only SQL
        // maybe make this a unique_ptr? investigation required.
        std::shared_ptr<ASTNodeBase> parseSOSQLStatement();


    private:
        std::shared_ptr<ASTNodeBase> parseSetQuantifier();
        std::shared_ptr<ASTNodeBase> parseCountSelect();
        std::shared_ptr<ASTNodeBase> parseDistinctSelect();
        std::shared_ptr<ASTNodeBase> parseColumnSelect();

        Lexer m_lexer;
};
