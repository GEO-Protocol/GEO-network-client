#include "CyclesFourNodesInitTransaction.h"

CyclesFourNodesInitTransaction::CyclesFourNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &debtorContractorUUID,
    const NodeUUID &creditorContractorUUID,
    TrustLinesManager *manager,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesInitTransaction,
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mStorageHandler(storageHandler),
    mDebtorContractorUUID(debtorContractorUUID),
    mCreditorContractorUUID(creditorContractorUUID)
{}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::run()
{
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

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runCollectDataAndSendMessageStage()
{
    debug() << "runCollectDataAndSendMessageStage from " << mDebtorContractorUUID << " to " << mCreditorContractorUUID;
    set<NodeUUID> neighbors = commonNeighborsForDebtorAndCreditorNodes();
    if (neighbors.size() == 0){
        info() << "CyclesFourNodesInitTransaction::runCollectDataAndSendMessageStage: "
               << "No common neighbors. Exit Transaction" << endl;
        return resultDone();
    }
    sendMessage<CyclesFourNodesBalancesRequestMessage>(
        mDebtorContractorUUID,
        mNodeUUID,
        currentTransactionUUID(),
        neighbors);

    sendMessage<CyclesFourNodesBalancesRequestMessage>(
        mCreditorContractorUUID,
        mNodeUUID,
        currentTransactionUUID(),
        neighbors);

    mStep = Stages::ParseMessageAndCreateCycles;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypesAndAwakeAfterMilliseconds(
            {Message::MessageType::Cycles_FourNodesBalancesResponse},
            mkWaitingForResponseTime));
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.size() != 2) {
        info() << "No responses messages are present. Can't create cycles paths;";

        return resultDone();
    }
    const auto firstMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*mContext.begin());
    const auto secondMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*(mContext.end()-1));
    const auto firstContractorUUID = firstMessage->senderUUID;
    const auto secondContractorUUID = secondMessage->senderUUID;

    const TrustLineBalance zeroBalance = 0;
    TrustLineBalance firstContractorBalance = mTrustLinesManager->balance(firstContractorUUID);
    TrustLineBalance secondContractorBalance = mTrustLinesManager->balance(secondContractorUUID);

    // In case if some payment operation was done and balances on the nodes was changed -
    // this check prevents redundant cycles closing operations.
    if ((firstContractorBalance > zeroBalance and secondContractorBalance > zeroBalance) or
        (firstContractorBalance < zeroBalance and secondContractorBalance < zeroBalance) or
        (firstContractorBalance == zeroBalance and secondContractorBalance == zeroBalance)) {

        info() << "Balances between initiator node and (" << firstContractorUUID <<  "), or "
                  "between initiator node and (" << secondContractorUUID << ") was changed. "
                  "Cannot create cycles.";
        return resultDone();
    }
    #ifdef TESTS
        vector<vector<NodeUUID>> ResultCycles;
    #endif
    bool isBuildCycles = false;
    for (auto &kNodeUUIDFirstMessage: firstMessage->NeighborsUUID()){
        for (auto &kNodeUUIDSecondMessage: secondMessage->NeighborsUUID()){
            if (kNodeUUIDSecondMessage == kNodeUUIDFirstMessage){
                vector<NodeUUID> stepPath = {
                    mDebtorContractorUUID,
                    kNodeUUIDSecondMessage,
                    mCreditorContractorUUID};
                debug() << "build path: " << mNodeUUID << " -> " << mDebtorContractorUUID
                        << " -> " << kNodeUUIDSecondMessage << " -> " << mCreditorContractorUUID
                        << " -> " << mNodeUUID;
                const auto cyclePath = make_shared<Path>(
                    mNodeUUID,
                    mNodeUUID,
                    stepPath);
                mCyclesManager->addCycle(
                    cyclePath);
                isBuildCycles = true;
                #ifdef TESTS
                    ResultCycles.push_back(stepPath);
                #endif
            }
        }
    }
    if (isBuildCycles) {
        cycleIsReadyForClosingSignal();
    }
    #ifdef TESTS
        cout << "CyclesThreeNodesInitTransaction::ResultCyclesCount " << to_string(ResultCycles.size()) << endl;
        for (vector<NodeUUID> KCyclePath: ResultCycles){
            stringstream ss;
            copy(KCyclePath.begin(), KCyclePath.end(), ostream_iterator<NodeUUID>(ss, ","));
            cout << "CyclesThreeNodesInitTransaction::CyclePath " << ss.str() << endl;
        }
        cout << "CyclesThreeNodesInitTransaction::End" << endl;
    #endif
    return resultDone();
}

set<NodeUUID> CyclesFourNodesInitTransaction::commonNeighborsForDebtorAndCreditorNodes()
{
    const auto creditorsNeighbors = mStorageHandler->routingTablesHandler()->neighborsOfOnRT2(
        mCreditorContractorUUID);
    const auto debtorsNeighbors = mStorageHandler->routingTablesHandler()->neighborsOfOnRT2(
        mDebtorContractorUUID);
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

const string CyclesFourNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesInitTransactionTA: " << currentTransactionUUID() << "] ";

    return s.str();
}
