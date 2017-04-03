#include "CycleThreeNodesInitTransaction.h"

CycleThreeNodesInitTransaction::CycleThreeNodesInitTransaction(TransactionsScheduler *scheduler)
        :UniqueTransaction(BaseTransaction::TransactionType::CyclesThreeNodesInitTransaction, scheduler) {

}

CycleThreeNodesInitTransaction::CycleThreeNodesInitTransaction(
        const BaseTransaction::TransactionType type,
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mStorageHandler(storageHandler),
          mTrustLinesManager(manager),
          mlogger(logger),
          mContractorUUID(contractorUUID)
{

}

set<NodeUUID> CycleThreeNodesInitTransaction::getNeighborsWithContractor() {
    TrustLineBalance balanceToContractor = mTrustLinesManager->balance(mContractorUUID);
    TrustLineBalance zeroBalance = 0;
    bool balancePositive = true;
    if (balanceToContractor < zeroBalance){
        balancePositive = false;
    }
    set<NodeUUID> contractorNeighbors = mStorageHandler->routingTablesHandler()->routingTable2Level()->allDestinationsForSource(
            mContractorUUID);
    set<NodeUUID> ownNeighbors;
    set<NodeUUID> commonNeighbors;
    for (auto &value: mTrustLinesManager->trustLines()){
        if (balancePositive){
            if (value.second->balance() < zeroBalance){
                ownNeighbors.insert(value.first);
            }
        } else {
            if (value.second->balance() > zeroBalance){
                ownNeighbors.insert(value.first);
            }
        }
        set_intersection(ownNeighbors.begin(), ownNeighbors.end(),
                         ownNeighbors.begin(), ownNeighbors.end(),
                         std::inserter(commonNeighbors, commonNeighbors.begin()));
    }
    return commonNeighbors;
}

TransactionResult::SharedConst CycleThreeNodesInitTransaction::run() {
    switch (mStep){
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessageStage();
        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();
        default:
            throw RuntimeError(
                    "CycleThreeNodesInitTransaction::run(): "
                            "invalid transaction step.");

    }
}

TransactionResult::SharedConst CycleThreeNodesInitTransaction::runCollectDataAndSendMessageStage() {

    set<NodeUUID> neighbors = getNeighborsWithContractor();
    sendMessage<ThreeNodesBalancesRequestMessage>(
            mContractorUUID,
            mNodeUUID,
            mTransactionUUID,
            neighbors
    );
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
            {Message::Payments_ReceiverInitPaymentResponse},
            kStandardConnectionTimeout);
}

TransactionResult::SharedConst CycleThreeNodesInitTransaction::runParseMessageAndCreateCyclesStage() {
    auto message = static_pointer_cast<ThreeNodesBalancesResponseMessage>(*mContext.begin());
    vector <pair<NodeUUID, TrustLineBalance>> neighborsAndBalances = message->NeighborsAndBalances();
    for(auto &value:neighborsAndBalances ){
        vector<NodeUUID> cycle;
        cycle.push_back(mNodeUUID);
        cycle.push_back(mContractorUUID);
        cycle.push_back(value.first);
//            todo run transaction to close cycle
    }
    return finishTransaction();
}
