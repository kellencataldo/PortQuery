#pragma once

#include <string>
#include <memory>

#include "Lexer.h"
#include "Network.h"


struct ASTNodeBase {

    // public:
    //      void accept(SOSQL visitor) = 0

};


struct SelectSet {
    SelectSet() : m_selectPort(false) { m_selectedProtocols.m_value = 0; }

    bool m_selectPort;
    Network::protocolFlags m_selectedProtocols;
};


// The three SelectNode structs form the base of the abstract syntax tree
// The parse function could return 

struct SelectStatement {

    SelectSet m_selectSet;
};

typedef SelectStatement SOSQLSelectStatement;

class Parser { 

    public:
        Parser(const std::string& queryString) : m_lexer(queryString) { }

        // SELECT Only SQL
        // maybe make this a unique_ptr? investigation required.
        SOSQLSelectStatement parseSOSQLStatement();


    private:
        SOSQLSelectStatement parseColumnSelect();
        SOSQLSelectStatement parseCountSelect();

        Lexer m_lexer;
};
