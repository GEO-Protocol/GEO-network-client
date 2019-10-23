#include "ObservingHandler.h"

ObservingHandler::ObservingHandler(
    vector<pair<string, string>> observersAddressesStr,
    IOService &ioService,
    StorageHandler *storageHandler,
    ResourcesManager *resourcesManager,
    Logger &logger) :
    LoggerMixin(logger),
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
{}

void ObservingHandler::sendClaimRequestToObservers(
    ObservingClaimAppendRequestMessage::Shared request)
{}

void ObservingHandler::addTransactionForChecking(
    const TransactionUUID &transactionUUID,
    BlockNumber maxBlockNumberForClaiming)
{}

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

    mResourcesManager->putResource(
        make_shared<BlockNumberRecourse>(
            transactionUUID,
            kDefaultBlockNumber));
}

void ObservingHandler::getActualBlockNumber()
{}

const string ObservingHandler::logHeader() const
{
    return "[ObservingHandler]";
}