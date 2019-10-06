#include "CommandParser.h"
#include <iostream>


int main(int argc, char* args[]) {
    commandParser parser(argc, args);
    parser.addOption<int>("--cole", "retard", 5);
    parser.addOption<int>("--colee", "retard", 12);
    parser.addOptionList<int>("--colerrr", "asshole", 10, 20);
    parser.parse();

    return 0;
}
