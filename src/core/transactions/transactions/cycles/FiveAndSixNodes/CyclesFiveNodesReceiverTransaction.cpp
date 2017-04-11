#include "CyclesFiveNodesReceiverTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"

CyclesFiveNodesReceiverTransaction::CyclesFiveNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFiveNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FiveNodesReceiverTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
    mInBetweenNodeTopologyMessage(message)
{}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesFiveNodesReceiverTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    stringstream ss;
    copy(path.begin(), path.end(), ostream_iterator<NodeUUID>(ss, "    "));
    cout << "CyclesFiveNodesReceiverTransaction::run() Path   " << ss.str() << endl;
    uint8_t currentDepth = path.size();
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance maxFlow = (-1) * mTrustLinesManager->balance(path.back());
    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
    stringstream ss1;
    copy(firstLevelNodes.begin(), firstLevelNodes.end(), ostream_iterator<NodeUUID>(ss1, "    "));
    cout << "CyclesFiveNodesReceiverTransaction::run() firstLevelNodes: " << ss1.str() << endl;
    bool creditorsBranch = true;
    if (maxFlow < zeroBalance){
        creditorsBranch = false;
        cout << "We are on Creditors branch (-) Branch" << endl;
    } else {
        cout << "We are on Debtors branch (+) Branch" << endl;
    }
    if ((creditorsBranch and currentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &kNodeUUID: firstLevelNodes)
            sendMessage(
                    kNodeUUID,
                    mInBetweenNodeTopologyMessage
            );
        return resultExit();
    }
    if ((creditorsBranch and currentDepth==2) or (not creditorsBranch and currentDepth==1)){
        path.push_back(mNodeUUID);
        sendMessage<CyclesFiveNodesBoundaryMessage>(
            path.front(),
            path,
            firstLevelNodes
        );
        return resultExit();
    }
    else {
        return resultExit();
    }

}
#pragma clang diagnostic pop

