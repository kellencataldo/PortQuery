#include "Parser.h"


void Parser::parseSOSQLStatement() {

    Token t = m_lexer.nextToken();


    // SOSQL statements can obviously only begin with the "SELECT" keyword
    if (!std::holds_alternative<KeywordToken>(t) || KeywordToken::SELECT != std::get<KeywordToken>(t).m_keyword) {

        throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
    }



    // sqlNode.columns to select = parseSelect list;



}
