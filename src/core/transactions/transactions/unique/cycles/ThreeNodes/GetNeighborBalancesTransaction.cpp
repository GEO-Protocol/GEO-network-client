#include "GetNeighborBalancesTransaction.h"

GetNeighborBalancesTransaction::GetNeighborBalancesTransaction(TransactionsScheduler *scheduler)
        :UniqueTransaction(BaseTransaction::TransactionType::GetNeighborBalancesTransaction, scheduler) {

}

GetNeighborBalancesTransaction::GetNeighborBalancesTransaction(
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

GetNeighborBalancesTransaction::GetNeighborBalancesTransaction(const BaseTransaction::TransactionType type,
                                                               const NodeUUID &nodeUUID,
                                                               BalancesRequestMessage::Shared message,
                                                               TransactionsScheduler *scheduler,
                                                               TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mRequestMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger)
{

}
TransactionResult::SharedConst GetNeighborBalancesTransaction::run() {
//    Check if something in context
    if (mContext.size() > 0){
        auto message = static_pointer_cast<BalancesResponseMessage>(*mContext.begin());
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
        sendMessage<BalancesRequestMessage>(
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



TransactionResult::SharedConst GetNeighborBalancesTransaction::waitingForNeighborBalances() {

    return transactionResultFromState(
            TransactionState::waitForMessageTypes(
                    {Message::MessageTypeID::ResponseMessageType},
                    mConnectionTimeout
            )
    );
}

