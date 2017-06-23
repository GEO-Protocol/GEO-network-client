#include "CyclesManager.h"

CyclesManager::CyclesManager(
    TransactionsScheduler *transactionsScheduler,
    as::io_service &ioService,
    Logger &logger) :

    mTransactionScheduler(transactionsScheduler),
    mIOService(ioService),
    mLog(logger),
    mIsCycleInProcess(false)
{
    mCurrentCycleClosingState = CycleClosingState::ThreeNodes;

    srand(time(NULL));
    int timeStarted = rand() % 60 * 60 * 6;
#ifdef TESTS
    timeStarted = 20;
#endif
    mFiveNodesCycleTimer = make_unique<as::steady_timer>(
        mIOService);
    mFiveNodesCycleTimer->expires_from_now(
        std::chrono::seconds(
            timeStarted));
    mFiveNodesCycleTimer->async_wait(
        boost::bind(
            &CyclesManager::runSignalFiveNodes,
            this,
            as::placeholders::error));
    timeStarted = rand() % 60 * 60 * 6;
#ifdef TESTS
    timeStarted = 20;
#endif
    mSixNodesCycleTimer = make_unique<as::steady_timer>(
        mIOService);
    mSixNodesCycleTimer->expires_from_now(
        std::chrono::seconds(
            timeStarted));
    mSixNodesCycleTimer->async_wait(
        boost::bind(
            &CyclesManager::runSignalSixNodes,
            this,
            as::placeholders::error));
}

void CyclesManager::addCycle(
    Path::ConstShared cycle)
{
    switch (cycle->length()) {
        case 4:
            debug() << "add three nodes cycle";
            mThreeNodesCycles.push_back(cycle);
            break;
        case 5:
            debug() << "add four nodes cycle";
            mFourNodesCycles.push_back(cycle);
            break;
        case 6:
            debug() << "add five nodes cycle";
            mFiveNodesCycles.push_back(cycle);
            break;
        case 7:
            debug() << "add six nodes cycle";
            mSixNodesCycles.push_back(cycle);
            break;
        default:
            throw ValueError("CyclesManager::addCycle: "
                                 "illegal length of cycle");
    }
}

void CyclesManager::closeOneCycle(
    bool nextCycleShouldBeRunned)
{
    // TODO : recovery in cycles can create desynchronization,
    // that's why we temporarily turn off them
    return;
    // nextCycleShouldBeRunned equals true when method closeOneCycle was called
    // by TransactionManager after finishing transaction of closing cycle.
    // When method closeOneCycle was called by transactions of building cycles it equals false
    if (nextCycleShouldBeRunned) {
        mIsCycleInProcess = false;
    }
    clearClosedCycles();
    debug() << "closeOneCycle";
    debug() << "currentCycleClosingState: " << mCurrentCycleClosingState;
    debug() << "3 NC count: " << mThreeNodesCycles.size()
            << " 4 NC count: " << mFourNodesCycles.size()
            << " 5 NC count: " << mFiveNodesCycles.size()
            << " 6 NC count: " << mSixNodesCycles.size();
    if (mIsCycleInProcess) {
        debug() << "Postpone closing this cycle, because another one in process";
        return;
    }
    CycleClosingState currentCycleClosingState = mCurrentCycleClosingState;
    do {
        auto cycles = cyclesVector(
            mCurrentCycleClosingState);
        if (!cycles->empty()) {
            auto cycle = *cycles->begin();
            cycles->erase(cycles->begin());
            debug() << "closeCycleSignal " << cycle->toString();
            closeCycleSignal(cycle);
            mIsCycleInProcess = true;
            return;
        }
        incrementCurrentCycleClosingState();
    } while (currentCycleClosingState != mCurrentCycleClosingState);
    mIsCycleInProcess = false;
}

vector<Path::ConstShared>* CyclesManager::cyclesVector(
    CycleClosingState currentCycleClosingState)
{
    switch (currentCycleClosingState) {
        case CycleClosingState::ThreeNodes:
            return &mThreeNodesCycles;
        case CycleClosingState::FourNodes:
            return &mFourNodesCycles;
        case CycleClosingState::FiveNodes:
            return &mFiveNodesCycles;
        case CycleClosingState::SixNodes:
            return &mSixNodesCycles;
    }
}

