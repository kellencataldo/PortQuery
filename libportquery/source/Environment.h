#pragma once

#include <cstdint>
#include <memory>

#include "Network.h"
#include "ThreadPool.h"


namespace PortQuery {


    class IEnvironment {

        public:

            virtual bool scanPort(void) = 0;
            virtual void setProtocolsToScan(const NetworkProtocol protocols);
            virtual void setPort(const uint16_t port);
            virtual uint16_t getPort(void) const;

        private:
            uint16_t m_port;
            NetworkProtocol m_protocolsToScan;
    };


    using GeneratorFunction = std::shared_ptr<IEnvironment> (*)(const int threadCount);
    using EnvironmentPtr = std::shared_ptr<IEnvironment>;


    class NetworkEnvironment : public IEnvironment {

        public:

            NetworkEnvironment(const int threadCount) : m_threadPool(threadCount) { }
            virtual bool scanPort(void) override;

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
