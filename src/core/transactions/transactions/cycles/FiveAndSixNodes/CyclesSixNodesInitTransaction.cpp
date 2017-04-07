#include "CyclesSixNodesInitTransaction.h"

const BaseTransaction::TransactionType CyclesSixNodesInitTransaction::transactionType() const{
    return BaseTransaction::TransactionType::Cycles_SixNodesInitTransaction;
}

TransactionResult::SharedConst CyclesSixNodesInitTransaction::runCollectDataAndSendMessagesStage() {
    const auto firstLevelNodes = mTrustLinesManager->firstLevelNeighborsWithNoneZeroBalance();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    for(const auto &kNodeUUID: firstLevelNodes){
        sendMessage<CyclesSixNodesInBetweenMessage>(
            kNodeUUID,
            path
        );
    }
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwaikAfterMilliseconds(mkWaitingForResponseTime);
}

CyclesSixNodesInitTransaction::CyclesSixNodesInitTransaction(
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    Logger *logger) :
    CyclesBaseFiveSixNodesInitTransaction(
        BaseTransaction::TransactionType::Cycles_SixNodesInitTransaction,
        nodeUUID,
        manager,
        logger)
{}


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesSixNodesInitTransaction::runParseMessageAndCreateCyclesStage() {

    const TrustLineBalance kZeroBalance = 0;
    CycleMap mCreditors;
    TrustLineBalance creditorsStepFlow;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<CyclesSixNodesBoundaryMessage>(mess);
        const auto stepPath = make_shared<vector<NodeUUID>>(message->Path());
//  It has to be exactly nodes count in path
        if (stepPath->size() != 3)
            continue;
        creditorsStepFlow = mTrustLinesManager->balance((*stepPath)[1]);
//  If it is Debtor branch - skip it
        if (creditorsStepFlow > kZeroBalance)
            continue;
//  Check all Boundary Nodes and add it to map if all checks path
        for (auto &nodeUUID: message->BoundaryNodes()){
//  Prevent loop on cycles path
            if (nodeUUID == stepPath->front())
                continue;
            mCreditors.insert(make_pair(
                nodeUUID,
                stepPath));
        }
    }

//    Create Cycles comparing BoundaryMessages data with debtors map
    TrustLineBalance debtorsStepFlow;
    TrustLineBalance commonStepMaxFlow;
    vector<NodeUUID> stepPath;
    for(const auto &mess: mContext) {
        auto message = static_pointer_cast<CyclesSixNodesBoundaryMessage>(mess);
        debtorsStepFlow = mTrustLinesManager->balance(message->Path()[1]);
        //  If it is Creditors branch - skip it
        if (debtorsStepFlow < kZeroBalance)
            continue;
        stepPath = message->Path();
        //  It has to be exactly nodes count in path
        if (stepPath.size() != 3)
            continue;
        for (const auto &kNodeUUID: message->BoundaryNodes()) {
            //  Prevent loop on cycles path
            if (kNodeUUID == stepPath.front())
                continue;
            auto NodeUIIDAndPathRange = mCreditors.equal_range(kNodeUUID);
            for (auto NodeUIDandPairOfPathandBalace = NodeUIIDAndPathRange.first;
                 NodeUIDandPairOfPathandBalace != NodeUIIDAndPathRange.second; ++NodeUIDandPairOfPathandBalace) {
                if (((*NodeUIDandPairOfPathandBalace->second)[2] == stepPath[1]) or ((*NodeUIDandPairOfPathandBalace->second)[1] == stepPath[2]))
                    continue;
                //  Find minMax flow between 3 value. 1 in map. 1 in boundaryNodes. 1 we get from creditor first node in path
                vector<NodeUUID> stepCyclePath = {stepPath[0],
                                                  stepPath[1],
                                                  stepPath[2],
                                                  kNodeUUID,
                                                  (*(NodeUIDandPairOfPathandBalace->second))[2],
                                                  (*(NodeUIDandPairOfPathandBalace->second))[1]};
                //    Todo run cycles
                // Потрібно ставити первірку на доречність перекриття цилів
                // Ця транакція має верта нам дані про те через який трастлайн неможна зробити перрозрахунок
                stepCyclePath.clear();
            }
        }
    }
    mContext.clear();

    return finishTransaction();
}
