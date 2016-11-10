#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"

#include <iostream>


class Logger {
public:
    void logException(const char* subsystem, const std::exception &e){
        std::cout << "Exception occurred in subsystem " << subsystem << ": "
             << e.what() << ".";
        std::cout.flush();
    }

    void logException(const char* subsystem, const Exception &e){
        std::cout << "Exception occurred in subsystem " << subsystem << ": "
                  << e.message() << ".";
        std::cout.flush();
    }
};

#endif //GEO_NETWORK_CLIENT_LOGGER_H
