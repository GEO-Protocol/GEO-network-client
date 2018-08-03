#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :

    mIOService(IOService),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mLog(logger),

    mTransactions(new map<BaseTransaction::Shared, TransactionState::SharedConst>()),
    mProcessingTimer(new as::steady_timer(mIOService))
{}

/*!
 * Schedules awakening for the next transaction.
 * Doesn't blocks execution.
 */
void TransactionsScheduler::run()
{
    if (mTransactions->empty()) {
        // If no transactions are present -
        // there is no reason to reschedule next interruption:
        // no transactions would be launched
        // (new transaction can only be launched
        // via network message or user command).
        return;
    }

    // WARN:
    // No one transaction should be launched directly from this method:
    // there is no any guarantee that the rest system components are ready.
    //
    // (initialisation order:
    // transactions manager may be initialised before other system components)
    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::scheduleTransaction(
    BaseTransaction::Shared transaction)
{
    for (const auto &transactionAndState : *mTransactions){
        if (transaction->currentTransactionUUID() == transactionAndState.first->currentTransactionUUID()) {
            warning() << "scheduleTransaction: Duplicate TransactionUUID. Already exists. "
                      << "Current TA type: " << transaction->transactionType()
                      << ". Conflicted TA type:" << transactionAndState.first->transactionType();
            throw ConflictError("Duplicate Transaction UUID");
        }
    }
    (*mTransactions)[transaction] = TransactionState::awakeAsFastAsPossible();

    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::postponeTransaction(
    BaseTransaction::Shared transaction,
    uint32_t millisecondsDelay)
{
    (*mTransactions)[transaction] = TransactionState::awakeAfterMilliseconds(millisecondsDelay);

    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::tryAttachMessageToTransaction(
    Message::Shared message)
{
    if (!message->isTransactionMessage()) {
        throw ValueError(
            "TransactionsScheduler::tryAttachMessageToTransaction: "
                "message received is not TransactionMessage " + to_string(message->typeID()));
    }

    auto transactionMessage = static_pointer_cast<TransactionMessage>(message);
    for (auto const &transactionAndState : *mTransactions) {

        if (transactionMessage->transactionUUID() != transactionAndState.first->currentTransactionUUID()) {
            continue;
        }

        for (auto const &messageType : transactionAndState.second->acceptedMessagesTypes()) {
            if (message->typeID() != messageType) {
                continue;
            }

            // filtering TTL response messages for payment TAs
            // this messages can send only transaction coordinator
            // in future if such cases will be more this code should make separate method
            if (message->typeID() == Message::Payments_TTLProlongationResponse or
                    message->typeID() == Message::Payments_FinalAmountsConfiguration or
                    message->typeID() == Message::Payments_FinalPathConfiguration) {
                auto paymentTransaction = static_pointer_cast<BasePaymentTransaction>(
                    transactionAndState.first);
                if (paymentTransaction->coordinatorUUID() != transactionMessage->senderUUID) {
                    continue;
                }
            }

            transactionAndState.first->pushContext(message);
            if (transactionAndState.second->mustBeAwakenedOnMessage()) {
                launchTransaction(transactionAndState.first);
            }
            return;
        }
    }
    throw NotFoundError(
        "TransactionsScheduler::tryAttachMessageToTransaction: " +
            transactionMessage->transactionUUID().stringUUID() +
            " invalid/unexpected message/response received " +
            to_string(message->typeID()));
}

void TransactionsScheduler::tryAttachResourceToTransaction(
    BaseResource::Shared resource)
{
    for (const auto& transactionAndState : *mTransactions) {

        if (resource->transactionUUID() != transactionAndState.first->currentTransactionUUID()) {
            continue;
        }

        for (const auto &resType : transactionAndState.second->acceptedResourcesTypes()) {
            if (resource->type() != resType) {
                continue;
            }

            transactionAndState.first->pushResource(resource);
        }

        launchTransaction(transactionAndState.first);
        return;

    }

    throw NotFoundError(
        "TransactionsScheduler::tryAttachResourceToTransaction: "
            "Can't find transaction that requires given resource.");
}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction)
{
#ifdef TESTS
    if (mTrustLinesInfluenceController->isTerminateProcessOnScheduler()) {
        debug() << "terminateProcessOnScheduler";
        exit(100);
    }
#endif
    try {
        const auto kTAType = transaction->transactionType();
        if (kTAType >= BaseTransaction::CoordinatorPaymentTransaction
            && kTAType <= BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction) {

            info() << "Payment or cycle closing TA launched:"
                << " UUID: " << transaction->currentTransactionUUID()
                << " Type: " << transaction->transactionType()
                << " Step: " << transaction->currentStep();
        }


        // Even if transaction will raise an exception -
        // it must not be thrown up,
        // to not to break transactions processing flow.
        auto result = transaction->run();
        if (result == nullptr) {
            throw ValueError(
                "TransactionsScheduler::launchTransaction: "
                "transaction->run() returned nullptr result.");
        }

        handleTransactionResult(
            transaction,
            result);
        adjustAwakeningToNextTransaction();

    } catch (exception &e) {
        error() << "TA error occurred:"
            << " UUID: " << transaction->currentTransactionUUID()
            << " Type: " << transaction->transactionType()
            << " Step: " << transaction->currentStep()
            << " Error message: " << e.what()
            << " Transaction dropped";

        forgetTransaction(transaction);
    }
}

void TransactionsScheduler::handleTransactionResult(
    BaseTransaction::Shared transaction,
    TransactionResult::SharedConst result)
{
    switch (result->resultType()) {
        case TransactionResult::ResultType::CommandResultType: {
            processCommandResult(
                transaction,
                result->commandResult());
            break;
        }

        case TransactionResult::ResultType::TransactionStateType: {
            processTransactionState(
                transaction,
                result->state());
            break;
        }
        case TransactionResult::ResultType::HybridType: {
            processCommandResultAndTransactionState(
                transaction,
                result->commandResult(),
                result->state());
            break;
        }
    }
}

void TransactionsScheduler::processCommandResult(
    BaseTransaction::Shared transaction,
    CommandResult::SharedConst result)
{
    commandResultIsReadySignal(result);
    forgetTransaction(transaction);
}

void TransactionsScheduler::processTransactionState(
    BaseTransaction::Shared transaction,
    TransactionState::SharedConst state)
{
    if (state->mustSavePreviousStateState()) {
        return;
    }
    if (state->mustBeRescheduled()){
        if (state->needSerialize())
            serializeTransactionSignal(transaction);

        // From the C++ reference:
        //
        // std::map::insert:
        // ... Because element keys in a map are unique,
        // the insertion operation checks whether each inserted element has a key
        // equivalent to the one of an element already in the container, and if so,
        // the element is NOT INSERTED, ...
        //
        // So the [] operator must be used
        (*mTransactions)[transaction] = state;

    } else {
        forgetTransaction(transaction);
    }
}

void TransactionsScheduler::processCommandResultAndTransactionState(
    BaseTransaction::Shared transaction,
    CommandResult::SharedConst result,
    TransactionState::SharedConst state)
{
    commandResultIsReadySignal(result);
    processTransactionState(
        transaction,
        state);
}

void TransactionsScheduler::forgetTransaction(
    BaseTransaction::Shared transaction)
{
    const auto kTAType = transaction->transactionType();
    if (kTAType >= BaseTransaction::CoordinatorPaymentTransaction
        && kTAType <= BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction) {

        info() << "Payment or cycle closing TA has been forgotten:"
                << " UUID: " << transaction->currentTransactionUUID()
                << " Type: " << transaction->transactionType()
                << " Step: " << transaction->currentStep();
    }

    if (transaction->transactionType() == BaseTransaction::Payments_CycleCloserInitiatorTransaction) {
        cycleCloserTransactionWasFinishedSignal(
            static_pointer_cast<BasePaymentTransaction>(
                transaction)->equivalent());
    }
    mTransactions->erase(transaction);
}

void TransactionsScheduler::adjustAwakeningToNextTransaction()
{
    try {
        asyncWaitUntil(
            transactionWithMinimalAwakeningTimestamp().second);

    } catch (NotFoundError &e) {}
}

/*!
 *
 * Throws NotFoundError in case if no transactions are delayed.
 */
pair<BaseTransaction::Shared, GEOEpochTimestamp> TransactionsScheduler::transactionWithMinimalAwakeningTimestamp() const
{
    if (mTransactions->empty()) {
        throw NotFoundError(
            "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
                "There are no any delayed transactions.");
    }

    auto nextTransactionAndState = mTransactions->cbegin();
    if (mTransactions->size() > 1) {
        for (auto it=(mTransactions->cbegin()++); it != mTransactions->cend(); ++it){
            if (it->second == nullptr) {
                // Transaction has no state, and, as a result, doesn't have timeout set.
                // Therefore, it can't be considered for awakening by the timeout.
                continue;
            }

            if (it->second->awakeningTimestamp() < nextTransactionAndState->second->awakeningTimestamp()){
                nextTransactionAndState = it;
            }
        }
    }

    if (nextTransactionAndState->second->mustBeRescheduled()){
        return make_pair(
            nextTransactionAndState->first,
            nextTransactionAndState->second->awakeningTimestamp());
    }

    throw NotFoundError(
        "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
            "there are no any delayed transactions.");
}

// This method used for finding earlier creating transaction of given type.
// Used for MaxFlowCalculationStepTwoTransaction for finishing those TA which was earlier started
// and find only TA which on stage of calculating max flow (mStep = 3)
BaseTransaction::Shared TransactionsScheduler::getEarlierTransaction(
    BaseTransaction::Shared transaction)
{
    auto earlierTransaction = transaction;
    if (mTransactions->size() > 1) {
        for (auto it=(mTransactions->begin()++); it != mTransactions->end(); ++it){
            if (it->second == nullptr) {
                // Transaction has no state, and, as a result, doesn't have timeout set.
                // Therefore, it can't be considered for awakening by the timeout.
                continue;
            }
            if (it->first->transactionType() != transaction->transactionType()) {
                continue;
            }

            if (it->first->currentStep() != 3) {
                continue;
            }

            if (microsecondsSinceGEOEpoch(utc_now()) >= it->second->awakeningTimestamp()) {
                it->second = TransactionState::awakeAsFastAsPossible();
            }
            if (it->first->timeStarted() < earlierTransaction->timeStarted()){
                earlierTransaction = it->first;
            }
        }
    }

    return earlierTransaction;
}

void TransactionsScheduler::asyncWaitUntil(
    GEOEpochTimestamp nextAwakeningTimestamp)
{
    GEOEpochTimestamp microsecondsDelay = 0;
    GEOEpochTimestamp now = microsecondsSinceGEOEpoch(utc_now());
    if (nextAwakeningTimestamp > now) {
        // NOTE: "now" is used twice:
        // in comparison and in delay calculation.
        //
        // It is important to use THE SAME timestamp in comparison and subtraction,
        // so it must not be replaced with 2 calls to microsecondsSinceGEOEpoch(utc_now())
        microsecondsDelay = nextAwakeningTimestamp - now;
    }

    mProcessingTimer->expires_from_now(
        chrono::microseconds(microsecondsDelay));

    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleAwakening,
            this,
            as::placeholders::error));
}

