#include <iostream>
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
        return static_cast<unsigned int>(-1);
    }

    std::string queryString = parser.getQueryString();
    portQuery pq;
    pq.printTest();
    return 0;
}