void CyclesManager::incrementCurrentCycleClosingState()
{
    switch (mCurrentCycleClosingState) {
        case CycleClosingState::ThreeNodes:
            mCurrentCycleClosingState = CycleClosingState::FourNodes;
            break;
        case CycleClosingState::FourNodes:
            mCurrentCycleClosingState = CycleClosingState::FiveNodes;
            break;
        case CycleClosingState::FiveNodes:
            mCurrentCycleClosingState = CycleClosingState::SixNodes;
            break;
        case CycleClosingState::SixNodes:
            mCurrentCycleClosingState = CycleClosingState::ThreeNodes;
            break;
    }
}

void CyclesManager::runSignalFiveNodes(
    const boost::system::error_code &error)
{
    if (error) {
        cout << error.message() << endl;
    }
    mFiveNodesCycleTimer->cancel();
    mFiveNodesCycleTimer->expires_from_now(
        std::chrono::seconds(
            kFiveNodesSignalRepeatTimeSeconds));
    mFiveNodesCycleTimer->async_wait(
        boost::bind(
            &CyclesManager::runSignalFiveNodes,
            this,
            as::placeholders::error));
    buildFiveNodesCyclesSignal();
}

void CyclesManager::runSignalSixNodes(
    const boost::system::error_code &error)
{
    if (error) {
        cout << error.message() << endl;
    }
    mSixNodesCycleTimer->cancel();
    mSixNodesCycleTimer->expires_from_now(
        std::chrono::seconds(
            kSixNodesSignalRepeatTimeSeconds));
    mSixNodesCycleTimer->async_wait(
        boost::bind(
            &CyclesManager::runSignalSixNodes,
            this,
            as::placeholders::error));
    buildSixNodesCyclesSignal();
}

bool CyclesManager::isChellengerTransactionWinReservation(
    BasePaymentTransaction::Shared chellengerTransaction,
    BasePaymentTransaction::Shared reservedTransaction)
{
    debug() << "isChellengerTransactionWinReservation chellenger: " << chellengerTransaction->currentTransactionUUID()
            << " nodeUUID: " << chellengerTransaction->currentNodeUUID()
            << " transaction type: " << chellengerTransaction->transactionType()
            << " votesCheckingStage: " << chellengerTransaction->isCommonVotesCheckingstage()
            << " cycle length: " << to_string(chellengerTransaction->cycleLength())
            << " coordinator: " << chellengerTransaction->coordinatorUUID();
    debug() << "isChellengerTransactionWinReservation reserved: " << reservedTransaction->currentTransactionUUID()
            << " nodeUUID: " << chellengerTransaction->currentNodeUUID()
            << " transaction type: " << reservedTransaction->transactionType()
            << " votesCheckingStage: " << reservedTransaction->isCommonVotesCheckingstage()
            << " cycle length: " << to_string(reservedTransaction->cycleLength())
            << " coordinator: " << reservedTransaction->coordinatorUUID();
    if (reservedTransaction->transactionType() != BaseTransaction::TransactionType::Payments_CycleCloserInitiatorTransaction
        && reservedTransaction->transactionType() != BaseTransaction::TransactionType::Payments_CycleCloserIntermediateNodeTransaction) {
        debug() << "isChellengerTransactionWinReservation false: reserved is not cycle transaction";
        return false;
    }
    if (reservedTransaction->isCommonVotesCheckingstage()) {
        debug() << "isChellengerTransactionWinReservation false: reserved on votesChecking stage";
        return false;
    }
    if (chellengerTransaction->cycleLength() != reservedTransaction->cycleLength()) {
        debug() << "isChellengerTransactionWinReservation "
                << (chellengerTransaction->cycleLength() > reservedTransaction->cycleLength()) << " on cycles lengths";
        return chellengerTransaction->cycleLength() > reservedTransaction->cycleLength();
    }
    debug() << "isChellengerTransactionWinReservation "
            << (chellengerTransaction->coordinatorUUID() > reservedTransaction->coordinatorUUID()) << " on coordinatorUUIDs";
    return chellengerTransaction->coordinatorUUID() > reservedTransaction->coordinatorUUID();
}

