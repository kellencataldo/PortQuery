#include <iostream>
#include <cstdlib>

#include "ArgumentParser.h"
#include "PortQuery.h"

#define UNUSED(x) (void)(x)


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


class QueryContext {


};


void QueryCallback(std::any callbackContext, std::vector<uint16_t> columns) {

    QueryContext* context = std::any_cast<QueryContext*>(callbackContext);
    UNUSED(columns);
}


int main(int argc, char* args[]) {

    ArgumentParser<STDOutput> parser(argc, args);
    parser.addCommand<int>("--timeout", "duration in seconds to wait on a response", 2);
    parser.addCommand<int>("--threads", "number of threads to use (0 = number of processors on the machine", 0);

    if(!parser.parse()) {

        return EXIT_FAILURE;
    }

    const std::string queryString = parser.getQueryString();
    const int timeout = parser.getCommand<int>("--timeout");
    const int threadCount = parser.getCommand<int>("--threads");

    std::unique_ptr<QueryContext> context = std::make_unique<QueryContext>(QueryContext{});

    PortQuery pq{ QueryCallback, context.get(), timeout, threadCount};

    pq.execute("cool");
    const char ugh[] = "coool";
    pq.execute(ugh);

    return 0;
}
