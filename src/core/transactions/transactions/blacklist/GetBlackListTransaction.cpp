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
        0,      // none equivalent
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
