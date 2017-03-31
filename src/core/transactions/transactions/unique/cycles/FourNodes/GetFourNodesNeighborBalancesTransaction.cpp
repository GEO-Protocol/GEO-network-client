#include "GetFourNodesNeighborBalancesTransaction.h"


GetFourNodesNeighborBalancesTransaction::GetFourNodesNeighborBalancesTransaction(TransactionsScheduler *scheduler)
        :UniqueTransaction(BaseTransaction::TransactionType::GetThreeNodesNeighborBalancesTransaction, scheduler) {

}

GetFourNodesNeighborBalancesTransaction::GetFourNodesNeighborBalancesTransaction(
        const BaseTransaction::TransactionType type,
        const NodeUUID &nodeUUID,
        const NodeUUID &debtorContractorUUID,
        const NodeUUID &creditorContractorUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mTrustLinesManager(manager),
          mlogger(logger),
          mDebtorContractorUUID(debtorContractorUUID),
          mCreditorContractorUUID(creditorContractorUUID)
{

}

GetFourNodesNeighborBalancesTransaction::GetFourNodesNeighborBalancesTransaction(const BaseTransaction::TransactionType type,
                                                               const NodeUUID &nodeUUID,
                                                               FourNodesBalancesRequestMessage::Shared message,
                                                               TransactionsScheduler *scheduler,
                                                               TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mRequestMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger)
{

}
TransactionResult::SharedConst GetFourNodesNeighborBalancesTransaction::run() {
//  Check if something in context
    if (mContext.size() > 0){
//      There are some FourNodesBalancesResponseMessage in Context. Try to create cycles
        if (mContext.size() != 2){

            mlogger->error("GetFourNodesNeighborBalancesTransaction:"
                           "Responses Messages count Not equal 2;"
                           "Can not create Cycles;");
            return finishTransaction();
        }
        auto first_message = static_pointer_cast<FourNodesBalancesResponseMessage>(*mContext.begin());
        auto second_message = static_pointer_cast<FourNodesBalancesResponseMessage>(*mContext.end());

//        vector <pair<NodeUUID, TrustLineBalance>> neighborsAndBalancesCreditors = message->NeighborsBalancesCreditors();
//        vector <pair<NodeUUID, TrustLineBalance>> neighborsAndBalancesDebtors = message->NeighborsBalancesDebtors();
//        for(auto &creditor:neighborsAndBalancesCreditors ){
//            for(auto &debtor:neighborsAndBalancesCreditors ){
//
//                vector<NodeUUID> cycle;
//                cycle.push_back(mNodeUUID);
//                cycle.push_back(creditor.first);
//                cycle.push_back(mContractorUUID);
//                cycle.push_back(debtor.first);
//
////            todo run transaction to close cycle
//            }
//        }
        return finishTransaction();
//   Nothing in context; No answer from neighbor
    } else if(mRequestMessage == nullptr){
//    TrustLineBalance maxFlow = mTrustLinesManager->balance(mContractorUUID);
//    vector<NodeUUID> neighborsDebtors;
//    vector<NodeUUID> neighborsCreditors;
//        sendMessage<FourNodesBalancesRequestMessage>(
//                mContractorUUID,
//                maxFlow,
//                neighborsDebtors,
//                neighborsCreditors
//        );
        return waitingForNeighborBalances();
    } else if(mRequestMessage != nullptr){
//        TrustLineBalance maxFlow = mRequestMessage->MaxFlow();
//        vector<NodeUUID> neighborsDebtors = mRequestMessage->NeighborsDebtor();
//        vector<NodeUUID> neighborsCreditors = mRequestMessage->NeighborsCreditor();
//        vector<pair<NodeUUID, TrustLineBalance>> neighborsAndBalancesDebtors;
//        vector<pair<NodeUUID, TrustLineBalance>> neighborsAndBalancesCreditors;
//        for(auto &value: neighborsDebtors){
//            neighborsAndBalancesDebtors.push_back(
//                    make_pair(
//                            value,
//                            min(mTrustLinesManager->balance(mContractorUUID), maxFlow)
//            ));
//        }
//        for(auto &value: neighborsCreditors){
//            neighborsAndBalancesCreditors.push_back(
//                    make_pair(
//                            value,
//                            min(mTrustLinesManager->balance(mContractorUUID), maxFlow)
//                    ));
//        }
//        sendMessage<FourNodesBalancesResponseMessage>(
//                mContractorUUID,
//                neighborsAndBalancesDebtors,
//                neighborsAndBalancesCreditors
//        );
        return finishTransaction();
    }
    return finishTransaction();
}



TransactionResult::SharedConst GetFourNodesNeighborBalancesTransaction::waitingForNeighborBalances() {

    return transactionResultFromState(
            TransactionState::waitForMessageTypes(
                    {Message::MessageTypeID::ResponseMessageType},
                    mConnectionTimeout
            )
    );
}

