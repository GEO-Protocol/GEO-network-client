#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    storage::UUIDMapBlockStorage *storage,
    Logger *logger) :

    mIOService(IOService),
    mStorage(storage),
    mLog(logger),

    mTransactions(new map<BaseTransaction::Shared, TransactionState::SharedConst>()),
    mProcessingTimer(new as::deadline_timer(mIOService)) {
}

/*!
 * Schedules awakening for the next transaction.
 * Doesn't blocks execution.
 */
void TransactionsScheduler::run() {

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
    BaseTransaction::Shared transaction) {

    mTransactions->insert(
        make_pair(
            transaction,
            TransactionState::awakeAsFastAsPossible()
        )
    );

    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::handleMessage(
    Message::Shared message) {

    for (auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->UUID() != message->transactionUUID()) {
            continue;
        }

        for (auto &messageType : transactionAndState.second->acceptedMessagesTypes()) {
            if (messageType != message->typeID()) {
                continue;
            }

            transactionAndState.first->setContext(message);
            launchTransaction(transactionAndState.first);
        }
    }
}

void TransactionsScheduler::killTransaction(
    const TransactionUUID &transactionUUID) {

    for (const auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->UUID() == transactionUUID) {
           forgetTransaction(transactionAndState.first);
        }
    }
}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction) {

    try {
        // Even if transaction will raise an exception -
        // it must not be thrown up,
        // to not to break transactions processing flow.

        auto result = transaction->run();
        handleTransactionResult(
            transaction,
            result
        );

    } catch (exception &e) {
        mLog->logException(
            "TransactionScheduler::launchTransaction:",
            e
        );
        forgetTransaction(transaction);
        processNextTransactions();
        // todo: add production log here. (this one is unusable)
        /*auto errors = mLog->error("TransactionsScheduler");
        errors << "Transaction interrupted with exception: "
               << "transaction type: " << transaction->transactionType()
               << e.what();*/
    }
}


void TransactionsScheduler::handleTransactionResult(
    BaseTransaction::Shared transaction,
    TransactionResult::SharedConst result) {

    switch (result->resultType()) {
        case TransactionResult::ResultType::CommandResultType: {
            processCommandResult(
                transaction,
                result->commandResult()
            );
            break;
        }

        case TransactionResult::ResultType::MessageResultType: {
            processMessageResult(
                transaction,
                result->messageResult()
            );
            break;
        }

        case TransactionResult::ResultType::TransactionStateType: {
            processTransactionState(
                transaction,
                result->state()
            );
            break;
        }
    }

    processNextTransactions();
}

void TransactionsScheduler::processCommandResult(
    BaseTransaction::Shared transaction,
    CommandResult::SharedConst result) {

#ifdef DEBUG
    assert(isTransactionScheduled(transaction));
#endif

    commandResultIsReadySignal(result);
    forgetTransaction(transaction);
}

void TransactionsScheduler::processMessageResult(
    BaseTransaction::Shared transaction,
    MessageResult::SharedConst result) {

#ifdef DEBUG
    assert(isTransactionScheduled(transaction));
#endif

    // todo: add writing to remote transactions results log.
    forgetTransaction(transaction);
}

