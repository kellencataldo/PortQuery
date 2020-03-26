#include <iostream>

#include "PortQuery.h"
#include "Lexer.h"

void portQuery::printTest() {
    std::cout << "linked correctly" << std::endl;
    Lexer lex("just a dumb example");
    Token fuck = lex.nextToken();

}
