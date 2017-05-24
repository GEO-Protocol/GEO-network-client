﻿#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"
#include "../common/time/TimeUtils.h"

#include <iostream>
#include <sstream>
#include <string>


using namespace std;


class Logger;
class LoggerStream:
    public stringstream {

public:
    enum StreamType {
        Standard = 0,
        Transaction,
        Dummy
    };

public:
    // Dummy logger
    explicit LoggerStream();

    explicit LoggerStream(
        Logger *logger,
        const string &group,
        const string &subsystem,
        const StreamType type = Standard);

    LoggerStream(const LoggerStream &other);

    ~LoggerStream();

    static LoggerStream dummy();

private:
    Logger *mLogger;
    const string mGroup;
    const string mSubsystem;
    const StreamType mType;
    const string mLogfile = "operations.log";
};


class Logger {
    friend class LoggerStream;

public:
    void logException(
        const string &subsystem,
        const exception &e);

    void logInfo(
        const string &subsystem,
        const string &message);

    void logSuccess(
        const string &subsystem,
        const string &message);

    void logError(
        const string &subsystem,
        const string &message);

    void logFatal(
        const string &subsystem,
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
        const string &group);

    void logRecord(
        const string &group,
        const string &subsystem,
        const string &message);

    void logRecordFile(
        const string &logFileName,
        const string &group,
        const string &subsystem,
        const string &message);
};
#endif //GEO_NETWORK_CLIENT_LOGGER_H
