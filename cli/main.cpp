#include "ArgumentParser.h"
#include <iostream>


int main(int argc, char* args[]) {

    argumentParser parser(argc, args);

    parser.parse();

   return 0;
}
