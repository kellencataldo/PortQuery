#pragma once
#include <string>


class PortQuery {

    public:

        enum QueryResult : uint16_t {
            OPEN =      0,
            CLOSED =    1,
            REJECTED =  2,
        };

        struct ColumnType {
            union {
                QueryResult result;
                uint16_t port;
            };
        };
        
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
