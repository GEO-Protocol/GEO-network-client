#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"
#include "../common/time/TimeUtils.h"
#include "../common/NodeUUID.h"

#include <iostream>
#include <sstream>
#include <string>


using namespace std;


class Logger;

/**
 * Logger stream is used as interface for the logger.
 * It collects information
 */
class LoggerStream:
    public stringstream {

public:
    enum StreamType {
        Standard = 0,

        // Used into the transactions.
        // Helps distinquish transactions log from other logs flows.
        Transaction,

        //...
        // Other logs types must be located here

        // Dummy logger is used in configurations,
        // where no log records should be issued,
        // but the logger structure is required by some type of signature(s).
        Dummy,
    };

public:
    explicit LoggerStream(
        Logger *logger,
        const string &group,
        const string &subsystem,
        const StreamType type = Standard);

    LoggerStream(
        const LoggerStream &other);

    ~LoggerStream();

    static LoggerStream dummy();

private:
    Logger *mLogger;
    const string mGroup;
    const string mSubsystem;
    const StreamType mType;
};


class Logger {
    friend class LoggerStream;

public:
    Logger(
        const NodeUUID &nodeUUID);

    void logException(
        const string &subsystem,
        const exception &e);

    LoggerStream info(
        const string &subsystem);

    LoggerStream error(
        const string &subsystem);

    LoggerStream debug(
        const string &subsystem);

    LoggerStream warning(
        const string &subsystem);

protected:
    const uint32_t maxRotateLimit = 500000;

protected:
    const string formatMessage(
        const string &message) const;

    const string recordPrefix(
        const string &group);

    void logRecord(
        const string &group,
        const string &subsystem,
        const string &message);

    void rotate();

    bool formatLogFileName(
        string& str,
        const string& from,
        const string& to);

    void calculateOperationsLogFileLinesNumber();

private:
    NodeUUID mNodeUUID;
    std::ofstream mOperationsLogFile;
    uint32_t mOperationsLogFileLinesNumber;
    string mOperationLogFileName;
};
#endif //GEO_NETWORK_CLIENT_LOGGER_H