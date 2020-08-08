#pragma once

#include <cstdint>
#include <memory>

#include "Network.h"
#include "ThreadPool.h"


namespace PortQuery {


    class IEnvironment {

        public:

            virtual bool submitPortForScan(const uint16_t port, const NetworkProtocol requestedProtocols) = 0;
            virtual void setPort(const uint16_t port);
            virtual bool getPort(void) const;

        private:
            uint16_t m_port;
    };


    using GeneratorFunction = std::shared_ptr<IEnvironment> (*)(const int threadCount);
    using EnvironmentPtr = std::shared_ptr<IEnvironment>;


    class NetworkEnvironment : public IEnvironment {

        public:

            NetworkEnvironment(const int threadCount) : m_threadPool(threadCount) { }
            virtual bool submitPortForScan(const uint16_t port, NetworkProtocol requestedProtocols) override;

        private:

            ThreadPool m_threadPool;
    };


    class EnvironmentFactory {

        public:

             static void setGenerator(const GeneratorFunction generator);
             static EnvironmentPtr createEnvironment(const unsigned int threadCount);

        private:
            static EnvironmentPtr defaultGenerator(const int threadCount) {

                EnvironmentPtr out = std::make_shared<NetworkEnvironment>(threadCount);
                return out;
            }

            static GeneratorFunction m_generator;
    };

}