void TransactionsScheduler::handleAwakening(
    const boost::system::error_code &errorMessage)
{
    static auto errorsCount = 0;

    if (errorMessage && errorMessage == as::error::operation_aborted) {
        return;
    }

    if (errorMessage && errorMessage != as::error::operation_aborted) {

        if (errorsCount < 10) {
            error() << "handleAwakening. Error occurred on planned awakening. Details: " << errorMessage.message().c_str()
                   << ". Next awakening would be scheduled for " << errorsCount << " seconds from now.";

            // Transactions processing must not be cancelled.
            // Next awakening would be delayed for 10 seconds
            // (for cases when OS runs out of memory, or similar).
            asyncWaitUntil(
                microsecondsSinceGEOEpoch(
                    utc_now() + pt::seconds(errorsCount)));

        } else {
            error() << "handleAwakening. Some error repeatedly occurs on awakening. "
                   << "It seems that it can't be recovered automatically."
                   << "Next awakening would not be planned.";

            throw RuntimeError(
                "TransactionsScheduler::handleAwakening: "
                    "Some error repeated too much times");
        }
    }

    try {
        auto transactionAndDelay = transactionWithMinimalAwakeningTimestamp();
        if (microsecondsSinceGEOEpoch(utc_now()) >= transactionAndDelay.second) {
            if (transactionAndDelay.first->transactionType() !=
                    BaseTransaction::MaxFlowCalculationStepTwoTransactionType) {
                launchTransaction(transactionAndDelay.first);
                errorsCount = 0;
            } else {
                auto transaction = getEarlierTransaction(
                    transactionAndDelay.first);
                launchTransaction(transaction);
                errorsCount = 0;
            }
        }

        adjustAwakeningToNextTransaction();

    } catch (NotFoundError &) {
        // There are no transactions left.
        // Awakenings cycle reached the end.
        return;
    }
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
    TransactionsScheduler *scheduler)
{
    return scheduler->mTransactions.get();
}