bool CyclesManager::resolveReservationConflict(
    const TransactionUUID &challengerTransactionUUID,
    const TransactionUUID &reservedTransactionUUID)
{
    debug() << "resolveReservationConflict";
    auto challengerTransaction = static_pointer_cast<BasePaymentTransaction>(
        mTransactionScheduler->transactionByUUID(
            challengerTransactionUUID));
    auto reservedTransaction = static_pointer_cast<BasePaymentTransaction>(
        mTransactionScheduler->transactionByUUID(
            reservedTransactionUUID));
    debug() << "conflict between  " << challengerTransactionUUID << " and " << reservedTransactionUUID;
    if (isChellengerTransactionWinReservation(
        challengerTransaction,
        reservedTransaction)) {
        reservedTransaction->setRollbackByOtherTransactionStage();
        mTransactionScheduler->postponeTransaction(
            reservedTransaction,
            kPostponningRollbackTransactionTimeMSec);
        return true;
    }
    return false;
}

bool CyclesManager::isTransactionStillAlive(
    const TransactionUUID &transactionUUID)
{
    debug() << "isTransactionStillAlive: " << transactionUUID;
    try {
        mTransactionScheduler->transactionByUUID(
            transactionUUID);
        debug() << "Still alive";
        return true;
    } catch (NotFoundError &e) {
        debug() << "Not alive";
        return false;
    }
}

void CyclesManager::addClosedTrustLine(
    const NodeUUID &source,
    const NodeUUID &destination)
{
    mClosedTrustLines.push_back(
        make_pair(
            source,
            destination));
}

void CyclesManager::addOfflineNode(
    const NodeUUID &nodeUUID)
{
    mOfflineNodes.push_back(
        nodeUUID);
}

void CyclesManager::removeCyclesWithClosedTrustLine(
    const NodeUUID &sourceClosed,
    const NodeUUID &destinationClosed,
    vector<Path::ConstShared> &cycles)
{
    auto itCycle = cycles.begin();
    while (itCycle != cycles.end()) {
        if ((*itCycle)->containsTrustLine(
            sourceClosed,
            destinationClosed)) {
            cycles.erase(
                itCycle);
        } else {
            itCycle++;
        }
    }
}

void CyclesManager::removeCyclesWithOfflineNode(
    const NodeUUID &offlineNode,
    vector<Path::ConstShared> &cycles)
{
    auto itCycle = cycles.begin();
    while (itCycle != cycles.end()) {
        if ((*itCycle)->positionOfNode(
            offlineNode) >= 0) {
            cycles.erase(
                itCycle);
        } else {
            itCycle++;
        }
    }
}

void CyclesManager::clearClosedCycles()
{
    debug() << "clearClosedCycles closed trust lines cnt: " << mClosedTrustLines.size();
    if (!mClosedTrustLines.empty()) {
        auto source = mClosedTrustLines.begin()->first;
        auto destination = mClosedTrustLines.begin()->second;
        mClosedTrustLines.erase(mClosedTrustLines.begin());

        removeCyclesWithClosedTrustLine(
            source,
            destination,
            mThreeNodesCycles);

        removeCyclesWithClosedTrustLine(
            source,
            destination,
            mFourNodesCycles);

        removeCyclesWithClosedTrustLine(
            source,
            destination,
            mFiveNodesCycles);

        removeCyclesWithClosedTrustLine(
            source,
            destination,
            mSixNodesCycles);
    }

    debug() << "clearClosedCycles offline nodes cnt: " << mOfflineNodes.size();
    if (!mOfflineNodes.empty()) {
        auto offlineNode = *mOfflineNodes.begin();
        mOfflineNodes.erase(
            mOfflineNodes.begin());

        removeCyclesWithOfflineNode(
            offlineNode,
            mThreeNodesCycles);

        removeCyclesWithOfflineNode(
            offlineNode,
            mFourNodesCycles);

        removeCyclesWithOfflineNode(
            offlineNode,
            mFiveNodesCycles);

        removeCyclesWithOfflineNode(
            offlineNode,
            mSixNodesCycles);
    }
}

LoggerStream CyclesManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream CyclesManager::debug() const
{
    return mLog.debug(logHeader());
}

const string CyclesManager::logHeader() const
{
    return "[CyclesManager]";
}
