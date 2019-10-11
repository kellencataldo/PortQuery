#include "ArgumentParser.h"
#include <iostream>


int main(int argc, char* args[]) {

    argumentParser parser(argc, args);
    if(!parser.parse()) {
        return static_cast<unsigned int>(-1);
    }
    
    return 0;
}
