#include "GetTopologyAndBalancesTransaction.h"

//GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(BaseTransaction::TransactionType type,
//                                                                     NodeUUID &nodeUUID,
//                                                                     TransactionsScheduler *scheduler,
//                                                                     TrustLinesManager *manager,
//                                                                     Logger *logger
//)
//        : UniqueTransaction(type, nodeUUID, scheduler),
//          mTrustLinesManager(manager),
//          mNodeUUID(nodeUUID),
//          mlogger(logger)
//{
//
//};

//GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler)
//        : UniqueTransaction(scheduler) {
//    cout << "Constructor scheduler" << endl;
//}

//TransactionResult::SharedConst GetTopologyAndBalancesTransaction::run() {
//
////    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles();
////    vector<NodeUUID> path;
////    path.push_back(mNodeUUID);
////    for(const auto &value: firstLevelNodes){
////
////        addMessage(
////                Message::Shared(new GetTopologyAndBalancesMessageInBetweenNode(
////                        value.second,
////                        mMax_depth,
////                        path
////                )),
////                value.first
////        );
////    }
//    return TransactionResult::SharedConst();
//}
pair<BytesShared, size_t> GetTopologyAndBalancesTransaction::serializeToBytes() const {
    throw ValueError("Not implemented");
}

TransactionResult::SharedConst GetTopologyAndBalancesTransaction::run() {
    throw ValueError("Not implemented");
//    return TransactionResult::SharedConst();
}

void GetTopologyAndBalancesTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(BaseTransaction::TransactionType type,
                                                                     NodeUUID &nodeUUID,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager,
                                                                     Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler) {
    cout << "We create Trasnsaction" << endl;
}

GetTopologyAndBalancesTransaction::~GetTopologyAndBalancesTransaction() {

}
