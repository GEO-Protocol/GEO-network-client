#include "ObservingHandler.h"

ObservingHandler::ObservingHandler(
    vector<pair<string, string>> observersAddressesStr,
    IOService &ioService,
    StorageHandler *storageHandler,
    ResourcesManager *resourcesManager,
    Logger &logger) :
    LoggerMixin(logger),
    mIOService(ioService),
    mObservingCommunicator(
        make_unique<ObservingCommunicator>(
            ioService,
            logger)),
    mInitialRequestTimer(ioService),
    mClaimsTimer(ioService),
    mTransactionsTimer(ioService),
    mRequestsTimer(ioService),
    mStorageHandler(storageHandler),
    mResourcesManager(resourcesManager)
{
    if (observersAddressesStr.empty()) {
        throw ValueError("ObservingHandler: empty observers list");
    }
    for (const auto &addressStr : observersAddressesStr) {
        if (addressStr.first == "ipv4") {
            try {
                mObservers.push_back(
                    make_shared<IPv4WithPortAddress>(
                        addressStr.second));
            } catch (...) {
                throw ValueError("ObservingHandler: can't create own address of type " + addressStr.first);
            }

        } else {
            throw ValueError("ObservingHandler: can't create own address. "
                                 "Wrong address type " + addressStr.first);
        }
    }

    mInitialRequestTimer.expires_from_now(
        std::chrono::seconds(
            kInitialObservingRequestShiftSeconds));
    mTransactionsTimer.async_wait(
        boost::bind(
            &ObservingHandler::initialObservingRequest,
            this));
}

void ObservingHandler::sendClaimRequestToObservers(
    ObservingClaimAppendRequestMessage::Shared request)
{
    info() << "sendClaimRequestToObservers";
    try {
        auto observingResponse = mObservingCommunicator->sendRequestToObserver(
                mObservers.front(),
                request);
        auto claimAppendResponse = make_shared<ObservingClaimAppendResponseMessage>(
                observingResponse);
        info() << "claimAppendResponse " << claimAppendResponse->observingResponse();

        if (claimAppendResponse->observingResponse() == ObservingTransaction::NoInfo) {
            // todo : try request to another observer
        }
        if (claimAppendResponse->observingResponse() == ObservingTransaction::ParticipantsVotesPresent) {
            // todo : get participants signatures
        }
    } catch (...) {
        this->warning() << "Can't send request to observer";
    }

    auto newClaim = make_shared<ObservingTransaction>(
        request,
        mObservers.front(),
        // todo : uncomment after implementing logic on observers
        /*claimAppendResponse->observingResponse()*/ObservingTransaction::ClaimInPool);

    mClaims.insert(make_pair(
        request->transactionUUID(),
        newClaim));
    if (mClaims.size() == 1) {
        rescheduleResending();
    }
}

void ObservingHandler::addTransactionForChecking(
    const TransactionUUID &transactionUUID,
    BlockNumber maxBlockNumberForClaiming)
{
    debug() << "addTransactionForChecking " << transactionUUID << " " << maxBlockNumberForClaiming;
    mCheckedTransactions.insert(
        make_pair(
            transactionUUID,
            maxBlockNumberForClaiming));
}

void ObservingHandler::requestActualBlockNumber(
    const TransactionUUID &transactionUUID)
{
    debug() << "requestActualBlockNumber for " << transactionUUID;
    mRequestsTimer.expires_from_now(
        std::chrono::milliseconds(
            5));
    mRequestsTimer.async_wait(
        boost::bind(
            &ObservingHandler::responseActualBlockNumber,
            this,
            transactionUUID));
}

