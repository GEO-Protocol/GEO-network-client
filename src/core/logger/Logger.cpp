#include <fstream>
#include <iostream>
#include "Logger.h"
#include "../settings/Settings.h"


LoggerStream::LoggerStream(
    Logger *logger,
    const char *group,
    const char *subsystem):
    mLogger(logger),
    mGroup(group),
    mSubsystem(subsystem){}

LoggerStream::~LoggerStream() {
    auto message = this->str();
    if (message.size() > 0) {
        mLogger->logRecord(mGroup, mSubsystem, message);
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
    const char *subsystem){

    return LoggerStream(this, "INFO", subsystem);
}

LoggerStream Logger::error(
    const char *subsystem) {
    return LoggerStream(this, "ERROR", subsystem);
}

const string Logger::formatMessage(
    const string &message) const {
    if (message.size() == 0) {
        return message;
    }

    auto m = message;
    if (m.at(m.size()-1) != '.') {
        m += ".";
    }

    return m;
}

const string Logger::recordPrefix(
    const char *group) {
    // todo: add Timestamp
    return string(group) + string("\t\t");
}

void Logger::logRecord(
    const char *group,
    const char *subsystem,
    const string &message) {
    cout << recordPrefix(group)
         << subsystem << "\t\t\t"
         << formatMessage(message) << endl;
    cout.flush();
}

void Logger::logTransactionStatus(
            const NodeUUID &contractorUUID,
            const TrustLineAmount &amount
    ){
    ofstream logfile;
    logfile.open("logfile.txt");
    stringstream ss;
    ss << amount;
    string command = "ContractorUUID\t" + contractorUUID.stringUUID() + "\tAmount\t" + ss.str() + "\t";
    cout << "ContractorUUID" << '\t' << contractorUUID.stringUUID() << endl;
    logfile << command << endl;
}
