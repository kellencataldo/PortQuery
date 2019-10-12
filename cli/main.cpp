#include <iostream>
#include "ArgumentParser.h"
#include "PortQuery.h"


int main(int argc, char* args[]) {

    argumentParser parser(argc, args);
    if(!parser.parse()) {
        return static_cast<unsigned int>(-1);
    }

    std::string queryString = parser.getQueryString();
    portQuery pq;
    pq.printTest();
    return 0;
}
