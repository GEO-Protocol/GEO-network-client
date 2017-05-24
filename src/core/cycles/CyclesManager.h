#ifndef GEO_NETWORK_CLIENT_CYCLESMANAGER_H
#define GEO_NETWORK_CLIENT_CYCLESMANAGER_H

#include "../transactions/scheduler/TransactionsScheduler.h"
#include "../paths/lib/Path.h"
#include "../logger/Logger.h"

#include <boost/signals2.hpp>
#include <vector>

namespace signals = boost::signals2;

class CyclesManager {
public:
    enum CycleClosingState {
        ThreeNodes = 1,
        FourNodes,
        FiveNodes,
        SixNodes
    };

public:
    typedef signals::signal<void(Path::ConstShared cycle)> CloseCycleSignal;

public:
    CyclesManager(
        Logger *logger);

    void closeOneCycle();

    void addCycle(
        Path::ConstShared);

private:
    vector<Path::ConstShared> cyclesVector(
        CycleClosingState currentCycleClosingState) const;

    void incrementCurrentCycleClosingState();

    LoggerStream info() const;

    LoggerStream debug() const;

    const string logHeader() const;

public:
    mutable CloseCycleSignal closeCycleSignal;

private:
    TransactionsScheduler *mTransactionScheduler;
    vector<Path::ConstShared> mThreeNodesCycles;
    vector<Path::ConstShared> mFourNodesCycles;
    vector<Path::ConstShared> mFiveNodesCycles;
    vector<Path::ConstShared> mSixNodesCycles;

    CycleClosingState mCurrentCycleClosingState;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_CYCLESMANAGER_H
