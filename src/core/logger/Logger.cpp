#include "Logger.h"


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

/**
 * Outputs collected log infromation.
 */
LoggerStream::~LoggerStream()
{
    if (mType == Dummy) {
        // No output must be generated.
        return;
    }

    if (mType == Transaction) {
        // If this message was received from the transaction,
        // but transactions log was disabled - ignore this.
#ifndef TRANSACTIONS_LOG
        return;
#endif
    }

    auto message = this->str();
    mLogger->logRecord(
        mGroup,
        mSubsystem,
        message);
}

/**
 * Returns logger stream, that would collect information, but would never outputs it.
 */
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


Logger::Logger(
    const NodeUUID &nodeUUID):

    mNodeUUID(nodeUUID),
    mOperationLogFileName("operations.log"),
    mOperationsLogFileLinesNumber(0)
{

    calculateOperationsLogFileLinesNumber();

#ifdef DEBUG
    // It is mush more useful to truncate log file on each application start, in debug mode.
    mOperationsLogFile.open("operations.log", std::fstream::out | std::fstream::trunc);
#endif

#ifndef DEBUG
    // In production mode, logs must be appended.
    mOperationsLogFile.open("operations.log", std::fstream::out | std::fstream::app);
#endif

}

void Logger::logException(
    const string &subsystem,
    const exception &e)
{
    auto m = string(e.what());
    logRecord("EXCEPT", subsystem, m);
}

LoggerStream Logger::info(
    const string &subsystem)
{
    return LoggerStream(this, "INFO", subsystem);
}

LoggerStream Logger::warning(
        const string &subsystem)
{
    return LoggerStream(this, "WARNING", subsystem);
}

LoggerStream Logger::debug(
    const string &subsystem)
{
    return LoggerStream(this, "DEBUG", subsystem);
}

LoggerStream Logger::error(
    const string &subsystem)
{
    return LoggerStream(this, "ERROR", subsystem);
}

const string Logger::formatMessage(
    const string &message) const
{
    if (message.empty()) {
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
    stringstream recordStream;
    recordStream << recordPrefix(group)
                 << subsystem << "\t"
                 << formatMessage(message) << endl;

    // Logging to the console
    cout << recordStream.str();

    // Logging to the file
    mOperationsLogFile << recordStream.str();
    mOperationsLogFile.flush();

    mOperationsLogFileLinesNumber++;
    if(mOperationsLogFileLinesNumber >= maxRotateLimit){
        rotate();
        mOperationsLogFileLinesNumber = 0;
    }
}

void Logger::rotate()
{
    mOperationsLogFile.close();

    stringstream rotateFileName;
    rotateFileName << "archieved_operation_" << utc_now() << ".log";

    std::string tmp = rotateFileName.str();
    formatLogFileName(tmp, " ", "_");
    const char* newname = tmp.c_str();
    rename("operations.log", newname);

    mOperationsLogFile.open("operations.log", std::fstream::out | std::fstream::trunc);
}

void Logger::calculateOperationsLogFileLinesNumber()
{
    ifstream kOperationLogFile(mOperationLogFileName);
    std::string line;
    while (std::getline(kOperationLogFile , line)){
        ++mOperationsLogFileLinesNumber;
    }
    if (kOperationLogFile.is_open()){
        kOperationLogFile.close();
    }
}

bool Logger::formatLogFileName(
    string& str,
    const string& from,
    const string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
