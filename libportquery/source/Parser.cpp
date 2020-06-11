#include "Parser.h"

// helper class for std::visit. Constructs a callable which accepts various types based on deduction
template<class ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


NetworkProtocols ConvertTokenColumnToNetworkProtocol(const ColumnToken::Column column) {

    NetworkProtocols protocol = NetworkProtocols::NONE;
    switch(column) {
        
        case ColumnToken::TCP:
            protocol = NetworkProtocols::TCP;
            break;

        case ColumnToken::UDP:
            protocol = NetworkProtocols::UDP;
            break;

        case ColumnToken::PORT:
            throw std::invalid_argument("Unable to convert token column to network protocol");
    }

    return protocol;
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
        [=] (ColumnToken) { return parseSelectList(); },
        [=] (PunctuationToken<'*'>) { return SelectSet{ true, (NetworkProtocols::TCP|NetworkProtocols::UDP)}; },

        // add in erroneous token here?
        [=] (auto) -> SelectSet { throw std::invalid_argument("Unknown or invalid column selected"); } },
        m_lexer.peek());

};

SelectSet Parser::parseSelectList() {

    SelectSet selectedColumns{false, NetworkProtocols::NONE};

    while (bool moreColumns = true) {
        
        Token t = m_lexer.nextToken();
        if (!std::holds_alternative<ColumnToken>(t)) {

            // add in erroneous token here?
            throw std::invalid_argument("Unknown or invalid column selected");
        }

        ColumnToken column = std::get<ColumnToken>(t);
        if (ColumnToken::PORT == column.m_column) { 

            if (selectedColumns.m_selectPort) { 
                throw std::invalid_argument("Duplicate column specified");
            }

            selectedColumns.m_selectPort = true;

        } else {

            const NetworkProtocols protocol = ConvertTokenColumnToNetworkProtocol(column.m_column);
            if (NetworkProtocols::NONE != (protocol & selectedColumns.m_selectedProtocols)) {
                // add which erroneous token was selected here.
                throw std::invalid_argument("Duplicate column specified");
            }

            selectedColumns.m_selectedProtocols |= protocol;
        }

        if (std::holds_alternative<PunctuationToken<','>>(m_lexer.peek())) {
            m_lexer.nextToken();
        } else {

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

