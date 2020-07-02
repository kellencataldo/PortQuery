
#pragma once

#include <string>
#include <functional>
#include <any>

#include "../source/Parser.h"


class PortQuery {

    public:

        static constexpr uint16_t OPEN = 0;
        static constexpr uint16_t CLOSED = 1;
        static constexpr uint16_t REJECTED = 2;

        using PQCallback = std::function<void(std::any, std::vector<uint16_t>)>;
        PortQuery(PQCallback const callback=nullptr, const std::any context=nullptr, const int timeout=TIMEOUT_DEFAULT) : 
            m_userCallback(callback), m_userContext(context), m_timeout(timeout), m_selectStatement(nullptr) { }

        bool prepare(std::string queryString);
        bool run();
        bool finalize();
        bool execute(std::string queryString);


        void setUserCallback(const PQCallback userCallback) {

            m_userCallback = userCallback;
        }

        void setUserData(const std::any userContext) {

            m_userContext = userContext;
        }

        void setTimeout(const int timeout) {

            m_timeout = timeout;
        }

        std::string getErrorString() const {

            return m_errorString;
        }

    private:

        static constexpr int TIMEOUT_DEFAULT = 2;
        int m_timeout;

        PQCallback m_userCallback;
        std::any m_userContext;

        SOSQLSelectStatement m_selectStatement;
        std::string m_errorString;
};
