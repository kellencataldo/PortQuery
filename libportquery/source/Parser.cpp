#include "Parser.h"

// helper class for std::visit. Constructs a callable which accepts various types based on deduction
template<class ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


ASTNode Parser::parseSOSQLStatement() {

    // SOSQL statements can obviously only begin with the "SELECT" keyword
    if (!std::holds_alternative<SELECTToken>(m_lexer.nextToken())) {

        throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
    }

    return std::visit(overloaded {
            [=] (COUNTToken)  { return parseCountSelect(); },
            [=] (auto) { return parseColumnSelect(); } }, 
            m_lexer.peek());
}


ASTNode Parser::parseColumnSelect() {

    ASTNode columnSelect = std::make_shared<ASTColumnSelectNode>();

    

    return NULL;
}


ASTNode Parser::parseCountSelect() {

    // scan past the COUNT token
    m_lexer.nextToken();
    if (!std::holds_alternative<PunctuationToken<'('>>(m_lexer.nextToken())) {

        throw std::invalid_argument("COUNT aggregates must be enclosed in parentheses");
    }


    // parse column list:::

    return NULL;
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

