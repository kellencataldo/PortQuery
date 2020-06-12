#include "Parser.h"

#include <iostream>

// helper class for std::visit. Constructs a callable which accepts various types based on deduction
template<class ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;



std::string GetTokenString(const Token t) {

    return std::visit(overloaded {
                [=] (NumericToken n)  { return std::to_string(n.m_value); },
                [=] (ComparisonToken) { return std::string("[COMPARISON TOKEN]"); },
                [=] (UserToken u)     { return u.m_UserToken; },
                [=] (ALLToken)        { return std::string("[ALL KEYWORD]"); },
                [=] (ANDToken)        { return std::string("AND KEYWORD]"); },
                [=] (ANYToken)        { return std::string("[ANY KEYWORD]"); },
                [=] (BETWEENToken)    { return std::string("[BETWEEN KEYWORD]"); },
                [=] (COUNTToken)      { return std::string("[COUNT KEYWORD]"); },
                [=] (FROMToken)       { return std::string("[FROM KEYWORD]"); },
                [=] (IFToken)         { return std::string("[IF KEYWORD]"); },
                [=] (INToken)         { return std::string("[IN KEYWORD]"); },
                [=] (ISToken)         { return std::string("[IS KEYWORD]"); },
                [=] (LIKEToken)       { return std::string("[LIKE KEYWORD]"); },
                [=] (LIMITToken)      { return std::string("[LIMIT KEYWORD]"); },
                [=] (NOTToken)        { return std::string("[NOT KEYWORD]"); },
                [=] (ORToken)         { return std::string("[OR KEYWORD]"); },
                [=] (ORDERToken)      { return std::string("[ORDER KEYWORD]"); },
                [=] (SELECTToken)     { return std::string("[SELECT KEYWORD]");},
                [=] (WHEREToken)      { return std::string("[WHERE KEYWORD]"); },
                [=] (PORTToken)       { return std::string("[PORT  KEYWORD]"); },
                [=] (ProtocolToken)   { return std::string("[PROTOCOL TOKEN]"); }, // more granularity please
                [=] (EOFToken)        { return std::string("[END OF INPUT]"); },
                [=] (PunctuationToken<'*'>) { return std::string("[ * ]"); },
                [=] (PunctuationToken<'('>) { return std::string("[ ( ]"); }, 
                [=] (PunctuationToken<')'>) { return std::string("[ ) ]"); },
                [=] (PunctuationToken<';'>) { return std::string("[ ; ]"); }, 
                [=] (PunctuationToken<','>) { return std::string("[ , ]"); },
                [=] (auto) -> std::string { return std::string("[UNKNOWN TOKEN]"); } }, 
            t);
    }


SOSQLSelectStatement Parser::parseSOSQLStatement() {

    // SOSQL statements can obviously only begin with the "SELECT" keyword
    if (!std::holds_alternative<SELECTToken>(m_lexer.nextToken())) {

        throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
    }

    return parseSimpleSelect();

}


SOSQLSelectStatement Parser::parseSimpleSelect() {


    SOSQLSelectStatement selectStatement { };
    selectStatement.m_selectSet = parseSelectSetQuantifier();




    // parse where statement here.




    return selectStatement;
}


SelectSet Parser::parseSelectSetQuantifier() {

    return std::visit(overloaded {
            [=] (ProtocolToken)         { return parseSelectList(); },
            [=] (PORTToken)             { return parseSelectList(); },
            [=] (PunctuationToken<'*'>) { return SelectSet{ true, (NetworkProtocols::TCP|NetworkProtocols::UDP)}; },

            [=] (auto t) -> SelectSet { 
                const std::string exceptionString = "Invalid token in select list: " + GetTokenString(t);
                throw std::invalid_argument(exceptionString); 
            } },
        m_lexer.peek());

};

SelectSet Parser::parseSelectList() {

    SelectSet selectedColumns{false, NetworkProtocols::NONE};

    // nextToken() gets advanced in the for loop declaration here only if a common token is 
    // detected after whatever column token gets scanned in the body of the for loop.
    for (bool moreColumns = true; moreColumns; m_lexer.nextToken()) {
        std::visit(overloaded { 
                [&selectedColumns] (PORTToken) { 
                    if (selectedColumns.m_selectPort) { 
                        throw std::invalid_argument("Duplicate PORT column specified");
                    }
                    selectedColumns.m_selectPort = true; 
                },

                [&selectedColumns] (ProtocolToken p) {
                    if (NetworkProtocols::NONE != (p.m_protocol & selectedColumns.m_selectedProtocols)) { 
                        throw std::invalid_argument("Duplicate protocol column specified"); 
                    }
                    selectedColumns.m_selectedProtocols |= p.m_protocol;
                },

                [=] (auto t) -> void { 
                    std::string exceptionString = "Invalid token specified in select list: " + GetTokenString(t);
                    throw std::invalid_argument(exceptionString);
                } },
           
            m_lexer.nextToken());

        // if there is a comma here, there are more columns to scan. If there is not a comma,
        // parsing the column list is complete and the for loop is exited
        moreColumns = std::holds_alternative<PunctuationToken<','>>(m_lexer.peek());
    }

    return selectedColumns;
}


/*
ASTNode Parser::parseSetQuantifier() {

    return std::visit(overloaded {
            [=] (COUNTToken)  { return parseCountSelect(); },
            [=] (ColumnToken) { return parseColumnSelect(); },
            [=] (PunctuationToken<'('>) { return parseColumnSelect(); },
            [=] (auto) -> ASTNode { throw std::invalid_argument("Invalid token following SELECT keyword"); } }, 
            m_lexer.peek());
}

SOSQLSelectStatement Parser::parseCountSelect() {

    // scan past the COUNT token
    m_lexer.nextToken();
    if (!std::holds_alternative<PunctuationToken<'('>>(m_lexer.nextToken())) {

        throw std::invalid_argument("COUNT aggregates must be enclosed in parentheses");
    }


    // parse column list here

    return SOSQLSelectStatement();
}

*/
