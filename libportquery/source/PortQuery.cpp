#include <iostream>

#include "PortQuery.h"
#include "Parser.h"


bool PortQuery::prepare(std::string queryString) {

    if (m_selectStatement) {

        m_errorString = "Previous query has not been finalized";
        return false;
    }

    Parser parseEngine{queryString};
    try {

        m_selectStatement = parseEngine.parseSOSQLStatement();
    } 
    catch (std::invalid_argument& e) {

        m_errorString = e.what();
        return false;
    }

    m_errorString.clear();
    return true;
}


bool PortQuery::run() {



    return true;
}


bool PortQuery::finalize() {
    if (m_selectStatement) {

        m_selectStatement.reset();
        m_errorString.clear();
        return true;
    }

    m_errorString = "No query prepared";
    return false;
}

bool PortQuery::execute(std::string queryString) { 
    return false;
}


