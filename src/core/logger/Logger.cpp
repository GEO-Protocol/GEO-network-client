#include "Logger.h"

LoggerStream::LoggerStream(
    Logger *logger,
    const char *group,
    const char *subsystem):
    mLogger(logger),
    mGroup(group),
    mSubsystem(subsystem){}

LoggerStream::LoggerStream(
    Logger* logger,
    const char* group,
    const string& subsystem):
    mLogger(logger),
    mGroup(group),
    mSubsystem(subsystem){}

LoggerStream::~LoggerStream() {
    auto message = this->str();
    if (message.size() > 0) {
        mLogger->logRecord(
            mGroup.c_str(),
            mSubsystem.c_str(),
            message);
    }
}

LoggerStream::LoggerStream(const LoggerStream &other):
    mLogger(other.mLogger),
    mGroup(other.mGroup),
    mSubsystem(other.mSubsystem){}


void Logger::logException(
    const char *subsystem,
    const exception &e) {

    auto m = string(e.what());
    logRecord("EXCEPT", subsystem, m);
}

void Logger::logInfo(
    const char *subsystem,
    const string &message) {

    logRecord("INFO", subsystem, message);
}

void Logger::logSuccess(
    const char *subsystem,
    const string &message) {

    logRecord("SUCCESS", subsystem, message);
}

void Logger::logError(
    const char *subsystem,
    const string &message){

    logRecord("ERROR", subsystem, message);
}

void Logger::logFatal(
    const char *subsystem,
    const string &message){

    logRecord("FATAL", subsystem, message);
}

LoggerStream Logger::info(
    const string &subsystem){
    return LoggerStream(this, "INFO", subsystem);
}

LoggerStream Logger::error(
    const string &subsystem) {
    return LoggerStream(this, "ERROR", subsystem);
}

LoggerStream Logger::debug(
    const string &subsystem) {
    return LoggerStream(this, "DEBUG", subsystem);
}

const string Logger::formatMessage(
    const string &message) const {
    if (message.size() == 0) {
        return message;
    }

    auto m = message;
    if (m.at(m.size()-1) == '\n') {
        m = m.substr(0, m.size()-1);
    }

    if (m.at(m.size()-1) != '.' && m.at(m.size()-1) != '\n') {
        m += ".";
    }

    return m;
}

const string Logger::recordPrefix(
    const char *group) {
    // TODO: add timestamp
    return string(group) + string("\t");
}

void Logger::logRecord(
    const char *group,
    const char *subsystem,
    const string &message) {
    cout << recordPrefix(group)
         << subsystem << "\t"
         << formatMessage(message) << endl;
    cout.flush();
}
