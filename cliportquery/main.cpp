#include <iostream>
#include <cstdlib>

#include "ArgumentParser.h"
#include "PortQuery.h"


struct STDOutput {
    public:
        static void output(const std::string outputString) { 
            std::cout << outputString;
        }

        static void setWidth(const size_t width) { 
            std::cout.width(width);
        }

    protected:
        ~STDOutput() { }
};


int main(int argc, char* args[]) {

    ArgumentParser<STDOutput> parser(argc, args);
    parser.addCommand<int>("--timeout", "duration in seconds to wait on a response", 2);

    if(!parser.parse()) {

        return EXIT_FAILURE;
    }

    const int timeout = parser.getCommand<int>("--timeout");
    const std::string queryString = parser.getQueryString();


    PortQuery pq{};

    return 0;
}
