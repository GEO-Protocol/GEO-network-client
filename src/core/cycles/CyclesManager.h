#ifndef GEO_NETWORK_CLIENT_CYCLESMANAGER_H
#define GEO_NETWORK_CLIENT_CYCLESMANAGER_H

#include "../transactions/scheduler/TransactionsScheduler.h"
#include "../transactions/transactions/regular/payments/base/BasePaymentTransaction.h"
#include "../paths/lib/Path.h"
#include "../paths/lib/PathNew.h"
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
    typedef signals::signal<void(const SerializedEquivalent equivalent)> BuildSixNodesCyclesSignal;
    typedef signals::signal<void(const SerializedEquivalent equivalent)> BuildFiveNodesCyclesSignal;
    typedef signals::signal<void(
            const SerializedEquivalent equivalent,
            Path::ConstShared cycle)> CloseCycleSignal;
    typedef signals::signal<void(
            const SerializedEquivalent equivalent,
            PathNew::ConstShared cycle)> CloseCycleSignalNew;

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

    void closeOneCycleNew(
        bool nextCycleShouldBeRunned = false);

    void addCycle(
        Path::ConstShared);

    void addCycle(
        PathNew::ConstShared);

    bool resolveReservationConflict(
        const TransactionUUID &challengerTransactionUUID,
        const TransactionUUID &reservedTransactionUUID);

    bool isTransactionStillAlive(
        const TransactionUUID &transactionUUID);

    void addClosedTrustLine(
        const NodeUUID &source,
        const NodeUUID &destination);

    void addClosedTrustLineNew(
        BaseAddress::Shared source,
        BaseAddress::Shared destination);

    void addOfflineNode(
        const NodeUUID &nodeUUID);

    void addOfflineNodeNew(
        BaseAddress::Shared nodeAddress);

private:
    void runSignalSixNodes(
        const boost::system::error_code &err);

    void runSignalFiveNodes(
        const boost::system::error_code &err);

    void updateOfflineNodesAndClosedTLLists(
        const boost::system::error_code &err);

    vector<Path::ConstShared>* cyclesVector(
        CycleClosingState currentCycleClosingState);

    vector<PathNew::ConstShared>* cyclesVectorNew(
        CycleClosingState currentCycleClosingState);

    void incrementCurrentCycleClosingState();

    bool isChallengerTransactionWinReservation(
        BasePaymentTransaction::Shared challengerTransaction,
        BasePaymentTransaction::Shared reservedTransaction);

    void clearClosedCycles();

    void clearClosedCyclesNew();

    void removeCyclesWithClosedTrustLine(
        const NodeUUID &sourceClosed,
        const NodeUUID &destinationClosed,
        vector<Path::ConstShared> &cycles);

    void removeCyclesWithClosedTrustLineNew(
        BaseAddress::Shared sourceClosed,
        BaseAddress::Shared destinationClosed,
        vector<PathNew::ConstShared> &cycles);

    void removeCyclesWithOfflineNode(
        const NodeUUID &offlineNode,
        vector<Path::ConstShared> &cycles);

    void removeCyclesWithOfflineNodeNew(
        BaseAddress::Shared offlineNode,
        vector<PathNew::ConstShared> &cycles);

    uint32_t randomInitializer() const;

    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

public:
    mutable CloseCycleSignal closeCycleSignal;

    mutable CloseCycleSignalNew closeCycleSignalNew;

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
    vector<Path::ConstShared> mThreeNodesCycles;
    vector<Path::ConstShared> mFourNodesCycles;
    vector<Path::ConstShared> mFiveNodesCycles;
    vector<Path::ConstShared> mSixNodesCycles;

    vector<PathNew::ConstShared> mThreeNodesCyclesNew;
    vector<PathNew::ConstShared> mFourNodesCyclesNew;
    vector<PathNew::ConstShared> mFiveNodesCyclesNew;
    vector<PathNew::ConstShared> mSixNodesCyclesNew;

    map<DateTime, pair<NodeUUID, NodeUUID>> mClosedTrustLines;
    map<DateTime, NodeUUID> mOfflineNodes;

    map<DateTime, pair<BaseAddress::Shared, BaseAddress::Shared>> mClosedTrustLinesNew;
    map<DateTime, BaseAddress::Shared> mOfflineNodesNew;

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
