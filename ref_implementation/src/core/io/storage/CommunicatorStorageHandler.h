/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_COMMUNICATORSTORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_COMMUNICATORSTORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "CommunicatorMessagesQueueHandler.h"
#include "CommunicatorIOTransaction.h"
#include "../../common/exceptions/IOError.h"
#include "../../../libs/sqlite3/sqlite3.h"

#include <boost/filesystem.hpp>
#include <vector>

namespace fs = boost::filesystem;

class CommunicatorStorageHandler {

public:
    CommunicatorStorageHandler(
        const string &directory,
        const string &dataBaseName,
        Logger &logger);

    ~CommunicatorStorageHandler();

    CommunicatorIOTransaction::Shared beginTransaction();

    CommunicatorIOTransaction::Unique beginTransactionUnique();

private:
    static void checkDirectory(
        const string &directory);

    static sqlite3* connection(
        const string &dataBaseName,
        const string &directory);

    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    const string kMessagesQueueTableName = "communicator_messages_queue";

private:
    static sqlite3 *mDBConnection;

private:
    Logger &mLog;
    CommunicatorMessagesQueueHandler mCommunicatorMessagesQueueHandler;
    string mDirectory;
    string mDataBaseName;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATORSTORAGEHANDLER_H
