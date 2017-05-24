#include "CyclesManager.h"

CyclesManager::CyclesManager(
    Logger *logger) :
    mLog(logger)
{
    mCurrentCycleClosingState = CycleClosingState::ThreeNodes;
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
        if (!cycles.empty()) {
            auto cycle = *cycles.begin();
            cycles.erase(cycles.begin());
            debug() << "closeCycleSignal " << cycle->toString();
            closeCycleSignal(cycle);
            return;
        }
        incrementCurrentCycleClosingState();
    } while (currentCycleClosingState != mCurrentCycleClosingState);
}

vector<Path::ConstShared> CyclesManager::cyclesVector(
    CycleClosingState currentCycleClosingState) const
{
    switch (currentCycleClosingState) {
        case CycleClosingState::ThreeNodes:
            return mThreeNodesCycles;
        case CycleClosingState::FourNodes:
            return mFourNodesCycles;
        case CycleClosingState::FiveNodes:
            return mFiveNodesCycles;
        case CycleClosingState::SixNodes:
            return mSixNodesCycles;
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
