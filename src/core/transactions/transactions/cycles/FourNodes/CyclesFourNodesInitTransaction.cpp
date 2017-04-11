#include "CyclesFourNodesInitTransaction.h"

CyclesFourNodesInitTransaction::CyclesFourNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &debtorContractorUUID,
    const NodeUUID &creditorContractorUUID,
    TrustLinesManager *manager,
    RoutingTablesHandler *routingTablesHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesInitTransaction,
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mLogger(logger),
    mRoutingTablesHandler(routingTablesHandler),
    mDebtorContractorUUID(debtorContractorUUID),
    mCreditorContractorUUID(creditorContractorUUID)
{}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::run() {
    switch (mStep) {
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessageStage();

        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();

        default:
            throw RuntimeError(
                "CycleThreeNodesInitTransaction::run(): "
                "Invalid transaction step.");
    }
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runCollectDataAndSendMessageStage() {

    set<NodeUUID> neighbors = commonNeighborsForDebtorAndCreditorNodes();
    cout << "\n\n\n\n" << endl;
    cout << "runCollectDataAndSendMessageStage Transaction UUID: " << UUID() << endl;
    cout << "runCollectDataAndSendMessageStage Debtor UUID:  " << mDebtorContractorUUID << endl;
    cout << "runCollectDataAndSendMessageStage Credior UUID:   " << mCreditorContractorUUID << endl;
    stringstream ss;
    copy(neighbors.begin(), neighbors.end(), ostream_iterator<NodeUUID>(ss, "\n"));
    cout << "runCollectDataAndSendMessageStage Neighbors() : \n" << ss.str() << endl;
    if (neighbors.size() == 0){
        cout << "No common neighbors. Exit Transaction" << endl;
        return resultExit();
    }
    sendMessage<CyclesFourNodesBalancesRequestMessage>(
        mDebtorContractorUUID,
        mNodeUUID,
        UUID(),
        neighbors);

    sendMessage<CyclesFourNodesBalancesRequestMessage>(
        mCreditorContractorUUID,
        mNodeUUID,
        UUID(),
        neighbors);

    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwaikAfterMilliseconds(
            mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage() {
    if (mContext.size() != 2) {
        cout << "CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage: "
                   "Responses messages count not equals to 2; "
                   "Can't create cycles;";

        return resultExit();
    }
    cout << "\n\n\n\n\n" << endl;
    cout << "CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage()" << endl;
    const auto firstMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*mContext.begin());
    const auto secondMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*(mContext.end()-1));
    const auto firstContractorUUID = firstMessage->senderUUID();
    const auto secondContractorUUID = secondMessage->senderUUID();
    const TrustLineBalance zeroBalance = 0;
    cout << "FirstMessage Sender UUID " << firstContractorUUID << endl;
    stringstream ns1;
    auto path1 = firstMessage->NeighborsUUID();
    copy(path1.begin(), path1.end(), ostream_iterator<NodeUUID>(ns1, "]["));
    cout << "FirstMessage Neighbors " << ns1.str() << endl;
    cout << "SecondMessage Sender UUID " << secondContractorUUID << endl;
    auto path2 = secondMessage->NeighborsUUID();
    stringstream ns2;
    copy(path2.begin(), path2.end(), ostream_iterator<NodeUUID>(ns2, "]["));
    cout << "SecondMessage Neighbors " << ns2.str() << endl;

    TrustLineBalance firstContractorBalance = mTrustLinesManager->balance(firstContractorUUID);
    TrustLineBalance secondContractorBalance = mTrustLinesManager->balance(secondContractorUUID);

    // In case if some payment operation was done and balances on the nodes was changed -
    // this check prevents redundant cycles closing operations.
    if ((firstContractorBalance > zeroBalance and secondContractorBalance > zeroBalance) or
        (firstContractorBalance < zeroBalance and secondContractorBalance < zeroBalance) or
        (firstContractorBalance == zeroBalance and secondContractorBalance == zeroBalance)) {

        cout << "CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage: "
                  "Balances between initiator node and (" << firstContractorUUID <<  "), or "
                  "between initiator node and (" << secondContractorUUID << ") was changed. "
                  "Cannot create cycles.";
        return resultExit();
    }

    for (auto &kNodeUUIDFirstMessage: firstMessage->NeighborsUUID()){
        for (auto &kNodeUUIDSecondMessage: secondMessage->NeighborsUUID()){
            if (kNodeUUIDSecondMessage == kNodeUUIDFirstMessage){
                vector<NodeUUID> stepPath = {
                    mNodeUUID,
                    mDebtorContractorUUID,
                    kNodeUUIDSecondMessage,
                    mCreditorContractorUUID};
                // Run transaction to close cycle
                stringstream ss;
                copy(stepPath.begin(), stepPath.end(), ostream_iterator<NodeUUID>(ss, "\n"));
                cout << "CycleFound : \n" << ss.str() << endl;
            }
        }
    }
    return resultExit();
}

set<NodeUUID> CyclesFourNodesInitTransaction::commonNeighborsForDebtorAndCreditorNodes() {
    const auto creditorsNeighbors = mRoutingTablesHandler->routingTable2Level()->allDestinationsForSource(
        mCreditorContractorUUID);
    stringstream ss;
    copy(creditorsNeighbors.begin(), creditorsNeighbors.end(), ostream_iterator<NodeUUID>(ss, "]["));
    cout << "commonNeighborsForDebtorAndCreditorNodes creditorsNeighbor : " << ss.str() << endl;
    const auto debtorsNeighbors = mRoutingTablesHandler->routingTable2Level()->allDestinationsForSource(
        mDebtorContractorUUID);
    stringstream s2;
    copy(debtorsNeighbors.begin(), debtorsNeighbors.end(), ostream_iterator<NodeUUID>(s2, "]["));
    cout << "commonNeighborsForDebtorAndCreditorNodes debtorNeighbors : " << s2.str() << endl;
    set<NodeUUID> commonNeighbors;
    set_intersection(
        creditorsNeighbors.begin(),
        creditorsNeighbors.end(),
        debtorsNeighbors.begin(),
        debtorsNeighbors.end(),
        std::inserter(
            commonNeighbors,
            commonNeighbors.begin()));

    return commonNeighbors;
}
