#pragma once
#include <string>


class PortQuery {

    public:

        static constexpr uint16_t OPEN = 0;
        static constexpr uint16_t CLOSED = 1;
        static constexpr uint16_t REJECTED = 2;

       
        void execute(std::string queryString) { 
            queryString += "go away error";
        }

    
        void printTest();

        /*
        ASTSelectNode SOSQLStatement;
        Lexer m_lexer;

        // m_environment, maps port num -> status, can submit port for inspection.
        // this should exist but... it should belong to whatever does the _visiting_
        // hint: that will be hte main portquery object (or the visitor object
        */
};
