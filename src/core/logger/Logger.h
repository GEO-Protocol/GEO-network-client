#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"

#include <iostream>
#include <sstream>
#include <string>


using namespace std;


class Logger;
class LoggerStream:
    public stringstream {

public:
    explicit LoggerStream(
        Logger *logger,
        const char *group,
        const char *subsystem);

    explicit LoggerStream(
        Logger *logger,
        const char *group,
        const string &subsystem);

    LoggerStream(const LoggerStream &other);
    ~LoggerStream();

private:
    Logger *mLogger;
    const string mGroup;
    const string mSubsystem;
};


class Logger {
    friend class LoggerStream;

public:
    void logException(
        const char *subsystem,
        const exception &e);
    void logInfo(
        const char *subsystem,
        const string &message);
    void logSuccess(
        const char *subsystem,
        const string &message);
    void logError(
        const char *subsystem,
        const string &message);
    void logFatal(
        const char *subsystem,
        const string &message);

    LoggerStream info(
        const string &subsystem);
    LoggerStream error(
        const string &subsystem);
    LoggerStream debug(
        const string &subsystem);

private:
    const string formatMessage(
        const string &message) const;
    const string recordPrefix(
        const char *group);
    void logRecord(
        const char *group,
        const char *subsystem,
        const string &message);
};


#endif //GEO_NETWORK_CLIENT_LOGGER_H
