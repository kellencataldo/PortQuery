#include "Parser.h"

#include <iostream>

// helper class for std::visit. Constructs a callable which accepts various types based on deduction
template<class ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;



std::string GetTokenString(const Token t) {

    return std::visit(overloaded {
            [=] (NumericToken n) { return std::to_string(n.m_value); },
            // ComparisonToken, << fix this.
            // [=] (UserToken u)    { return u.m_UserToken; },
            [=] (ALLToken)       { return std::string("ALL"); },
            [=] (ANDToken)       { return std::string("AND"); },
            
            [=] (ANYToken)       { return std::string("ANY"); },
            [=] (BETWEENToken)   { return std::string("BETWEEN"); },
            [=] (COUNTToken)     { return std::string("COUNT"); },
            [=] (FROMToken)      { return std::string("FROM"); },
            [=] (IFToken)        { return std::string("IF"); },
/*
           
            INToken,
            ISToken,
            LIKEToken,
            LIMITToken,
            NOTToken,
            ORToken,
            ORDERToken,
            SELECTToken,
            WHEREToken,
            PORTToken,
            PunctuationToken<'*'>,
            PunctuationToken<'('>,
            PunctuationToken<')'>,
            PunctuationToken<','>,
            PunctuationToken<';'>,
            */


            [=] (PunctuationToken<','>) { return std::string("[,]"); },

            [=] (ProtocolToken) { return std::string("PROTOCOL TOKEN"); }, // more granularity please
            [=] (auto) -> std::string { return std::string("[UNKNOWN TOKEN]"); } }, 
            t);
}


SOSQLSelectStatement Parser::parseSOSQLStatement() {

    // SOSQL statements can obviously only begin with the "SELECT" keyword
    if (!std::holds_alternative<SELECTToken>(m_lexer.nextToken())) {

        throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
    }

    return std::visit(overloaded {
            [=] (COUNTToken)  { return parseCountSelect(); },
            [=] (auto) { return parseColumnSelect(); } }, 
            m_lexer.peek());
}


SOSQLSelectStatement Parser::parseColumnSelect() {
    SOSQLSelectStatement selectStatement{};
    selectStatement.m_selectSet = parseSelectSet();
    return selectStatement;
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


SelectSet Parser::parseSelectSet() {

    return std::visit(overloaded {
            [=] (ProtocolToken)         { return parseSelectList(); },
            [=] (PORTToken)             { return parseSelectList(); },
            [=] (PunctuationToken<'*'>) { return SelectSet{ true, (NetworkProtocols::TCP|NetworkProtocols::UDP)}; },

            // add in erroneous token here?
            [=] (auto) -> SelectSet     { throw std::invalid_argument("Unknown or invalid column selected"); } },
        m_lexer.peek());

};

SelectSet Parser::parseSelectList() {

    SelectSet selectedColumns{false, NetworkProtocols::NONE};

    bool moreColumns = true;

    while (moreColumns) {

        std::visit(overloaded { 
                [&selectedColumns] (PORTToken) { 
                    if (selectedColumns.m_selectPort) { throw std::invalid_argument("Duplicate PORT column specified"); }
                    selectedColumns.m_selectPort = true; 
                },

                [&selectedColumns] (ProtocolToken p) {
                    if (NetworkProtocols::NONE != (p.m_protocol & selectedColumns.m_selectedProtocols)) { 
                        throw std::invalid_argument("Duplicate protocol colum specified"); }
                    selectedColumns.m_selectedProtocols |= p.m_protocol;
                },

                [=] (auto t) -> void { 
                    std::string exceptionString = "Invalid token specified in select list: " + GetTokenString(t);
                    throw std::invalid_argument(exceptionString);
                } },
                m_lexer.nextToken());


        if (std::holds_alternative<PunctuationToken<','>>(m_lexer.peek())) {

            m_lexer.nextToken();
        } 
        else {

            // if there are more columns to follow, it's someone elses problem
            moreColumns = false;
        }
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

*/