void ObservingHandler::initialObservingRequest()
{
    debug() << "initialObservingRequest";
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &uncertainTransaction : ioTransaction->paymentTransactionsHandler()->transactionsWithUncertainObservingState()) {
        mCheckedTransactions.insert(
            uncertainTransaction);
    }
    debug() << "Checking transactions count " << mCheckedTransactions.size();

    if (!mCheckedTransactions.empty()) {
        mTransactionsTimer.expires_from_now(
            std::chrono::seconds(
                kInitialObservingRequestShiftSeconds));
        mTransactionsTimer.async_wait(
            boost::bind(
                &ObservingHandler::runTransactionsChecking,
                this,
                as::placeholders::error));
        return;
    }

    mTransactionsTimer.expires_from_now(
        std::chrono::seconds(
            kTransactionCheckingSignalRepeatTimeSeconds));
    mTransactionsTimer.async_wait(
        boost::bind(
            &ObservingHandler::runTransactionsChecking,
            this,
            as::placeholders::error));

    try {
        auto getActualBlockNumberMessage = make_shared<ObservingBlockNumberRequest>();
        auto observerResponse = mObservingCommunicator->sendRequestToObserver(
            mObservers.front(),
            getActualBlockNumberMessage);
        auto actualBlockNumberResponse = make_shared<ObservingBlockNumberResponse>(
            observerResponse);
        debug() << "Actual observing block number: " << actualBlockNumberResponse->actualBlockNumber();
        mLastUpdatedBlockNumber = make_pair(
            actualBlockNumberResponse->actualBlockNumber(),
            utc_now());
    } catch (...) {
        this->warning() << "Can't parse observers response";
    }
}

const DateTime ObservingHandler::closestClaimPerformingTimestamp() const
    noexcept
{
    if (mClaims.empty()) {
        return utc_now() + boost::posix_time::seconds(2);
    }

    DateTime nextClearingDateTime = mClaims.begin()->second->nextActionDateTime();
    for (const auto &claim : mClaims) {
        const auto kQueueNextAttemptPlanned = claim.second->nextActionDateTime();
        if (kQueueNextAttemptPlanned < nextClearingDateTime) {
            nextClearingDateTime = kQueueNextAttemptPlanned;
        }
    }
    return nextClearingDateTime;
}

void ObservingHandler::rescheduleResending()
{
    if (mClaims.empty()) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "There are no claims present. "
                         "Cleaning would not be scheduled any more.";
#endif

        return;
    }

    const auto kCleaningTimeout = closestClaimPerformingTimestamp() - utc_now();
    mClaimsTimer.expires_from_now(chrono::microseconds(kCleaningTimeout.total_microseconds()));
    mClaimsTimer.async_wait([this] (const boost::system::error_code &e) {

        if (e == boost::asio::error::operation_aborted) {
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Actions performing started.";
#endif

        this->performActions();
        this->rescheduleResending();

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Actions performing finished.";
#endif
    });
}

void ObservingHandler::performActions()
{
    const auto now = utc_now();

    for (const auto &claim : mClaims) {
        if (claim.second->nextActionDateTime() > now) {
            // This claim's timeout is not fired up yet.
            continue;
        }

        if (claim.second->observingResponseType() == ObservingTransaction::NoInfo or
                claim.second->observingResponseType() == ObservingTransaction::ParticipantsVotesPresent) {
            // todo : need correct reaction
        }
        try {
            if (performOneClaim(claim.second)) {
                mClaims.erase(claim.first);
            }
        } catch (...) {
            this->warning() << "Can't send request to observer";
        }
    }
}

