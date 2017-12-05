#include "AddNodeToBlackListTransaction.h"

AddNodeToBlackListTransaction::AddNodeToBlackListTransaction(
    NodeUUID &nodeUUID,
    AddNodeToBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::AddNodeToBlackListTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

AddNodeToBlackListCommand::Shared AddNodeToBlackListTransaction::command() const
{
    return AddNodeToBlackListCommand::Shared();
}

TransactionResult::SharedConst AddNodeToBlackListTransaction::run() {
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto contractorNode = mCommand->contractorUUID();
    ioTransaction->blackListHandler()->addNode(contractorNode);
    return transactionResultFromCommand(mCommand->responseOK());
}

const string AddNodeToBlackListTransaction::logHeader() const
{
    stringstream s;
    s << "[AddNodeToBlackListTA: " << currentTransactionUUID() << "] ";
    return s.str();
}