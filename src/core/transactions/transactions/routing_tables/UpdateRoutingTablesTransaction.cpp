#include "UpdateRoutingTablesTransaction.h"

UpdateRoutingTablesTransaction::UpdateRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    UpdateRoutingTablesCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
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

    switch (mStep) {
        case sendCRC32Rt2SRequestMessage:
            return checkCRC32rt2Sum();
        case UpdateRT2:
            return updateRoughtingTables();
        default:
            throw ValueError("UpdateRoutingTablesTransaction::run: "
                                 "unexpected Trust line state");
    }

}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::checkCRC32rt2Sum() {
    auto message = make_shared<CRC32Rt2RequestMessage>(mNodeUUID);
    for(const auto kNodeUUID: mTrustLinesManager->rt1())
        sendMessage(
            kNodeUUID,
            message);

    return resultWaitForMessageTypes(
        {Message::RoutingTables_CRC32Rt2ResponseMessage},
        mkStandardConnectionTimeout);
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::updateRoughtingTables(){

    set<NodeUUID> neighborsForUpdate;
    for(const auto kNodeUUID: mTrustLinesManager->rt1())
        neighborsForUpdate.insert(kNodeUUID);
    if(mContext.size()>0) {
        auto stepMessage = popNextMessage<CRC32Rt2ResponseMessage>();
        auto stepCRC32rt1 = mTrustLinesManager->crc32SumSecondLevel(stepMessage->senderUUID);
        if (stepCRC32rt1 == stepMessage->crc32Rt2Sum())
            neighborsForUpdate.erase(stepMessage->senderUUID);
    }
    for(auto kNodeUUID: neighborsForUpdate){
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