void TransactionsScheduler::processTransactionState(
    BaseTransaction::Shared transaction,
    TransactionState::SharedConst state) {

#ifdef DEBUG
    assert(isTransactionScheduled(transaction));
#endif

    if (state->mustBeRescheduled()){
        if (state->needSerialize())
            serializeTransaction(transaction);

        // From the c++ reference:
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

void TransactionsScheduler::forgetTransaction(
    BaseTransaction::Shared transaction) {

    mTransactions->erase(transaction);

    try {
        mStorage->erase(
            storage::uuids::uuid(transaction->UUID())
        );

    } catch (IndexError &) {}
}

void TransactionsScheduler::serializeTransaction(
    BaseTransaction::Shared transaction) {

    auto transactionBytesAndCount = transaction->serializeToBytes();
    if (!mStorage->isExist(storage::uuids::uuid(transaction->UUID()))) {
        mStorage->write(
            storage::uuids::uuid(transaction->UUID()),
            transactionBytesAndCount.first.get(),
            transactionBytesAndCount.second
        );

    } else {
        mStorage->rewrite(
            storage::uuids::uuid(transaction->UUID()),
            transactionBytesAndCount.first.get(),
            transactionBytesAndCount.second
        );
    }
}

void TransactionsScheduler::processNextTransactions(){

    const size_t maxTAWithoutInterruption = 64;
    for (size_t iteration=0; iteration < maxTAWithoutInterruption; ++iteration){
        try {
            auto transactionAndTimestamp = transactionWithMinimalAwakeningTimestamp();
            if (microsecondsSinceGEOEpoch(utc_now()) >= transactionAndTimestamp.second) {
                // Next transaction is ready to be executed.
                // (there is no reason to run one more async call: next transaction may be launched directly).
                //
                // Note:
                // Transactions that may be launched in this manner is limited,
                // to prevent ignoring async loop (and network messages)
                // in case when node is executing huge amount of transactions;
                launchTransaction(transactionAndTimestamp.first);

            } else {
                asyncWaitUntil(transactionAndTimestamp.second);

                // Note: asyncWaitUntil call will not block.
                // "return" is needed to prevent multiple transactions scheduling.
                return;
            }

        } catch (NotFoundError) {
            // No delayed transactions are present.
            return;
        }
    }

    // Transactions limit reached
    // Force interruption to process sockets,
    // but only if other transactions are present in queue.
    if (mTransactions->size() > 0){
        asyncWaitUntil(
            TransactionState::awakeAfterMilliseconds(50)->awakeningTimestamp()
        );
    }
}

void TransactionsScheduler::adjustAwakeningToNextTransaction() {

    try {
        asyncWaitUntil(transactionWithMinimalAwakeningTimestamp().second);

    } catch (NotFoundError &e) {}
}

/*!
 *
 * Throws NotFoundError in case if no transactions are delayed.
 */
pair<BaseTransaction::Shared, GEOEpochTimestamp> TransactionsScheduler::transactionWithMinimalAwakeningTimestamp() const {

    if (mTransactions->empty()) {
        throw NotFoundError(
            "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
                "There are no any delayed transactions.");
    }

    auto nextTransactionAndState = mTransactions->cbegin();
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

    if (nextTransactionAndState->second->mustBeRescheduled()){
        return make_pair(
            nextTransactionAndState->first,
            nextTransactionAndState->second->awakeningTimestamp()
        );
    }

    throw NotFoundError(
        "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
            "there are no any delayed transactions.");
}

void TransactionsScheduler::asyncWaitUntil(
    GEOEpochTimestamp nextAwakeningTimestamp) {

    auto awakeningDateTime =
        dateTimeFromGEOEpochTimestamp(
            nextAwakeningTimestamp);

    mProcessingTimer->cancel();
    mProcessingTimer->expires_from_now(
        awakeningDateTime - utc_now()
    );

    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleAwakening,
            this,
            as::placeholders::error)
    );
}

void TransactionsScheduler::handleAwakening(
    const boost::system::error_code &error) {

    static auto errorsCount = 0;

    if (error && error != as::error::operation_aborted) {

        auto errors = mLog->error("TransactionsScheduler::handleAwakening");
        if (errorsCount < 10) {
            errors << "Error occurred on planned awakening. Details: " << error.message().c_str()
                   << ". Next awakening would be scheduled for" << errorsCount << " seconds from now.";

            // Transactions processing must not be cancelled.
            // Next awakening would be delayed for 10 seconds
            // (for cases when OS runs out of memory, or similar).
            asyncWaitUntil(
                microsecondsSinceGEOEpoch(
                    utc_now() + pt::seconds(errorsCount)
                )
            );

        } else {
            errors << "Some error repeatedly occurs on awakening. "
                   << "It seems that it can't be recovered automatically."
                   << "Next awakening would not be planned.";

            throw RuntimeError(
                "TransactionsScheduler::handleAwakening: "
                    "Some error repeated too much times");
        }
    }

    try {
        auto transactionAndState = transactionWithMinimalAwakeningTimestamp();
        launchTransaction(transactionAndState.first);

        errorsCount = 0;

    } catch (NotFoundError &) {
        // There are no transactions left.
        // Awakenings cycle reached the end.
        return;
    }
}

bool TransactionsScheduler::isTransactionScheduled(
    BaseTransaction::Shared transaction) {

    return mTransactions->count(transaction) != 0;
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
    TransactionsScheduler *scheduler) {

    return scheduler->mTransactions.get();
}