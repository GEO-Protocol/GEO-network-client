#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"
#include "../common/NodeUUID.h"
#include "../common/Types.h"
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
    LoggerStream(const LoggerStream &other);
    ~LoggerStream();

private:
    Logger *mLogger;
    const char *mGroup;
    const char *mSubsystem;
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
    void logTustlineState(const NodeUUID &conractorUUID, string direction, string status);
    void logTruslineOperationStatus(
            const NodeUUID &contractorUUID,
            const TrustLineAmount &incoming_amount,
            const TrustLineAmount &outgoing_amount,
            const TrustLineBalance &balance,
            const TrustLineDirection &direction
    );

    LoggerStream info(
        const char *subsystem);
    LoggerStream error(
        const char *subsystem);

private:
    const string log_filename = "transactionlog.txt";
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
