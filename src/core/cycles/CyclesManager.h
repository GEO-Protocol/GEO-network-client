#ifndef GEO_NETWORK_CLIENT_CYCLESMANAGER_H
#define GEO_NETWORK_CLIENT_CYCLESMANAGER_H

#include "../transactions/scheduler/TransactionsScheduler.h"
#include "../transactions/transactions/regular/payments/base/BasePaymentTransaction.h"
#include "../paths/lib/Path.h"
#include "../logger/Logger.h"
#include "../common/time/TimeUtils.h"
#include "../subsystems_controller/SubsystemsController.h"

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <vector>
#include <map>

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
    typedef signals::signal<void(const SerializedEquivalent)> BuildSixNodesCyclesSignal;
    typedef signals::signal<void(const SerializedEquivalent)> BuildFiveNodesCyclesSignal;
    typedef signals::signal<void(const SerializedEquivalent, Path::Shared)> CloseCycleSignal;

public:
    CyclesManager(
        const SerializedEquivalent equivalent,
        const NodeUUID &nodeUUID,
        TransactionsScheduler *transactionsScheduler,
        as::io_service &ioService,
        Logger &logger,
        SubsystemsController *subsystemsController);

    void closeOneCycle(
        bool nextCycleShouldBeRunned = false);

    void addCycle(
        Path::Shared);

    bool resolveReservationConflict(
        const TransactionUUID &challengerTransactionUUID,
        const TransactionUUID &reservedTransactionUUID);

    bool isTransactionStillAlive(
        const TransactionUUID &transactionUUID);

    void addClosedTrustLine(
        BaseAddress::Shared source,
        BaseAddress::Shared destination);

    void addOfflineNode(
        BaseAddress::Shared nodeAddress);

private:
    void runSignalSixNodes(
        const boost::system::error_code &err);

    void runSignalFiveNodes(
        const boost::system::error_code &err);

    void updateOfflineNodesAndClosedTLLists(
        const boost::system::error_code &err);

    vector<Path::Shared>* cyclesVector(
        CycleClosingState currentCycleClosingState);

    void incrementCurrentCycleClosingState();

    bool isChallengerTransactionWinReservation(
        BasePaymentTransaction::Shared challengerTransaction,
        BasePaymentTransaction::Shared reservedTransaction);

    void clearClosedCycles();

    void removeCyclesWithClosedTrustLine(
        BaseAddress::Shared sourceClosed,
        BaseAddress::Shared destinationClosed,
        vector<Path::Shared> &cycles);

    void removeCyclesWithOfflineNode(
        BaseAddress::Shared offlineNode,
        vector<Path::Shared> &cycles);

    uint32_t randomInitializer() const;

    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

public:
    mutable CloseCycleSignal closeCycleSignal;

    mutable BuildSixNodesCyclesSignal buildSixNodesCyclesSignal;

    mutable BuildFiveNodesCyclesSignal buildFiveNodesCyclesSignal;

private:
    const uint32_t kSixNodesSignalRepeatTimeSeconds = 24 * 60 * 60;
    const uint32_t kFiveNodesSignalRepeatTimeSeconds = 24 * 60 * 60;
    const uint16_t kPostponingRollbackTransactionTimeMSec = 10;
    const uint32_t kUpdatingTimerPeriodSeconds = 10 * 60;

    static const byte kOfflineNodesAndClosedTLLiveHours = 0;
    static const byte kOfflineNodesAndClosedTLLiveMinutes = 30;
    static const byte kOfflineNodesAndClosedTLLiveSeconds = 0;

    static Duration& kOfflineNodesAndClosedTLLiveDuration() {
        static auto duration = Duration(
            kOfflineNodesAndClosedTLLiveHours,
            kOfflineNodesAndClosedTLLiveMinutes,
            kOfflineNodesAndClosedTLLiveSeconds);
        return duration;
    }

private:
    TransactionsScheduler *mTransactionScheduler;
    NodeUUID mNodeUUID;
    SerializedEquivalent mEquivalent;
    as::io_service &mIOService;

    vector<Path::Shared> mThreeNodesCycles;
    vector<Path::Shared> mFourNodesCycles;
    vector<Path::Shared> mFiveNodesCycles;
    vector<Path::Shared> mSixNodesCycles;

    map<DateTime, pair<BaseAddress::Shared, BaseAddress::Shared>> mClosedTrustLines;
    map<DateTime, BaseAddress::Shared> mOfflineNodes;

    CycleClosingState mCurrentCycleClosingState;
    Logger &mLog;

    unique_ptr<as::steady_timer> mSixNodesCycleTimer;
    unique_ptr<as::steady_timer> mFiveNodesCycleTimer;
    unique_ptr<as::steady_timer> mUpdatingTimer;

    // prevent launching closing cycle if another one in closing process
    bool mIsCycleInProcess;

    SubsystemsController *mSubsystemsController;
};


#endif //GEO_NETWORK_CLIENT_CYCLESMANAGER_H
