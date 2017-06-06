#ifndef GEO_NETWORK_CLIENT_LOGGER_H
#define GEO_NETWORK_CLIENT_LOGGER_H

#include "../common/exceptions/Exception.h"
#include "../common/time/TimeUtils.h"
#include "../common/NodeUUID.h"


#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <queue>

using namespace std;
namespace as = boost::asio;
using as::ip::tcp;

typedef vector<pair<string,string>> addInfoType;

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

    explicit LoggerStream(
        Logger *logger,
        const string &group,
        const string &subsystem,
        addInfoType &addinfo,
        const StreamType type = Standard);

    LoggerStream(const LoggerStream &other);

    ~LoggerStream();

    static LoggerStream dummy();

private:
    Logger *mLogger;
    const addInfoType mAddInfo;
    const string mGroup;
    const string mSubsystem;
    const StreamType mType;
    const string mLogfile = "operations.log";
};


class Logger {
    friend class LoggerStream;

public:
    Logger(
        const NodeUUID &nodeUUID,
        as::io_service &mIOService,
        const string &interface,
        const string &dbname,
        const uint16_t port);
//
//    Logger(const Logger &);

//    Logger& operator= (
//        const Logger &other)
//    noexcept;

    void logException(
        const string &subsystem,
        const exception &e);

    LoggerStream info(
        const string &subsystem);

    LoggerStream error(
        const string &subsystem);

    LoggerStream debug(
        const string &subsystem);

    LoggerStream info(
        const string &subsystem,
        addInfoType &&addinfo);

    LoggerStream error(
        const string &subsystem,
        addInfoType &&addinfo);

    LoggerStream debug(
        const string &subsystem,
        addInfoType &&addinfo);

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

protected:
    struct LogRecordINFLUX {
        time_t initTime;
        const string group;
        const string subsystem;
        const string message;
        addInfoType additionalInfo;
    };

private:
    const string formatMessage(
        const string &message) const;

    const string recordPrefix(
        const string &group);

    void logRecord(
        const string &group,
        const string &subsystem,
        const string &message,
        const addInfoType &addinfo);

    void flushRecordsQueue(const boost::system::error_code &error);

    time_t get_unixtime();

private:
    const string mInterface;
    const uint16_t mPort;
    const string mDbName;

    NodeUUID mNodeUUID;

    queue <LogRecordINFLUX> mRecordsQueue;

    as::io_service &mIOService;

    tcp::socket mSocket;
    tcp::resolver mResolver;
    tcp::resolver::query mQuery;
    tcp::resolver::iterator mEndpointIterator;

    boost::asio::streambuf mRequest;
    std::ostream mRequestStream;

    unique_ptr<as::steady_timer> mFlushRecordsQueueTimer;
    uint8_t mQuequeFlushDelaySeconds = 5;

    std::ofstream mOperationsLogFile;
};
#endif //GEO_NETWORK_CLIENT_LOGGER_H
