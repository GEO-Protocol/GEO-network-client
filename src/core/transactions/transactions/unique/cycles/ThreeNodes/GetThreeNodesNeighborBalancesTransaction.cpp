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
        Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mTrustLinesManager(manager),
          mlogger(logger),
          mContractorUUID(contractorUUID)
{

}

GetThreeNodesNeighborBalancesTransaction::GetThreeNodesNeighborBalancesTransaction(const BaseTransaction::TransactionType type,
                                                               const NodeUUID &nodeUUID,
                                                               ThreeNodesBalancesRequestMessage::Shared message,
                                                               TransactionsScheduler *scheduler,
                                                               TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mRequestMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger)
{

}
TransactionResult::SharedConst GetThreeNodesNeighborBalancesTransaction::run() {
//    Check if something in context
    if (mContext.size() > 0){
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
    TrustLineBalance maxFlow = mTrustLinesManager->balance(mContractorUUID);
    vector<NodeUUID> neighbors;
        sendMessage<ThreeNodesBalancesRequestMessage>(
                mContractorUUID,
                maxFlow,
                neighbors
        );
        return waitingForNeighborBalances();
    } else if(mRequestMessage != nullptr){
        vector<NodeUUID> neighbors = mRequestMessage->Neighbors();
        vector<pair<NodeUUID, TrustLineBalance>> neighborsAndBalances;
        for(auto &value: neighbors){
            neighborsAndBalances.push_back(
                    make_pair(
                            value,
                            mTrustLinesManager->balance(mContractorUUID)
            ));
        }
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

