#include <iostream>
#include <cstdlib>

#include "ArgumentParser.h"
#include "PortQuery.h"


struct STDOutput {
    public:
        static void output(const std::string outputString) { 
            std::cout << outputString << std::endl;
        }

        static void setWidth(const size_t width) { 
            std::cout.width(width);
        }

    protected:
        ~STDOutput() { }
};


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
