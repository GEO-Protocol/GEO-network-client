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
    mBlockNumberRequestTimer(ioService),
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
                throw ValueError("ObservingHandler: can't create observer address of type " + addressStr.first);
            }

        } else {
            throw ValueError("ObservingHandler: can't create observer address. "
                                 "Wrong address type " + addressStr.first);
        }
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &uncertainTransaction : ioTransaction->paymentTransactionsHandler()->transactionsWithUncertainObservingState()) {
        if (ioTransaction->transactionHandler()->isTransactionSerialized(uncertainTransaction.first)) {
            warning() << "There is serialized transaction " << uncertainTransaction.first << " with uncertain state";
            continue;
        }
        mCheckedTransactions.insert(
            uncertainTransaction);
    }
    debug() << "Checking transactions count " << mCheckedTransactions.size();

    mBlockNumberRequestTimer.expires_from_now(
        std::chrono::seconds(
            +kInitialObservingRequestShiftSeconds));
    mBlockNumberRequestTimer.async_wait(
        boost::bind(
            &ObservingHandler::initialObservingRequest,
            this));
}

void ObservingHandler::sendClaimRequestToObservers(
    ObservingClaimAppendRequestMessage::Shared request)
{
    info() << "sendClaimRequestToObservers " << request->transactionUUID()
           << " maximalBlockN " << request->maximalClaimingBlockNumber();
    auto newClaim = make_shared<ObservingTransaction>(
        request);
    mClaims.insert(make_pair(
        request->transactionUUID(),
        newClaim));

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->paymentTransactionsHandler()->updateTransactionState(
        request->transactionUUID(),
        ObservingTransaction::ClaimInPool);

    for (const auto &observer : mObservers) {
        BytesShared observingResponse;
        try {
            observingResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                request);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observer " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }
        try {
            auto claimAppendResponse = make_shared<ObservingClaimAppendResponseMessage>(
                observingResponse);
            info() << "claimAppendResponse " << claimAppendResponse->observingResponse();

            if (claimAppendResponse->observingResponse() == ObservingTransaction::NoInfo) {
                continue;
            }
            // if claim period has expired, then transaction should stay serialized and hold reservations forever
            if (claimAppendResponse->observingResponse() == ObservingTransaction::ClaimTimeExpired) {
                info() << "Claim period has expired";
                mUncertainTransactionSignal(
                    request->transactionUUID(),
                    request->maximalClaimingBlockNumber());
                mClaims.erase(request->transactionUUID());
                return;
            }

            newClaim->addRequestedObserver(
                observer);
            newClaim->setObservingResponseType(
                ObservingTransaction::ClaimInPool);
            if (mClaims.size() == 1) {
                rescheduleResending();
            }
            return;
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
    }

    warning() << "Can't send claim to all observers";
    newClaim->rescheduleNextActionSmallTime();
    if (mClaims.size() == 1) {
        rescheduleResending();
    }
}

void ObservingHandler::addTransactionForChecking(
    const TransactionUUID &transactionUUID,
    BlockNumber maxBlockNumberForClaiming)
{
#ifdef DEBUG_LOG_OBSEVING_HANDLER
    debug() << "addTransactionForChecking " << transactionUUID << " " << maxBlockNumberForClaiming;
#endif
    mCheckedTransactions.insert(
        make_pair(
            transactionUUID,
            maxBlockNumberForClaiming));
}

void ObservingHandler::requestActualBlockNumber(
    const TransactionUUID &transactionUUID)
{
#ifdef DEBUG_LOG_OBSEVING_HANDLER
    debug() << "requestActualBlockNumber for " << transactionUUID;
#endif
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
#ifdef DEBUG_LOG_OBSEVING_HANDLER
    debug() << "initialObservingRequest";
#endif
    mBlockNumberRequestTimer.cancel();

    auto getActualBlockNumberMessage = make_shared<ObservingBlockNumberRequest>();
    BytesShared observerResponse;

    for (const auto &observer : mObservers) {
        try {
            observerResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                getActualBlockNumberMessage);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observer " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }

        try {
            auto actualBlockNumberResponse = make_shared<ObservingBlockNumberResponse>(
                observerResponse);

            info() << "Actual observing block number: " << actualBlockNumberResponse->actualBlockNumber();
            mLastUpdatedBlockNumber = make_pair(
                actualBlockNumberResponse->actualBlockNumber(),
                utc_now());

            mAllowPaymentTransactionsSignal(true);
            mTransactionsTimer.expires_from_now(
                std::chrono::seconds(
                    +kInitialObservingRequestShiftSeconds));
            mTransactionsTimer.async_wait(
                boost::bind(
                    &ObservingHandler::runTransactionsChecking,
                    this,
                    as::placeholders::error));
            return;
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
    }

    warning() << "Can't get actual block number from all observers";
    mBlockNumberRequestTimer.expires_from_now(
        std::chrono::seconds(
            +kInitialObservingRequestNextSeconds));
    mBlockNumberRequestTimer.async_wait(
        boost::bind(
            &ObservingHandler::initialObservingRequest,
            this));
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

#ifdef DEBUG_LOG_OBSEVING_HANDLER
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

#ifdef DEBUG_LOG_OBSEVING_HANDLER
        this->debug() << "Actions performing started.";
#endif

        this->performActions();
        this->rescheduleResending();

#ifdef DEBUG_LOG_OBSEVING_HANDLER
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

        if (claim.second->observingResponseType() == ObservingTransaction::ParticipantsVotesPresent) {
            // todo : need correct reaction
        }
        if (performOneClaim(claim.second)) {
            mClaims.erase(claim.first);
        }
    }
}

bool ObservingHandler::performOneClaim(
    ObservingTransaction::Shared observingTransaction)
{
    debug() << "performOneClaim " << observingTransaction->transactionUUID()
            << " type " << observingTransaction->observingResponseType()
            << " maximalBlockN " << observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber();
    if (observingTransaction->observingResponseType() == ObservingTransaction::NoInfo) {
        sendClaimAgain(
            observingTransaction);
        return false;
    }

    vector<pair<TransactionUUID, BlockNumber>> requestedTransactions;
    requestedTransactions.emplace_back(
        observingTransaction->transactionUUID(),
        observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber());
    auto claimCheck = make_shared<ObservingTransactionsRequestMessage>(
        requestedTransactions);

    BytesShared observerResponse;
    for (const auto &observer : mObservers) {
        if (mObservers.size() > 1 and observer == observingTransaction->requestedObserver()) {
            // we check if claim in block on all observers except those which accepted claim
            continue;
        }
        try {
            observerResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                claimCheck);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observer " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }
        ObservingTransaction::ObservingResponseType observingResponseType;
        try {
            auto actualTransactionStateResponse = make_shared<ObservingTransactionsResponseMessage>(
                observerResponse);
            mLastUpdatedBlockNumber = make_pair(
                actualTransactionStateResponse->actualBlockNumber(),
                utc_now());

            debug() << "Actual block number " << actualTransactionStateResponse->actualBlockNumber();
            debug() << "Observer response " << actualTransactionStateResponse->transactionsResponses().size();
            if (actualTransactionStateResponse->transactionsResponses().size() > 1) {
                warning() << "Size of received transactions is invalid";
                continue;
            }
            observingResponseType = actualTransactionStateResponse->transactionsResponses().at(0);
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
#ifdef DEBUG_LOG_OBSEVING_HANDLER
        debug() << "Transaction state " << observingResponseType;
#endif
        if (observingResponseType == ObservingTransaction::NoInfo) {
            debug() << "No Info";
            // todo : need correct reaction
        } else if (observingResponseType == ObservingTransaction::ClaimInPool or
                observingResponseType == ObservingTransaction::ClaimInBlock) {
            info() << "ClaimInBlock";
            observingTransaction->setObservingResponseType(observingResponseType);
            if (mLastUpdatedBlockNumber.first >
                observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber()) {
                info() << "Claiming time has expired, transaction rejected";
                mRejectTransactionSignal(
                    observingTransaction->transactionUUID(),
                    observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber());
                auto ioTransaction = mStorageHandler->beginTransaction();
                ioTransaction->paymentTransactionsHandler()->updateTransactionState(
                    observingTransaction->transactionUUID(),
                    ObservingTransaction::RejectedByObserving);
                return true;
            }
            observingTransaction->rescheduleNextActionTime();
            return false;
        } else if (observingResponseType == ObservingTransaction::ParticipantsVotesPresent) {
            info() << "ParticipantsVotesPresent";
            if (!getParticipantsVotes(
                    observingTransaction->transactionUUID(),
                    observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber())) {
                observingTransaction->rescheduleNextActionSmallTime();
                return false;
            }
            return true;
        } else {
            warning() << "Unexpected transaction state " << observingResponseType;
            continue;
        }
    }

    warning() << "Can't send request to all observers";
    observingTransaction->rescheduleNextActionSmallTime();
    return false;
}

void ObservingHandler::sendClaimAgain(
    ObservingTransaction::Shared observingTransaction)
{
    debug() << "sendClaimAgain " << observingTransaction->transactionUUID();
    for (const auto &observer : mObservers) {
        BytesShared observingResponse;
        try {
            observingResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                observingTransaction->observingRequestMessage());
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observer " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }
        try {
            auto claimAppendResponse = make_shared<ObservingClaimAppendResponseMessage>(
                observingResponse);
            info() << "claimAppendResponse " << claimAppendResponse->observingResponse();

            if (claimAppendResponse->observingResponse() == ObservingTransaction::NoInfo) {
                continue;
            }
            // if claim period has expired, then transaction should stay serialized and hold reservations forever
            if (claimAppendResponse->observingResponse() == ObservingTransaction::ClaimTimeExpired) {
                info() << "Claim period has expired";
                mUncertainTransactionSignal(
                    observingTransaction->transactionUUID(),
                    observingTransaction->observingRequestMessage()->maximalClaimingBlockNumber());
                mClaims.erase(observingTransaction->transactionUUID());
                return;
            }

            observingTransaction->addRequestedObserver(
                observer);
            observingTransaction->setObservingResponseType(
                ObservingTransaction::ClaimInPool);
            observingTransaction->rescheduleNextActionTime();
            return;
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
    }

    warning() << "Can't send claim to all observers";
    observingTransaction->rescheduleNextActionSmallTime();
}

bool ObservingHandler::getParticipantsVotes(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    info() << "getParticipantsVotes " << transactionUUID;
    auto getTSLRequest = make_shared<ObservingParticipantsVotesRequestMessage>(
        transactionUUID,
        maximalClaimingBlockNumber);
    for (const auto &observer : mObservers) {
        BytesShared observerResponse;
        try {
            observerResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                getTSLRequest);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observer " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }
        try {
            auto participantsVotesMessage = make_shared<ObservingParticipantsVotesResponseMessage>(
                observerResponse);
            if (!participantsVotesMessage->isParticipantsVotesPresent()) {
                warning() << "ParticipantsVotes are absent";
                continue;
            }
            info() << "Receive participants votes " << participantsVotesMessage->transactionUUID() << " "
                    << participantsVotesMessage->maximalClaimingBlockNumber() << " "
                    << participantsVotesMessage->participantsSignatures().size();
            // todo : check if participantsVotesMessage is correct
            mParticipantsVotesSignal(
                transactionUUID,
                maximalClaimingBlockNumber,
                participantsVotesMessage->participantsSignatures());
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->paymentTransactionsHandler()->updateTransactionState(
            transactionUUID,
            ObservingTransaction::ParticipantsVotesPresent);
        return true;
    }

    warning() << "Can't get ParticipantsVotes from all observers";
    return false;
}

void ObservingHandler::runTransactionsChecking(
    const boost::system::error_code &errorCode)
{
#ifdef DEBUG_LOG_OBSEVING_HANDLER
    debug() << "runTransactionsChecking";
#endif
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    mTransactionsTimer.cancel();

    if (mCheckedTransactions.empty()) {
        mTransactionsTimer.expires_from_now(
            std::chrono::seconds(
                +kTransactionCheckingSignalRepeatTimeSeconds));
        mTransactionsTimer.async_wait(
            boost::bind(
                &ObservingHandler::runTransactionsChecking,
                this,
                as::placeholders::error));
        return;
    }

    vector<pair<TransactionUUID, BlockNumber>> checkedTransactions;
    for (const auto &transaction : mCheckedTransactions) {
        checkedTransactions.emplace_back(transaction.first, transaction.second);
    }
    auto transactionsRequestMessage = make_shared<ObservingTransactionsRequestMessage>(
        checkedTransactions);
    BytesShared observerResponse;
    for (const auto &observer : mObservers) {
        try {
            observerResponse = mObservingCommunicator->sendRequestToObserver(
                mObservers.front(),
                transactionsRequestMessage);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observers " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }
        ObservingTransactionsResponseMessage::Shared transactionsResponse;
        try {
            transactionsResponse = make_shared<ObservingTransactionsResponseMessage>(
                observerResponse);
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
        mLastUpdatedBlockNumber = make_pair(
            transactionsResponse->actualBlockNumber(),
            utc_now());
#ifdef DEBUG_LOG_OBSEVING_HANDLER
        debug() << "Actual observing block number: " << transactionsResponse->actualBlockNumber();
#endif
        if (transactionsResponse->transactionsResponses().size() != checkedTransactions.size()) {
            warning() << "Received data contains wrong number of transaction "
                      << transactionsResponse->transactionsResponses().size();
            continue;
        }
#ifdef DEBUG_LOG_OBSEVING_HANDLER
        debug() << "Observer response cnt " << transactionsResponse->transactionsResponses().size();
#endif

        auto transactionCheckingSignalRepeatTimeSeconds = kTransactionCheckingSignalRepeatTimeSeconds;
        size_t idxProcessedTransaction = 0;
        for (const auto &responseTransaction : transactionsResponse->transactionsResponses()) {
            auto processedTransaction = checkedTransactions.at(idxProcessedTransaction++).first;
#ifdef DEBUG_LOG_OBSEVING_HANDLER
            debug() << "Processed transaction " << processedTransaction << " with response " << responseTransaction;
#endif
            switch (responseTransaction) {
                case ObservingTransaction::NoInfo: {
#ifdef DEBUG_LOG_OBSEVING_HANDLER
                    debug() << "NoInfo";
#endif
                    // if claiming time has expired remove transaction from claiming map
                    if (mCheckedTransactions[processedTransaction] < transactionsResponse->actualBlockNumber()) {
                        info() << "Claim time has expired for " << processedTransaction;
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
                    info() << "Claim in pool for " << processedTransaction;
                    // todo: need correct reaction because observer get info about claim, when it on the blackchain
                    if (!sendParticipantsVoteMessageToObservers(
                        processedTransaction,
                        mCheckedTransactions[processedTransaction])) {
                        transactionCheckingSignalRepeatTimeSeconds = kTransactionCheckingSignalSmallRepeatTimeSeconds;
                    }
                    break;
                }
                case ObservingTransaction::ClaimInBlock: {
                    info() << "Claim in block for " << processedTransaction;
                    if (mCheckedTransactions[processedTransaction] < transactionsResponse->actualBlockNumber()) {
                        info() << "Transaction was rejected by observing. Cancelling.";
                        mCancelTransactionSignal(
                            processedTransaction,
                            mCheckedTransactions[processedTransaction]);
                        mCheckedTransactions.erase(
                            processedTransaction);
                        auto ioTransaction = mStorageHandler->beginTransaction();
                        ioTransaction->paymentTransactionsHandler()->updateTransactionState(
                            processedTransaction,
                            ObservingTransaction::RejectedByObserving);
                        break;
                    }
                    if (!sendParticipantsVoteMessageToObservers(
                        processedTransaction,
                        mCheckedTransactions[processedTransaction])) {
                        transactionCheckingSignalRepeatTimeSeconds = kTransactionCheckingSignalSmallRepeatTimeSeconds;
                    }
                    break;
                }
                case ObservingTransaction::ParticipantsVotesPresent: {
                    info() << "ParticipantsVotesPresent for " << processedTransaction;
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
                    continue;
                }
            }
        }

        mTransactionsTimer.expires_from_now(
            std::chrono::seconds(
                transactionCheckingSignalRepeatTimeSeconds));
        mTransactionsTimer.async_wait(
            boost::bind(
                &ObservingHandler::runTransactionsChecking,
                this,
                as::placeholders::error));
        return;
    }

    warning() << "Can't send request to all observers";
    mTransactionsTimer.expires_from_now(
        std::chrono::seconds(
            +kTransactionCheckingSignalSmallRepeatTimeSeconds));
    mTransactionsTimer.async_wait(
        boost::bind(
            &ObservingHandler::runTransactionsChecking,
            this,
            as::placeholders::error));
}

bool ObservingHandler::sendParticipantsVoteMessageToObservers(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    info() << "sendParticipantsVoteMessageToObservers " << transactionUUID << " " << maximalClaimingBlockNumber;
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto participantsSignatures = ioTransaction->paymentParticipantsVotesHandler()->participantsSignatures(
        transactionUUID);
    if (participantsSignatures.empty()) {
        warning() << "Empty participants signatures";
        // todo : need correct reaction
        return false;
    }
    auto participantsVotesAppendRequest = make_shared<ObservingParticipantsVotesAppendRequestMessage>(
        transactionUUID,
        maximalClaimingBlockNumber,
        participantsSignatures);
    BytesShared observerResponse;
    for (const auto &observer : mObservers) {
        try {
            observerResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                participantsVotesAppendRequest);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observers " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }
        try {
            auto participantsVotesAppendResponse = make_shared<ObservingParticipantsVotesAppendResponseMessage>(
                observerResponse);
            info() << "participantsVotesAppendResponse " << participantsVotesAppendResponse->observingResponse();
            if (participantsVotesAppendResponse->observingResponse() == ObservingTransaction::ClaimInPool or
                    participantsVotesAppendResponse->observingResponse() == ObservingTransaction::ClaimInBlock) {
                info() << "ParticipantsVotes put into blockchain";
                return true;
            } else if (participantsVotesAppendResponse->observingResponse() == ObservingTransaction::NoInfo) {
                info() << "No Info";
                continue;
            } else if (participantsVotesAppendResponse->observingResponse() == ObservingTransaction::RejectedByObserving) {
                info() << "RejectedByObserving";
                // if ParticipantsVotes sending period has expired,
                // then node should check if transaction is not rejected
                // and if yes reject transaction on it side
                mCancelTransactionSignal(
                    transactionUUID,
                    maximalClaimingBlockNumber);
                return true;
            } else {
                warning() << "Wrong observer response " << participantsVotesAppendResponse->observingResponse();
                continue;
            }
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
    }

    this->warning() << "Can't send ParticipantsVotesMessage to all observers";
    return false;
}

void ObservingHandler::responseActualBlockNumber(
    const TransactionUUID &transactionUUID)
{
#ifdef DEBUG_LOG_OBSEVING_HANDLER
    debug() << "responseActualBlockNumber " << transactionUUID;
#endif
    mRequestsTimer.cancel();
    Duration durationWithoutBlockNumberUpdating = utc_now() - mLastUpdatedBlockNumber.second;
    if (durationWithoutBlockNumberUpdating > kBlockNumberUpdateDuration()) {
        auto getActualBlockNumberMessage = make_shared<ObservingBlockNumberRequest>();
        BytesShared observerResponse;
        for (const auto &observer : mObservers) {
            try {
                observerResponse = mObservingCommunicator->sendRequestToObserver(
                    observer,
                    getActualBlockNumberMessage);
            } catch (std::exception &e) {
                this->warning() << "Can't send request to observer " << e.what();
                continue;
            } catch (...) {
                this->warning() << "Can't send request to observer";
                continue;
            }

            try {
                auto actualBlockNumberResponse = make_shared<ObservingBlockNumberResponse>(
                    observerResponse);
                mResourcesManager->putResource(
                    make_shared<BlockNumberRecourse>(
                        transactionUUID,
                        actualBlockNumberResponse->actualBlockNumber()));
                info() << "Actual observing block number: " << actualBlockNumberResponse->actualBlockNumber();
                mLastUpdatedBlockNumber = make_pair(
                    actualBlockNumberResponse->actualBlockNumber(),
                    utc_now());
                return;
            } catch (std::exception &e) {
                this->warning() << "Can't parse observer response " << e.what();
                continue;
            }
        }
        warning() << "Can't send request to all observers";
        mAllowPaymentTransactionsSignal(false);
        mBlockNumberRequestTimer.expires_from_now(
            std::chrono::seconds(
                +kInitialObservingRequestNextSeconds));
        mBlockNumberRequestTimer.async_wait(
            boost::bind(
                &ObservingHandler::getActualBlockNumber,
                this));
        // if node can't get actual block number ObservingHandler doesn't inform requested transaction
        // so it will be rejected
        return;
    }

    BlockNumber actualBlockNumber = mLastUpdatedBlockNumber.first +
            durationWithoutBlockNumberUpdating.seconds() / kApproximateBlockNumberIncrementingPeriodSeconds;
    info() << "Calculated observing block number: " << actualBlockNumber;
    mResourcesManager->putResource(
        make_shared<BlockNumberRecourse>(
            transactionUUID,
            actualBlockNumber));
}

void ObservingHandler::getActualBlockNumber()
{
#ifdef DEBUG_LOG_OBSEVING_HANDLER
    debug() << "getActualBlockNumber";
#endif
    mBlockNumberRequestTimer.cancel();

    auto getActualBlockNumberMessage = make_shared<ObservingBlockNumberRequest>();
    BytesShared observerResponse;

    for (const auto &observer : mObservers) {
        try {
            observerResponse = mObservingCommunicator->sendRequestToObserver(
                observer,
                getActualBlockNumberMessage);
        } catch (std::exception &e) {
            this->warning() << "Can't send request to observer " << e.what();
            continue;
        } catch (...) {
            this->warning() << "Can't send request to observer";
            continue;
        }

        try {
            auto actualBlockNumberResponse = make_shared<ObservingBlockNumberResponse>(
                observerResponse);

            info() << "Actual observing block number: " << actualBlockNumberResponse->actualBlockNumber();
            mLastUpdatedBlockNumber = make_pair(
                actualBlockNumberResponse->actualBlockNumber(),
                utc_now());

            mAllowPaymentTransactionsSignal(true);
            return;
        } catch (std::exception &e) {
            this->warning() << "Can't parse observer response " << e.what();
            continue;
        }
    }

    warning() << "Can't get actual block number from all observers";
    mBlockNumberRequestTimer.expires_from_now(
        std::chrono::seconds(
            +kInitialObservingRequestNextSeconds));
    mBlockNumberRequestTimer.async_wait(
        boost::bind(
            &ObservingHandler::getActualBlockNumber,
            this));
}

const string ObservingHandler::logHeader() const
    noexcept
{
    return "[ObservingHandler]";
}