#ifndef GEO_NETWORK_CLIENT_CYCLESMANAGER_H
#define GEO_NETWORK_CLIENT_CYCLESMANAGER_H

#include "../transactions/scheduler/TransactionsScheduler.h"
#include "../transactions/transactions/regular/payments/base/BasePaymentTransaction.h"
#include "../paths/lib/Path.h"
#include "../logger/Logger.h"
#include "../subsystems_controller/SubsystemsController.h"

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
        const NodeUUID &nodeUUID,
        TransactionsScheduler *transactionsScheduler,
        as::io_service &ioService,
        Logger &logger,
        SubsystemsController *subsystemsController);

    void closeOneCycle(
        bool nextCycleShouldBeRunned = false);

    void addCycle(
        Path::ConstShared);

    bool resolveReservationConflict(
        const TransactionUUID &challengerTransactionUUID,
        const TransactionUUID &reservedTransactionUUID);

    bool isTransactionStillAlive(
        const TransactionUUID &transactionUUID);

    void addClosedTrustLine(
        const NodeUUID &source,
        const NodeUUID &destination);

    void addOfflineNode(
        const NodeUUID &nodeUUID);

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

    void clearClosedCycles();

    void removeCyclesWithClosedTrustLine(
        const NodeUUID &sourceClosed,
        const NodeUUID &destinationClosed,
        vector<Path::ConstShared> &cycles);

    void removeCyclesWithOfflineNode(
        const NodeUUID &offlineNode,
        vector<Path::ConstShared> &cycles);

    uint32_t randomInitializer() const;

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
    NodeUUID mNodeUUID;
    as::io_service &mIOService;
    vector<Path::ConstShared> mThreeNodesCycles;
    vector<Path::ConstShared> mFourNodesCycles;
    vector<Path::ConstShared> mFiveNodesCycles;
    vector<Path::ConstShared> mSixNodesCycles;

    vector<pair<NodeUUID, NodeUUID>> mClosedTrustLines;
    vector<NodeUUID> mOfflineNodes;

    CycleClosingState mCurrentCycleClosingState;
    Logger &mLog;

    unique_ptr<as::steady_timer> mSixNodesCycleTimer;
    unique_ptr<as::steady_timer> mFiveNodesCycleTimer;

    // prevent launching closing cycle if another one in closing process
    bool mIsCycleInProcess;

    SubsystemsController *mSubsystemsController;
};


#endif //GEO_NETWORK_CLIENT_CYCLESMANAGER_H
