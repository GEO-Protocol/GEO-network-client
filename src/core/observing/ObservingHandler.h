#ifndef GEO_NETWORK_CLIENT_OBSERVINGHANDLER_H
#define GEO_NETWORK_CLIENT_OBSERVINGHANDLER_H

#include "ObservingCommunicator.h"
#include "ObservingTransaction.h"
#include "messages/ObservingClaimAppendResponseMessage.h"
#include "messages/ObservingTransactionsRequestMessage.h"
#include "messages/ObservingTransactionsResponseMessage.h"
#include "messages/ObservingBlockNumberRequest.h"
#include "messages/ObservingBlockNumberResponse.h"
#include "messages/ObservingParticipantsVotesRequestMessage.h"
#include "messages/ObservingParticipantsVotesResponseMessage.h"
#include "messages/ObservingParticipantsVotesAppendRequestMessage.h"
#include "messages/ObservingParticipantsVotesAppendResponseMessage.h"

#include "../io/storage/StorageHandler.h"
#include "../resources/manager/ResourcesManager.h"
#include "../resources/resources/BlockNumberRecourse.h"
#include "../logger/LoggerMixin.hpp"

#include <boost/asio/steady_timer.hpp>
#include <vector>
#include <map>

using namespace std;
namespace as = boost::asio;
namespace signals = boost::signals2;

class ObservingHandler : protected LoggerMixin {

public:
    typedef signals::signal<void(
            const TransactionUUID&,
            BlockNumber,
            map<PaymentNodeID, lamport::Signature::Shared>)> ParticipantsVotesSignal;
    typedef signals::signal<void(const TransactionUUID&, BlockNumber)> RejectTransactionSignal;
    typedef signals::signal<void(const TransactionUUID&, BlockNumber)> UncertainTransactionSignal;
    typedef signals::signal<void(const TransactionUUID&, BlockNumber)> CancelTransactionSignal;
    typedef signals::signal<void(bool)> AllowPaymentTransactionsSignal;

public:
    ObservingHandler(
        vector<pair<string, string>> observersAddressesStr,
        IOService &ioService,
        StorageHandler *storageHandler,
        ResourcesManager *resourcesManager,
        Logger &logger);

    void sendClaimRequestToObservers(
        ObservingClaimAppendRequestMessage::Shared request);

    void addTransactionForChecking(
        const TransactionUUID& transactionUUID,
        BlockNumber maxBlockNumberForClaiming);

    void requestActualBlockNumber(
        const TransactionUUID& transactionUUID);

protected:
    const string logHeader() const override;

    void initialObservingRequest();

    /**
     * @returns timestamp, when next timer awakeness must be performed.
     * This method checks all claims and returns the smallest one time duration,
     * between now and claim timeout.
     */
    const DateTime closestClaimPerformingTimestamp() const
    noexcept;

    /**
     * Schedules timer for next awakeness.
     */
    void rescheduleResending();

    /**
     * Performs action with claims.
     * This method would be called every time when some claim timeout would fire up.
     */
    void performActions();

    /**
     * Performs processing of one claim
     * @param observingTransaction
     * @return true if claim should be removed from queue
     */
    bool performOneClaim(
        ObservingTransaction::Shared observingTransaction);

    void sendClaimAgain(
        ObservingTransaction::Shared observingTransaction);

    bool getParticipantsVotes(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

    void runTransactionsChecking(
        const boost::system::error_code &errorCode);

    void responseActualBlockNumber(
        const TransactionUUID& transactionUUID);

    void getActualBlockNumber();

    bool sendParticipantsVoteMessageToObservers(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

public:
    mutable ParticipantsVotesSignal mParticipantsVotesSignal;
    mutable RejectTransactionSignal mRejectTransactionSignal;
    mutable UncertainTransactionSignal mUncertainTransactionSignal;
    mutable CancelTransactionSignal mCancelTransactionSignal;
    mutable AllowPaymentTransactionsSignal mAllowPaymentTransactionsSignal;

private:
    static const uint32_t kInitialObservingRequestShiftSeconds = 5;
    static const uint32_t kInitialObservingRequestNextSeconds = 30;
    static const BlockNumber kDefaultBlockNumber = 777;

    // 6 min for check one transaction state minimum 3 times
    static const uint32_t kTransactionCheckingSignalRepeatTimeSeconds = 360;
    static const uint32_t kTransactionCheckingSignalSmallRepeatTimeSeconds = 60;

#ifdef TESTS
    static const uint32_t kTransactionCheckingSignalRepeatTimeSecondsTests = 60;
    static const uint32_t kTransactionCheckingSignalSmallRepeatTimeSecondsTests = 10;
#endif

    // block number updating period
    static const byte kBlockNumberUpdateHours = 1;
    static const byte kBlockNumberUpdateMinutes = 0;
    static const byte kBlockNumberUpdateSeconds = 0;
    static Duration& kBlockNumberUpdateDuration() {
        static auto duration = Duration(
            kBlockNumberUpdateHours,
            kBlockNumberUpdateMinutes,
            kBlockNumberUpdateSeconds);
        return duration;
    }

    static const uint16_t kApproximateBlockNumberIncrementingPeriodSeconds = 60;
#ifdef TESTS
    static const uint16_t kApproximateBlockNumberIncrementingPeriodSecondsTests = 20;
#endif

private:
    vector<IPv4WithPortAddress::Shared> mObservers;
    unique_ptr<ObservingCommunicator> mObservingCommunicator;
    map<TransactionUUID, ObservingTransaction::Shared> mClaims;
    // transactionUUID and block number after which claim is impossible
    map<TransactionUUID, BlockNumber> mCheckedTransactions;

    // number of block getting with last response and response time
    pair<BlockNumber, DateTime> mLastUpdatedBlockNumber;

    as::steady_timer mBlockNumberRequestTimer;
    as::steady_timer mClaimsTimer;
    as::steady_timer mTransactionsTimer;
    as::steady_timer mRequestsTimer;

    StorageHandler *mStorageHandler;
    ResourcesManager *mResourcesManager;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGHANDLER_H
