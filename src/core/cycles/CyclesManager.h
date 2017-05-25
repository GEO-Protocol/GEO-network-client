#ifndef GEO_NETWORK_CLIENT_CYCLESMANAGER_H
#define GEO_NETWORK_CLIENT_CYCLESMANAGER_H

#include "../transactions/scheduler/TransactionsScheduler.h"
#include "../paths/lib/Path.h"
#include "../logger/Logger.h"

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <vector>

namespace as = boost::asio;
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
    typedef signals::signal<void()> BuildSixNodesCyclesSignal;
    typedef signals::signal<void()> BuildFiveNodesCyclesSignal;
    typedef signals::signal<void(Path::ConstShared cycle)> CloseCycleSignal;

public:
    CyclesManager(
        TransactionsScheduler *transactionsScheduler,
        as::io_service &ioService,
        Logger *logger);

    void closeOneCycle();

    void addCycle(
        Path::ConstShared);

private:
    void runSignalSixNodes(
        const boost::system::error_code &error);

    void runSignalFiveNodes(
        const boost::system::error_code &error);

private:
    vector<Path::ConstShared>* cyclesVector(
        CycleClosingState currentCycleClosingState);

    void incrementCurrentCycleClosingState();

    LoggerStream info() const;

    LoggerStream debug() const;

    const string logHeader() const;

public:
    mutable CloseCycleSignal closeCycleSignal;

    mutable BuildSixNodesCyclesSignal buildSixNodesCyclesSignal;

    mutable BuildFiveNodesCyclesSignal buildFiveNodesCyclesSignal;

private:
    const int mSixNodesSignalRepeatTimeSeconds = 24 * 60 * 60;
    const int mFiveNodesSignalRepeatTimeSeconds = 24* 60 * 60;

private:
    TransactionsScheduler *mTransactionScheduler;
    as::io_service &mIOService;
    vector<Path::ConstShared> mThreeNodesCycles;
    vector<Path::ConstShared> mFourNodesCycles;
    vector<Path::ConstShared> mFiveNodesCycles;
    vector<Path::ConstShared> mSixNodesCycles;

    CycleClosingState mCurrentCycleClosingState;
    Logger *mLog;

    unique_ptr<as::steady_timer> mSixNodesCycleTimer;
    unique_ptr<as::steady_timer> mFiveNodesCycleTimer;
};


#endif //GEO_NETWORK_CLIENT_CYCLESMANAGER_H