bool ObservingHandler::performOneClaim(
    ObservingTransaction::Shared observingTransaction)
{
    debug() << "performOneClaim " << observingTransaction->transactionUUID();
    vector<pair<TransactionUUID, BlockNumber>> requestedTransactions;
    requestedTransactions.emplace_back(
        observingTransaction->transactionUUID(),
        observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber());
    auto claimCheck = make_shared<ObservingTransactionsRequestMessage>(
        requestedTransactions);

    auto observerResponse = mObservingCommunicator->sendRequestToObserver(
        mObservers.front(),
        claimCheck);
    auto actualTransactionStateResponse = make_shared<ObservingTransactionsResponseMessage>(
        observerResponse);
    // todo : update mLastUpdatedBlockNumber

    debug() << "Observer response cnt " << actualTransactionStateResponse->transactionsAndResponses().size()
            << " " << actualTransactionStateResponse->actualBlockNumber();
    // todo : check response size
    // todo : check transactionUUID
    auto transactionAndState = actualTransactionStateResponse->transactionsAndResponses().at(0);
    debug() << "Transaction state " << transactionAndState.first << " " << transactionAndState.second;
    if (transactionAndState.second == ObservingTransaction::NoInfo) {
        debug() << "NoInfo";
        // todo : need correct reaction
    } else if (transactionAndState.second == ObservingTransaction::ClaimInPool or
            transactionAndState.second == ObservingTransaction::ClaimInBlock) {
        debug() << "ClaimInBlock";
        observingTransaction->setObservingResponseType(transactionAndState.second);
        mLastUpdatedBlockNumber = make_pair(
            actualTransactionStateResponse->actualBlockNumber(),
            utc_now());
        if (mLastUpdatedBlockNumber.first > observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber()) {
            mRejectTransactionSignal(
                observingTransaction->transactionUUID(),
                observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber());
            auto ioTransaction = mStorageHandler->beginTransaction();
            ioTransaction->paymentTransactionsHandler()->updateTransactionState(
                observingTransaction->transactionUUID(),
                ObservingTransaction::RejectedByObserving);
            return true;
        }
    } else if (transactionAndState.second == ObservingTransaction::ParticipantsVotesPresent) {
        debug() << "ParticipantsVotesPresent";
        auto getTSLRequest = make_shared<ObservingParticipantsVotesRequestMessage>(
            observingTransaction->transactionUUID(),
            observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber());
        observerResponse = mObservingCommunicator->sendRequestToObserver(
            mObservers.front(),
            claimCheck);
        auto participantsVotesMessage = make_shared<ObservingParticipantsVotesResponseMessage>(
            observerResponse);
        // todo : check if participantsVotesMessage is correct
        mParticipantsVotesSignal(
            observingTransaction->transactionUUID(),
            observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber(),
            participantsVotesMessage->participantsSignatures());
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->paymentTransactionsHandler()->updateTransactionState(
            observingTransaction->transactionUUID(),
            ObservingTransaction::ParticipantsVotesPresent);
        return true;
    }
    observingTransaction->rescheduleNextActionTime();
    return false;
}

