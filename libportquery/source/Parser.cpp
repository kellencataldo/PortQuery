#include "Parser.h"


std::shared_ptr<ASTNodeBase> Parser::parseSOSQLStatement() {

    Token t = m_lexer.nextToken();

    // SOSQL statements can obviously only begin with the "SELECT" keyword
    if (!std::holds_alternative<KeywordToken>(t) || KeywordToken::SELECT != std::get<KeywordToken>(t).m_keyword) {

        throw std::invalid_argument("Only SELECT statements are handled. Statement must begin with SELECT");
    }

    return parseSetQuantifier();
}


std::shared_ptr<ASTNodeBase> Parser::parseSetQuantifier() {

    Token t = m_lexer.nextToken();
    
    if (std::holds_alternative<KeywordToken>(t)) {

        KeywordToken kt = std::get<KeywordToken>(t);
        if (KeywordToken::COUNT == kt.m_keyword) {
            return parseCountSelect();
        }

        else if (KeywordToken::DISTINCT == kt.m_keyword) {
            return parseDistinctSelect();
        }

        throw std::invalid_argument("Invalid keyword provided as SELECT quantifier: " + KeywordToken::lookupStringByKeyword(kt.m_keyword));
    }

    return parseColumnSelect();
}


std::shared_ptr<ASTNodeBase> Parser::parseCountSelect() {
    return NULL;
}


std::shared_ptr<ASTNodeBase> Parser::parseDistinctSelect() {
    return NULL;
}


std::shared_ptr<ASTNodeBase> Parser::parseColumnSelect() {
    return NULL;
}
