#include "RemoveNodeFromBlackListTransaction.h"

RemoveNodeFromBlackListTransaction::RemoveNodeFromBlackListTransaction(
    NodeUUID &nodeUUID,
    RemoveNodeFromBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::RemoveNodeFromBlackListTransactionType,
        nodeUUID,
        0,      // none equivalent
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
            return transactionResultFromCommand(mCommand->responseTrustLineIsAbsent());
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
