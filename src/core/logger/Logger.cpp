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

}

Logger::Logger(const NodeUUID &nodeUUID)
{
    // For more comfortable debug use trunc instead of app.(Clear log file after restart)
    mOperationsLogFile.open("operations.log", std::fstream::out | std::fstream::app);
}
