/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CheckIfNodeInBlackListTransaction.h"

CheckIfNodeInBlackListTransaction::CheckIfNodeInBlackListTransaction(
    NodeUUID &nodeUUID,
    CheckIfNodeInBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::CheckIfNodeInBlackListTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{
}

CheckIfNodeInBlackListCommand::Shared CheckIfNodeInBlackListTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst CheckIfNodeInBlackListTransaction::run() {
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto contractorNode = mCommand->contractorUUID();
    const auto kContractorNodesBanned = ioTransaction->blackListHandler()->checkIfNodeExists(contractorNode);
    if (kContractorNodesBanned){
        return transactionResultFromCommand(mCommand->responseOK());
    } else {
        return transactionResultFromCommand(mCommand->responseTrustlineIsAbsent());
    }
}

const string CheckIfNodeInBlackListTransaction::logHeader() const
{
    stringstream s;
    s << "[CheckIfNodeInBlackListTA: " << currentTransactionUUID() << "] ";
    return s.str();
}