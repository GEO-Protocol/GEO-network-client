/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "RemoveNodeFromBlackListTransaction.h"

RemoveNodeFromBlackListTransaction::RemoveNodeFromBlackListTransaction(
    NodeUUID &nodeUUID,
    RemoveNodeFromBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::RemoveNodeFromBlackListTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{
}

RemoveNodeFromBlackListCommand::Shared RemoveNodeFromBlackListTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst RemoveNodeFromBlackListTransaction::run() {
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto contractorNode = mCommand->contractorUUID();
    const auto kContractorNodesBanned = ioTransaction->blackListHandler()->checkIfNodeExists(contractorNode);
    try {
        if (kContractorNodesBanned) {
            ioTransaction->blackListHandler()->removeNodeFromBlackList(contractorNode);
            return transactionResultFromCommand(mCommand->responseOK());
        } else {
            return transactionResultFromCommand(mCommand->responseTrustlineIsAbsent());
        }
    } catch (IOError &e){
        error() << e.what();
        return transactionResultFromCommand(mCommand->responseUnexpectedError());
    }
}

const string RemoveNodeFromBlackListTransaction::logHeader() const
{
    stringstream s;
    s << "[RemoveNodeFromBlackListTransaction: " << currentTransactionUUID() << "] ";
    return s.str();
}
