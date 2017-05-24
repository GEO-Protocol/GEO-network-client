#include "Logger.h"
#include <fstream>
#include <iostream>
#include "../settings/Settings.h"


LoggerStream::LoggerStream(
    Logger *logger,
    const string &group,
    const string &subsystem,
    const StreamType type) :

    mLogger(logger),
    mGroup(group),
    mSubsystem(subsystem),
    mType(type)
{}

LoggerStream::~LoggerStream()
{
    if (mLogger == nullptr)
        return;

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
    if (message.size() > 0) {
        mLogger->logRecordFile(
            mLogfile,
            mGroup,
            mSubsystem,
            message);

#ifdef DEBUG
        mLogger->logRecord(
            mGroup,
            mSubsystem,
            message);
#endif
    }


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
    mType(other.mType)
{}


void Logger::logException(
    const string &subsystem,
    const exception &e)
{
    auto m = string(e.what());
    logRecord("EXCEPT", subsystem, m);
}

void Logger::logInfo(
    const string &subsystem,
    const string &message)
{
    logRecord("INFO", subsystem, message);
}

void Logger::logSuccess(
    const string &subsystem,
    const string &message)
{
    logRecord("SUCCESS", subsystem, message);
}

void Logger::logError(
    const string &subsystem,
    const string &message)
{
    logRecord("ERROR", subsystem, message);
}

void Logger::logFatal(
    const string &subsystem,
    const string &message)
{
    logRecord("FATAL", subsystem, message);
}

LoggerStream Logger::info(
    const string &subsystem)
{
    return LoggerStream(this, "INFO", subsystem);
}

LoggerStream Logger::error(
    const string &subsystem)
{
    return LoggerStream(this, "ERROR", subsystem);
}

LoggerStream Logger::debug(
    const string &subsystem)
{
    return LoggerStream(this, "DEBUG", subsystem);
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
    const string &message)
{
    cout << recordPrefix(group)
         << subsystem << "\t"
         << formatMessage(message) << endl;
    cout.flush();
}

void Logger::logRecordFile(
    const string &logFileName,
    const string &group,
    const string &subsystem,
    const string &message)
{
    ofstream logfile;
    logfile.open(logFileName);
    logfile << recordPrefix(group)
         << subsystem << "\t"
         << formatMessage(message) << endl;
    logfile.close();
}
