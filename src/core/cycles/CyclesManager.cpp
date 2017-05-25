#include "CyclesManager.h"

CyclesManager::CyclesManager(
    TransactionsScheduler *transactionsScheduler,
    as::io_service &ioService,
    Logger *logger) :

    mTransactionScheduler(transactionsScheduler),
    mIOService(ioService),
    mLog(logger)
{
    mCurrentCycleClosingState = CycleClosingState::ThreeNodes;

    //    todo add set Time started to 60*6
    srand(time(NULL));
    int timeStarted = rand() % 60 * 60 * 6;
#ifdef TESTS
    timeStarted = 60;
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
    //    todo add set Time started to 60*6
    timeStarted = rand() % 60 * 60 * 6;
#ifdef TESTS
    timeStarted = 60;
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

void CyclesManager::closeOneCycle()
{
    debug() << "closeOneCycle";
    debug() << "currentCycleClosingState: " << mCurrentCycleClosingState;
    debug() << "3 NC count: " << mThreeNodesCycles.size()
            << " 4 NC count: " << mFourNodesCycles.size()
            << " 5 NC count: " << mFiveNodesCycles.size()
            << " 6 NC count: " << mSixNodesCycles.size();
    CycleClosingState currentCycleClosingState = mCurrentCycleClosingState;
    do {
        auto cycles = cyclesVector(
            mCurrentCycleClosingState);
        if (!cycles->empty()) {
            auto cycle = *cycles->begin();
            cycles->erase(cycles->begin());
            debug() << "closeCycleSignal " << cycle->toString();
            closeCycleSignal(cycle);
            return;
        }
        incrementCurrentCycleClosingState();
    } while (currentCycleClosingState != mCurrentCycleClosingState);
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
    debug() << "incrementCurrentCycleClosingState from " << mCurrentCycleClosingState;
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
            mFiveNodesSignalRepeatTimeSeconds));
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
            mSixNodesSignalRepeatTimeSeconds));
    mSixNodesCycleTimer->async_wait(
        boost::bind(
            &CyclesManager::runSignalSixNodes,
            this,
            as::placeholders::error));
    buildSixNodesCyclesSignal();
}

LoggerStream CyclesManager::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream CyclesManager::debug() const
{
    // TODO: remove me. Logger must be initialised in constructor by default
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->debug(logHeader());
}

const string CyclesManager::logHeader() const
{
    return "[CyclesManager]";
}