const BaseTransaction::Shared TransactionsScheduler::cycleClosingTransactionByUUID(
    const TransactionUUID &transactionUUID) const
{
    for (const auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->currentTransactionUUID() == transactionUUID) {
            if (transactionAndState.first->transactionType() != BaseTransaction::Payments_CycleCloserInitiatorTransaction &&
                transactionAndState.first->transactionType() != BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction) {
                throw ValueError("TransactionsScheduler::cycleClosingTransactionByUUID: "
                                     "requested transaction doesn't belong to CycleClosing transactions");
            }
            return transactionAndState.first;
        }
    }
    throw NotFoundError("TransactionsScheduler::cycleClosingTransactionByUUID: "
                         "there is no transaction with requested UUID");
}

bool TransactionsScheduler::isTransactionInProcess(
    const TransactionUUID &transactionUUID) const
{
    for (const auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->currentTransactionUUID() == transactionUUID) {
            return true;
        }
    }
    return false;
}

void TransactionsScheduler::tryAttachMessageToCollectTopologyTransaction(
    Message::Shared message)
{
    for (auto const &transactionAndState : *mTransactions) {
        if (transactionAndState.first->transactionType() == BaseTransaction::InitiateMaxFlowCalculationTransactionType
            or transactionAndState.first->transactionType() == BaseTransaction::MaxFlowCalculationStepTwoTransactionType
            or transactionAndState.first->transactionType() == BaseTransaction::FindPathByMaxFlowTransactionType
            or transactionAndState.first->transactionType() == BaseTransaction::MaxFlowCalculationFullyTransactionType) {
            transactionAndState.first->pushContext(message);
            return;
        }
    }
    throw NotFoundError(
            "TransactionsScheduler::tryAttachMessageToCollectTopologyTransaction: "
                    "can't find CollectTopologyTransaction");
}