void ObservingHandler::runTransactionsChecking(
    const boost::system::error_code &errorCode)
{
    debug() << "runTransactionsChecking";
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    mTransactionsTimer.cancel();
    mTransactionsTimer.expires_from_now(
        std::chrono::seconds(
            kTransactionCheckingSignalRepeatTimeSeconds));
    mTransactionsTimer.async_wait(
        boost::bind(
            &ObservingHandler::runTransactionsChecking,
            this,
            as::placeholders::error));

    if (mCheckedTransactions.empty()) {
        return;
    }

    vector<pair<TransactionUUID, BlockNumber>> checkedTransactions;
    for (const auto &transaction : mCheckedTransactions) {
        checkedTransactions.emplace_back(transaction.first, transaction.second);
    }
    auto transactionsRequestMessage = make_shared<ObservingTransactionsRequestMessage>(
        checkedTransactions);
    BytesShared observerResponse;
    try {
        observerResponse = mObservingCommunicator->sendRequestToObserver(
            mObservers.front(),
            transactionsRequestMessage);
    } catch (...) {
        this->warning() << "Can't send request to observers";
        return;
    }
    auto transactionsResponse = make_shared<ObservingTransactionsResponseMessage>(
        observerResponse);
    if (transactionsResponse->transactionsAndResponses().size() != checkedTransactions.size()) {
        // todo : need correct reaction
    }
    debug() << "Observer response cnt " << transactionsResponse->transactionsAndResponses().size()
            << " " << transactionsResponse->actualBlockNumber();

    size_t idxProcessedTransaction = 0;
    for (const auto &responseTransaction : transactionsResponse->transactionsAndResponses()) {
        debug() << "Process " << responseTransaction.first << " " << responseTransaction.second;
        auto processedTransaction = checkedTransactions.at(idxProcessedTransaction++).first;
        debug() << "Processed transaction " << processedTransaction;
        // todo : process transaction
        switch (responseTransaction.second) {
            case ObservingTransaction::NoInfo: {
                debug() << "NoInfo";
                // if claiming time has expired remove transaction from claiming map
                if (mCheckedTransactions[processedTransaction] < transactionsResponse->actualBlockNumber()) {
                    debug() << "Claim time has expired";
                    mCheckedTransactions.erase(
                        processedTransaction);
                    auto ioTransaction = mStorageHandler->beginTransaction();
                    ioTransaction->paymentTransactionsHandler()->updateTransactionState(
                        processedTransaction,
                        ObservingTransaction::ClaimTimeExpired);
                }
                break;
            }
            case ObservingTransaction::ClaimInPool: {
                // todo: need correct reaction
                break;
            }
            case ObservingTransaction::ClaimInBlock: {
                debug() << "Claim in block";
                auto ioTransaction = mStorageHandler->beginTransaction();
                auto participantsSignatures = ioTransaction->paymentParticipantsVotesHandler()->participantsSignatures(
                    processedTransaction);
                if (participantsSignatures.empty()) {
                    // todo : need correct reaction
                }
                auto participantsVotesAppendRequest = make_shared<ObservingParticipantsVotesAppendRequestMessage>(
                    processedTransaction,
                    mCheckedTransactions[processedTransaction],
                    participantsSignatures);
                try {
                    observerResponse = mObservingCommunicator->sendRequestToObserver(
                        mObservers.front(),
                        participantsVotesAppendRequest);
                } catch (...) {
                    this->warning() << "Can't send request to observers";
                    // todo : try another observer
                    break;
                }
                auto participantsVotesAppendResponse= make_shared<ObservingParticipantsVotesAppendResponseMessage>(
                    observerResponse);
                // todo : process participantsVotesAppendResponse
                break;
            }
            case ObservingTransaction::ParticipantsVotesPresent: {
                debug() << "ParticipantsVotesPresent";
                mCheckedTransactions.erase(
                    processedTransaction);
                // todo : check if TSL correct
                auto ioTransaction = mStorageHandler->beginTransaction();
                ioTransaction->paymentTransactionsHandler()->updateTransactionState(
                    processedTransaction,
                    ObservingTransaction::ParticipantsVotesPresent);
                break;
            }

            default: {
                this->warning() << "Invalid type of observing response type "
                                << processedTransaction;
            }
        }
    }

    mLastUpdatedBlockNumber = make_pair(
        transactionsResponse->actualBlockNumber(),
        utc_now());
    debug() << "Actual observing block number: " << transactionsResponse->actualBlockNumber();
}

void ObservingHandler::responseActualBlockNumber(
    const TransactionUUID &transactionUUID)
{
    debug() << "responseActualBlockNumber " << transactionUUID;
    mRequestsTimer.cancel();
    Duration durationWithoutBlockNumberUpdating = utc_now() - mLastUpdatedBlockNumber.second;
    if (durationWithoutBlockNumberUpdating > kBlockNumberUpdateDuration()) {
        auto getActualBlockNumberMessage = make_shared<ObservingBlockNumberRequest>();
        auto observerResponse = mObservingCommunicator->sendRequestToObserver(
            mObservers.front(),
            getActualBlockNumberMessage);
        try {
            auto actualBlockNumberResponse = make_shared<ObservingBlockNumberResponse>(
                observerResponse);
            mResourcesManager->putResource(
                make_shared<BlockNumberRecourse>(
                    transactionUUID,
                    actualBlockNumberResponse->actualBlockNumber()));
            debug() << "Actual observing block number: " << actualBlockNumberResponse->actualBlockNumber();
            mLastUpdatedBlockNumber = make_pair(
                actualBlockNumberResponse->actualBlockNumber(),
                utc_now());
        } catch (...) {
            this->warning() << "Can't parse observers response";
        }
        return;
    }

    BlockNumber actualBlockNumber = mLastUpdatedBlockNumber.first +
            durationWithoutBlockNumberUpdating.seconds() / kApproximateBlockNumberIncrementingPeriodSeconds;
    debug() << "Calculated observing block number: " << actualBlockNumber;
    mResourcesManager->putResource(
        make_shared<BlockNumberRecourse>(
            transactionUUID,
            actualBlockNumber));
}

const string ObservingHandler::logHeader() const
    noexcept
{
    return "[ObservingHandler]";
}