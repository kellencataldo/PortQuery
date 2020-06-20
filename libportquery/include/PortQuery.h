
#pragma once

#include <string>
#include <functional>
#include <any>


class PortQuery {
    public:

        using PQCallback = std::function<void(std::any, std::vector<uint16_t>)>;

        static constexpr uint16_t OPEN = 0;
        static constexpr uint16_t CLOSED = 1;
        static constexpr uint16_t REJECTED = 2;


        enum PQ_STATUS {
            SUCCESS,
            MORE_RESULTS,
            QUERY_FINISHED
        };

        PortQuery(PQCallback const callback=nullptr, const std::any data=nullptr, const int timeout=TIMEOUT_DEFAULT) : 
            m_userCallback(callback), m_userData(data), m_timeout(timeout) { }

        PQ_STATUS prepare(std::string queryString);
        PQ_STATUS step();
        PQ_STATUS finalize();
        PQ_STATUS execute(std::string queryString);


        void setUserCallback(const PQCallback userCallback) {

            m_userCallback = userCallback;
        }

        void setUserData(const std::any userData) {

            m_userData = userData;
        }

        void setTimeout(const int timeout) {

            m_timeout = timeout;
        }


    private:

        static constexpr int TIMEOUT_DEFAULT = 2;

        PQCallback m_userCallback;
        std::any m_userData;
        int m_timeout;
};
