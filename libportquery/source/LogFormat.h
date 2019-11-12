#pragma once
#include <string>



/* macros for various compilers should go here*/


std::string formatMessage(const char* fileName, const char* functionName, int lineNumber,
        const std::string message) {
    return std::string(">") + fileName + ":" + functionName + ":" + std::to_string(lineNumber) + "::" + message;
}
