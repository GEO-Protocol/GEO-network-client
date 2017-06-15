#include "Logger.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/date_time/posix_time/ptime.hpp>

LoggerStream::LoggerStream(
    Logger *logger,
    const string &group,
    const string &subsystem,
    const StreamType type) :

    mLogger(logger),
    mGroup(group),
    mSubsystem(subsystem),
    mType(type),
    mAddInfo({})
{
}

LoggerStream::LoggerStream(
    Logger *logger,
    const string &group,
    const string &subsystem,
    addInfoType &addinfo,
    const StreamType type) :

    mLogger(logger),
    mGroup(group),
    mSubsystem(subsystem),
    mAddInfo(addinfo),
    mType(type)
{}

LoggerStream::~LoggerStream()
{
//    if (mLogger == nullptr)
//        return;

    if (mType == Dummy)
        return;

    if (mType == Transaction) {
        // if this message was received from the transaction,
        // but transactions log was disabled -
        // ignore this.
#ifndef TRANSACTIONS_LOG
        return;
#endif
    }
    auto message = this->str();
    mLogger->logRecord(
        mGroup,
        mSubsystem,
        message,
        mAddInfo
    );
}

LoggerStream LoggerStream::dummy()
{
    return LoggerStream(nullptr, "", "", Dummy);
}

LoggerStream::LoggerStream(
    const LoggerStream &other) :

    mLogger(other.mLogger),
    mGroup(other.mGroup),
    mSubsystem(other.mSubsystem),
    mAddInfo(other.mAddInfo),
    mType(other.mType)
{}


void Logger::logException(
    const string &subsystem,
    const exception &e)
{
    auto m = string(e.what());
    logRecord("EXCEPT", subsystem, m, addInfoType());
}

LoggerStream Logger::info(
    const string &subsystem)
{
    return LoggerStream(this, "INFO", subsystem);
}

LoggerStream Logger::info(
    const string &subsystem,
    addInfoType &&addinfo)
{
    return LoggerStream(this, "INFO", subsystem, addinfo);
}

LoggerStream Logger::error(
    const string &subsystem)
{
    return LoggerStream(this, "ERROR", subsystem);
}

LoggerStream Logger::error(
    const string &subsystem,
    addInfoType &&addinfo)
{
    return LoggerStream(this, "ERROR", subsystem, addinfo);
}

LoggerStream Logger::debug(
    const string &subsystem)
{
    return LoggerStream(this, "DEBUG", subsystem);
}

LoggerStream Logger::debug(
    const string &subsystem,
    addInfoType &&addinfo)
{
    return LoggerStream(this, "DEBUG", subsystem, addinfo);
}

void Logger::logInfo(
    const string &subsystem,
    const string &message)
{
    logRecord("INFO", subsystem, message, addInfoType());
}

void Logger::logSuccess(
    const string &subsystem,
    const string &message)
{
    logRecord("SUCCESS", subsystem, message, addInfoType());
}

void Logger::logError(
    const string &subsystem,
    const string &message)
{
    logRecord("ERROR", subsystem, message, addInfoType());
}

void Logger::logFatal(
    const string &subsystem,
    const string &message)
{
    logRecord("FATAL", subsystem, message, addInfoType());
}

const string Logger::formatMessage(
    const string &message) const
{
    if (message.size() == 0) {
        return message;
    }

    auto m = message;
    if (m.at(m.size()-1) == '\n') {
        m = m.substr(0, m.size()-1);
    }

    if (m.at(m.size()-1) != '.' && m.at(m.size()-1) != '\n' && m.at(m.size()-1) != ':') {
        m += ".";
    }

    return m;
}

const string Logger::recordPrefix(
    const string &group)
{
    stringstream s;
    s << utc_now() << " : " << group << "\t";
    return s.str();
}


void Logger::logRecord(
    const string &group,
    const string &subsystem,
    const string &message,
    const addInfoType &addinfo)
{
    cout << recordPrefix(group)
         << subsystem << "\t"
         << formatMessage(message) << endl;
    cout.flush();

    // Log to file
    mOperationsLogFile << recordPrefix(group)
                       << subsystem << "\t"
                       << formatMessage(message) << endl;
    mOperationsLogFile.sync_with_stdio();

//    // Log to InfluxDB
//    mRecordsQueue.push({
//        get_unixtime(),
//        group,
//        subsystem,
//        formatMessage(message),
//        addinfo
//   });
}

Logger::Logger(
    const NodeUUID &nodeUUID,
    as::io_service &IOService,
    const string &interface,
    const string &dbname,
    const uint16_t port):

    mNodeUUID(nodeUUID),
    mIOService(IOService),
    mResolver(mIOService),
    mInterface(interface),
    mDbName(dbname),
    mPort(port),
    mSocket(mIOService),
    mQuery(mInterface, boost::lexical_cast<string>(mPort)),
    mRequestStream(&mRequest)
{
    mEndpointIterator = mResolver.resolve(mQuery);
    mFlushRecordsQueueTimer = make_unique<as::steady_timer>(mIOService);
    mFlushRecordsQueueTimer->expires_from_now(std::chrono::seconds(5));
    mFlushRecordsQueueTimer->async_wait(
        boost::bind(
            &Logger::flushRecordsQueue,
            this,
            as::placeholders::error));
    // For more comfortable debug use trunc instead of app.(Clear log file after restart)
    mOperationsLogFile.open("operations.log", std::fstream::out | std::fstream::app);
}

void Logger::flushRecordsQueue(
    const boost::system::error_code &error)
{
    stringstream url;
    url << "/write?db=";
    url << boost::lexical_cast<string>(mDbName);

    stringstream host;
    host << mInterface << ":" << mPort;

    stringstream body;
    auto mRecordsQueueLenght = mRecordsQueue.size();
    for(size_t i=0; i < mRecordsQueueLenght; ++i){
        auto record = mRecordsQueue.front();
        mRecordsQueue.pop();

        body << "nodes_log,";

        // Tags
        body << "node_uuid=\"";
        body<< boost::lexical_cast<string>(mNodeUUID) << "\",";

        body << "group=\"";
        body << record.group << "\" ";

        // Values
        body << "subsystem=\"";
        body << record.subsystem  << "\",";

        body << "message=\"";
        body << record.message << "\"";
        for (auto keyAndValue: record.additionalInfo){
            body << ",";
            body << keyAndValue.first << "=\"";
            body << keyAndValue.second << "\"";
        }
        body << " " << record.initTime;

        body << "\n";
    }
    mRequest.consume(mRequest.size());
    mRequestStream << "POST " << url.str() << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host.str() << "\r\n";
    mRequestStream << "Accept: */*\r\n";
    mRequestStream << "Content-Length: " << body.str().length() << "\r\n";
    mRequestStream << "content-type: application/x-www-form-urlencoded\r\n";
    mRequestStream << "Connection: close\r\n\r\n";
    mRequestStream << body.str();
    as::connect(mSocket, mEndpointIterator);
    as::write(mSocket, mRequest);


    mFlushRecordsQueueTimer->cancel();
    mFlushRecordsQueueTimer->expires_from_now(std::chrono::seconds(mQuequeFlushDelaySeconds));
    mFlushRecordsQueueTimer->async_wait(boost::bind(
        &Logger::flushRecordsQueue,
        this,
        as::placeholders::error
    ));
}

time_t Logger::get_unixtime(){
    Duration dur = pt::microsec_clock::universal_time() - pt::ptime(gt::date(1970,1,1));;
    return dur.total_microseconds();
}
