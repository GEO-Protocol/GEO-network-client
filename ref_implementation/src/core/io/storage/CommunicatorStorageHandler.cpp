/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CommunicatorStorageHandler.h"

sqlite3 *CommunicatorStorageHandler::mDBConnection = nullptr;

CommunicatorStorageHandler::CommunicatorStorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger &logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    mCommunicatorMessagesQueueHandler(connection(dataBaseName, directory), kMessagesQueueTableName, logger),
    mLog(logger)
{
    sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
}

CommunicatorStorageHandler::~CommunicatorStorageHandler()
{
    if (mDBConnection != nullptr) {
        sqlite3_close_v2(mDBConnection);
    }
}

void CommunicatorStorageHandler::checkDirectory(
    const string &directory)
{
    if (!fs::is_directory(fs::path(directory))){
        fs::create_directories(
            fs::path(directory));
    }
}

sqlite3* CommunicatorStorageHandler::connection(
    const string &dataBaseName,
    const string &directory)
{
    checkDirectory(directory);
    if (mDBConnection != nullptr)
        return mDBConnection;
    string dataBasePath = directory + "/" + dataBaseName;
    int rc = sqlite3_open_v2(dataBasePath.c_str(), &mDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("CommunicatorStorageHandler::connection "
                          "Can't open database " + dataBaseName);
    }
    return mDBConnection;
}

CommunicatorIOTransaction::Shared CommunicatorStorageHandler::beginTransaction()
{
    return make_shared<CommunicatorIOTransaction>(
        mDBConnection,
        &mCommunicatorMessagesQueueHandler,
        mLog);
}

CommunicatorIOTransaction::Unique CommunicatorStorageHandler::beginTransactionUnique()
{
    return make_unique<CommunicatorIOTransaction>(
        mDBConnection,
        &mCommunicatorMessagesQueueHandler,
        mLog);
}

LoggerStream CommunicatorStorageHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream CommunicatorStorageHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string CommunicatorStorageHandler::logHeader() const
{
    stringstream s;
    s << "[CommunicatorStorageHandler]";
    return s.str();
}
