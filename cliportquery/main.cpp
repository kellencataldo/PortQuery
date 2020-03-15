#include <iostream>
#include <cstdlib>


#include "ArgumentParser.h"
#include "PortQuery.h"


int main(int argc, char* args[]) {

    argumentParser<STDOutput> parser(argc, args);
    if(!parser.parse()) {
        // print output here.
        return EXIT_FAILURE;
    }

    std::string queryString = parser.getQueryString();
    portQuery pq;
    pq.printTest();
    return 0;
}
