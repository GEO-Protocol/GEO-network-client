#include "GetThreeNodesNeighborBalancesTransaction.h"

GetThreeNodesNeighborBalancesTransaction::GetThreeNodesNeighborBalancesTransaction(TransactionsScheduler *scheduler)
        :UniqueTransaction(BaseTransaction::TransactionType::GetThreeNodesNeighborBalancesTransaction, scheduler) {

}

GetThreeNodesNeighborBalancesTransaction::GetThreeNodesNeighborBalancesTransaction(
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

GetThreeNodesNeighborBalancesTransaction::GetThreeNodesNeighborBalancesTransaction(const BaseTransaction::TransactionType type,
                                                               const NodeUUID &nodeUUID,
                                                               const NodeUUID &contractorUUID,
                                                               ThreeNodesBalancesRequestMessage::Shared message,
                                                               TransactionsScheduler *scheduler,
                                                               TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mContractorUUID(contractorUUID),
          mRequestMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger)
{

}
TransactionResult::SharedConst GetThreeNodesNeighborBalancesTransaction::run() {
//    Check if something in context
    if (mContext.size() > 0){
//        There is ResponseMessage in Context. Get data from it and create cycles
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
//   Nothing in context; No answer from neighbor
    } else if(mRequestMessage == nullptr){
//  No response Messages. No Request Messages.
//  Create RequestMessage with neighbors node uuids and send it contractor
    TrustLineBalance maxFlow = mTrustLinesManager->balance(mContractorUUID);
    set<NodeUUID> neighbors = getNeighborsWithContractor();

        sendMessage<ThreeNodesBalancesRequestMessage>(
                mContractorUUID,
                mNodeUUID,
                mTransactionUUID,
                neighbors
        );
        return waitingForNeighborBalances();
    } else if(mRequestMessage != nullptr){
        set<NodeUUID> neighbors = mRequestMessage->Neighbors();
        vector<pair<NodeUUID, TrustLineBalance>> neighborsAndBalances;
        for(auto &value: neighbors){
            neighborsAndBalances.push_back(
                    make_pair(
                            value,
                            mTrustLinesManager->balance(mContractorUUID)
            ));
        }
        sendMessage<ThreeNodesBalancesResponseMessage>(
                mContractorUUID,
                mNodeUUID,
                mTransactionUUID,
                neighborsAndBalances
        );
        return finishTransaction();
    }
    return finishTransaction();
}



TransactionResult::SharedConst GetThreeNodesNeighborBalancesTransaction::waitingForNeighborBalances() {

    return transactionResultFromState(
            TransactionState::waitForMessageTypes(
                    {Message::MessageTypeID::ResponseMessageType},
                    mConnectionTimeout
            )
    );
}

set<NodeUUID> GetThreeNodesNeighborBalancesTransaction::getNeighborsWithContractor() {
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
