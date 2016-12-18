#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"

#include <iostream>
#include <string>


using namespace std;

class Logger {
public:
    void logException(const char *subsystem, const exception &e) {
        auto m = string(e.what());
        logRecord("EXCEPT", subsystem, m);
    }

    void logInfo(const char *subsystem, const string &message){
        logRecord("INFO", subsystem, message);
    }

    void logSuccess(const char *subsystem, const string &message){
        logRecord("SUCCESS", subsystem, message);
    }

    void logError(const char *subsystem, const string &message){
        logRecord("ERROR", subsystem, message);
    }

    void logFatal(const char *subsystem, const string &message){
        logRecord("FATAL", subsystem, message);
    }

private:
    const string formatMessage(const string &message) const {
        if (message.size() == 0) {
            return message;
        }

        auto m = message;
        if (m.at(m.size()-1) != '.') {
            m += ".";
        }

        return m;
    }

    const string recordPrefix(const char *group) {
        // todo: add timestamp
        return string(group) + string("\t\t");
    }

    void logRecord(const char *group, const char *subsystem, const string &message) {
        cout << recordPrefix(group)
             << subsystem << "\t\t\t"
             << formatMessage(message) << endl;
        cout.flush();
    }
};

#endif //GEO_NETWORK_CLIENT_LOGGER_H
