/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "GetBlackListTransaction.h"

GetBlackListTransaction::GetBlackListTransaction(
    NodeUUID &nodeUUID,
    GetBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger)
noexcept :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

GetBlackListCommand::Shared GetBlackListTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetBlackListTransaction::run() {
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto kBannedUsers = ioTransaction->blackListHandler()->allNodesUUIDS();
    stringstream ss;
    ss << to_string(kBannedUsers.size());

    for (const auto kNodeUUID: kBannedUsers) {
        ss << kTokensSeparator;
        ss << kNodeUUID;
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(mCommand->resultOk(kResultInfo));
}

const string GetBlackListTransaction::logHeader() const
{
    stringstream s;
    s << "[GetBlackListTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
