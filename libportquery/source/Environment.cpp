#include "Environment.h"


class NetworkEnvironment : public IEnvironment {

    public:
        NetworkEnvironment(const int threadCount) : m_threadCount(threadCount) { }
        virtual bool submitPortForScan(const uint16_t port, NetworkProtocols requestedProtocols) override {

            return true;
        }

    private:

        uint16_t m_threadCount; 
};


EnvironmentPtr EnvironmentFactory::defaultGenerator(const int threadCount) {

    return std::make_shared<NetworkEnvironment>(NetworkEnvironment{threadCount});
}

GeneratorFunction EnvironmentFactory::m_generator = defaultGenerator;