void TransactionsScheduler::tryAttachMessageToCyclesFiveAndSixNodes(
    Message::Shared message)
{
    for (auto const &transactionAndState : *mTransactions) {
        // Six and five nodes cycles should be discovered only once per day,
        // so, theoretically, only one discovering transaction may exist at once,
        // and message may be attached to first found transaction.
        if (transactionAndState.first->transactionType() == BaseTransaction::Cycles_SixNodesInitTransaction
            and message->typeID() == Message::Cycles_SixNodesBoundary
            and message->equivalent() == transactionAndState.first->equivalent()) {
            transactionAndState.first->pushContext(message);
            return;
        }
        if (transactionAndState.first->transactionType() == BaseTransaction::Cycles_FiveNodesInitTransaction
            and message->typeID() == Message::Cycles_FiveNodesBoundary
            and message->equivalent() == transactionAndState.first->equivalent()) {
            transactionAndState.first->pushContext(message);
            return;
        }
    }
    throw NotFoundError(
            "TransactionsScheduler::tryAttachMessageToCyclesFiveAndSixNodes: "
                "can't find appropriate transaction");
}

void TransactionsScheduler::tryAttachMessageToRoutingTableTransaction(
    Message::Shared message)
{
    for (auto const &transactionAndState : *mTransactions) {
        // Routing table updating TA should be run only once per 3 days,
        // so, theoretically, only one runned transaction may exist at once,
        // and message may be attached to first found transaction.
        if (transactionAndState.first->transactionType() == BaseTransaction::GatewayNotificationSenderType
            and message->typeID() == Message::RoutingTableResponse) {
            transactionAndState.first->pushContext(message);
            return;
        }
    }
    throw NotFoundError(
            "TransactionsScheduler::tryAttachMessageToRoutingTableTransaction: "
                "can't find appropriate transaction");
}

const BaseTransaction::Shared TransactionsScheduler::paymentTransactionByCommandUUID(
    const CommandUUID &commandUUID) const
{
    for (const auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->transactionType() != BaseTransaction::CoordinatorPaymentTransaction) {
            continue;
        }
        auto paymentTransaction = static_pointer_cast<CoordinatorPaymentTransaction>(transactionAndState.first);
        if (paymentTransaction->commandUUID() == commandUUID) {
            return transactionAndState.first;
        }
    }
    return nullptr;
}

string TransactionsScheduler::logHeader()
    noexcept
{
    return "[TransactionsScheduler]";
}

LoggerStream TransactionsScheduler::error() const
    noexcept
{
    return mLog.error(logHeader());
}

LoggerStream TransactionsScheduler::warning() const
    noexcept
{
    return mLog.warning(logHeader());
}

LoggerStream TransactionsScheduler::info() const
    noexcept
{
    return mLog.info(logHeader());
}

LoggerStream TransactionsScheduler::debug() const
    noexcept
{
    return mLog.debug(logHeader());
}