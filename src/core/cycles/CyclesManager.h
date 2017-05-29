#ifndef GEO_NETWORK_CLIENT_CYCLESMANAGER_H
#define GEO_NETWORK_CLIENT_CYCLESMANAGER_H

#include "../transactions/scheduler/TransactionsScheduler.h"
#include "../transactions/transactions/regular/payments/base/BasePaymentTransaction.h"
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

    bool resolveReservationConflict(
        const TransactionUUID &challengerTransactionUUID,
        const TransactionUUID &reservedTransactionUUID);

    bool isTransactionStillAlive(
        const TransactionUUID &transactionUUID);

private:
    void runSignalSixNodes(
        const boost::system::error_code &error);

    void runSignalFiveNodes(
        const boost::system::error_code &error);

    vector<Path::ConstShared>* cyclesVector(
        CycleClosingState currentCycleClosingState);

    void incrementCurrentCycleClosingState();

    bool isChellengerTransactionWinReservation(
        BasePaymentTransaction::Shared chellengerTransaction,
        BasePaymentTransaction::Shared reservedTransaction);

    LoggerStream info() const;

    LoggerStream debug() const;

    const string logHeader() const;

public:
    mutable CloseCycleSignal closeCycleSignal;

    mutable BuildSixNodesCyclesSignal buildSixNodesCyclesSignal;

    mutable BuildFiveNodesCyclesSignal buildFiveNodesCyclesSignal;

private:
    const uint32_t kSixNodesSignalRepeatTimeSeconds = 24 * 60 * 60;
    const uint32_t kFiveNodesSignalRepeatTimeSeconds = 24 * 60 * 60;
    const uint16_t kPostponningRollbackTransactionTimeMSec = 10;

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
