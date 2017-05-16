#include "UpdateRoutingTablesTransaction.h"

UpdateRoutingTablesTransaction::UpdateRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    UpdateRoutingTablesCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger)
noexcept :
    BaseTransaction(
        BaseTransaction::RoutingTables_UpdateRoutingTable,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler)
{}

UpdateRoutingTablesCommand::Shared UpdateRoutingTablesTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::run() {
    for(auto kNodeUUID: mTrustLinesManager->rt1()){
        auto transaction = make_shared<NeighborsCollectingTransaction>(
            mNodeUUID,
            kNodeUUID,
            mNodeUUID,
            0,
            mStorageHandler,
            mLog);
        launchSubsidiaryTransaction(transaction);
    }
    return transactionResultFromCommand(mCommand->responseOK());
}
