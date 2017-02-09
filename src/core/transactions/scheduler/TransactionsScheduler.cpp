#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    storage::UUIDMapBlockStorage *storage,
    ManagerCallback managerCallback,
    Logger *logger) :

    mIOService(IOService),
    mStorage(storage),
    mManagerCallback(managerCallback),
    mLog(logger) {

    try {
        mTransactions = new map<BaseTransaction::Shared, TransactionState::SharedConst>();

    } catch (bad_alloc &e) {
        throw MemoryError("TransactionsScheduler::TransactionsScheduler."
                              "Can not allocate memory for transactions map.");
    }

    try {
        mProcessingTimer = new as::deadline_timer(
            mIOService,
            posix::milliseconds(2 * 1000));

    } catch (bad_alloc &e) {
        delete mTransactions;
        throw MemoryError("TransactionsScheduler::TransactionsScheduler."
                              "Can not allocate memory for deadline timer instance.");
    }

}

TransactionsScheduler::~TransactionsScheduler() {

    // todo: remove this
    delete mTransactions;
    delete mProcessingTimer;
}

/*!
 * Schedules first interruption to the timestamp of the next enqueued transaction.
 *
 * Note:
 * Calling this doesn't blocks execution.
 * No one transaction would be launched directly from this method.
 */
void TransactionsScheduler::run() {

    if (mTransactions->empty()) {
        // If no transactions are present - there is no reason to reschedule next interruption:
        // no transactions would be launched.
        //
        // New trasnaction can be only launched via network message or user command.
        return;
    }

    // WARN:
    // No one transaction should be launched directly from this method:
    // it would be called by the core when it will build trasnactions manager,
    // but there is no any gurantee, that the rest components are ready at this point of time.
    //
    // Therefore delayed transactions launching is used.
    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::scheduleTransaction(
    BaseTransaction::Shared transaction) {

    mTransactions->insert(
        make_pair(
            transaction,
            TransactionState::awakeAsFastAsPossible()));

    // New transaction is added into the queue.
    // Next awakening should be adjusted for the case,
    // when new transaction should be awakened first.
    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::postponeRoutingTableTransaction(
    BaseTransaction::Shared tranasction) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    if (tranasction->transactionType() != BaseTransaction::TransactionType::SendRoutingTablesTransactionType) {
        throw ConflictError("TransactionsScheduler::postponeRoutingTableTransaction: "
                                "Only routing tables transaction can be postponed.");
    }
#endif

    TransactionState *state = new TransactionState(
        kPostponeMillisecondsTime,
        false
    );
    TransactionState::Shared stateShared(state);

    mTransactions->insert(
        make_pair(
            tranasction,
            stateShared
        )
    );

    // todo: (DM) use asyncWaitUntil()
    rescheduleNextInterruption(nextDelayedTransaction().second);
}

void TransactionsScheduler::killTransaction(
    const TransactionUUID &transactionUUID) {

    for (const auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->UUID() == transactionUUID) {
            mTransactions->erase(transactionAndState.first);

            // todo: (dm) erase from transacrtions storage??
        }
    }
}

void TransactionsScheduler::handleMessage(
    Message::Shared message) {

    for (auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->UUID() != message->transactionUUID())
            continue; // todo: cycles algorythm will change this behaviour.

        for (auto &messageType : transactionAndState.second->acceptedMessagesTypes()) {
            if (messageType != message->typeID())
                continue;

            transactionAndState.first->setContext(message);
            launchTransaction(transactionAndState.first);
        }
    }
}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction) {

    try {
        // Even if transaction will raise exception -
        // it must not be thrown up,
        // (to not to break transactions processing flow).

        auto result = transaction->run();
//        if (!isTransactionInScheduler(transaction)) {
//            mTransactions->insert(
//                make_pair(
//                    transaction,
//                    result->state()));
//        }
        handleTransactionResult(transaction, result);

    } catch (exception &e) {
        // todo: add production log here

        auto errors = mLog->error("TransactionsScheduler::launchTransaction");
        errors << "Transaction interrupted with exception: "
               << "transaction type: " << transaction->transactionType()
               << e.what();
    }
}

void TransactionsScheduler::handleTransactionResult(
    BaseTransaction::Shared transaction,
    TransactionResult::Shared result) {

    if (result->messageResult() != nullptr) {
        if (result->state() != nullptr) {
            throw ConflictError(
                "TransactionsManager::handleTransactionResult. "
                    "Transaction result must not contains message result and state at the same time.");
        }

        // todo: write result code to the journal
        forgetTransaction(transaction);
        processNextTransactions();
        return;
    }

    if (result->commandResult() != nullptr) {
        if (result->state() != nullptr) {
            throw ConflictError(
                "TransactionsManager::handleTransactionResult. "
                    "Transaction result must not contains command result and state at the same time.");
        }

#ifdef DEBUG
        {
            auto debug = mLog->debug("TransactionsScheduler");
            debug << "Transaction result received: "
                  << result->commandResult()->serialize();
        }
#endif

        mManagerCallback(result->commandResult());
        forgetTransaction(transaction);
        processNextTransactions();
        return;
    }

    if (result->state() != nullptr) {
        if (result->commandResult() != nullptr) {
            throw ConflictError(
                "TransactionsManager::handleTransactionResult. "
                    "Transaction result must not contains command result and state at the same time.");
        }

        if (result->messageResult() != nullptr) {
            throw ConflictError(
                "TransactionsManager::handleTransactionResult. "
                    "Transaction result must not contains message result and state at the same time.");
        }

        if (result->state()->needSerialize()){
            serializeTransaction(transaction);
        }

        if (result->state()->mustBeRescheduled()){
            mTransactions->insert(
                make_pair(
                    transaction,
                    result->state()));
        } else {
            forgetTransaction(transaction);
        }

        processNextTransactions();
    }
}

void TransactionsScheduler::forgetTransaction(
    BaseTransaction::Shared transaction) {

    mTransactions->erase(transaction);

    try {
        mStorage->erase(
            storage::uuids::uuid(transaction->UUID()));

    } catch (IndexError &) {}
}

void TransactionsScheduler::serializeTransaction(
    BaseTransaction::Shared transaction) {

    auto transactionBytesAndCount = transaction->serializeToBytes();
    if (!mStorage->isExist(storage::uuids::uuid(transaction->UUID()))) {
        mStorage->write(
            storage::uuids::uuid(transaction->UUID()),
            transactionBytesAndCount.first.get(),
            transactionBytesAndCount.second);

    } else {
        mStorage->rewrite(
            storage::uuids::uuid(transaction->UUID()),
            transactionBytesAndCount.first.get(),
            transactionBytesAndCount.second);
    }
}

void TransactionsScheduler::processNextTransactions(){

    for (size_t iteration=0; iteration<64; ++iteration){
        try {
            auto transactionAndTimestamp = transactionWithMinimalAwakeningTimestamp();

            if (transactionAndTimestamp.second > now()) {
                asyncWaitUntil(transactionAndTimestamp.second);

            } else {
                // Next transaction is ready to be executed.
                // There is no reason to run one more async calls cycle:
                // transaction may be launched directly;
                //
                // WARN:
                // Iterations count is limited to prevent ignoring async loop (and network messages),
                // and stack overflowing;
                launchTransaction(transactionAndTimestamp.first);
            }

        } catch (NotFoundError) {
            // No delayed transaction is present.
            break;
        }
    }
}

pair<BaseTransaction::Shared, Duration> TransactionsScheduler::nextDelayedTransaction() {

    BaseTransaction::Shared transaction(nullptr);
    Duration minimalTimeout = posix::milliseconds(0);

    for (auto &it : *mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue->awakeningTimestamp() > 0) {
                minimalTimeout = posix::milliseconds(transactionStateValue->awakeningTimestamp());
                transaction = it.first;
            }
        }
    }

    for (auto &it : *mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue->awakeningTimestamp() > 0 && posix::milliseconds(
                transactionStateValue->awakeningTimestamp()) < minimalTimeout) {
                minimalTimeout = posix::milliseconds(transactionStateValue->awakeningTimestamp());
                transaction = it.first;
            }
        }
    }

    return make_pair(transaction, minimalTimeout);

}

/*!
 * Returns transaction that is next in transactions queue (by the awakening timestamps).
 *
 * WARN:
 * This method returns transaction even if it's awakening timestamp has been not fired up.
 * Transaction it returns must not be launched, if it's state prohibits it.
 * To get next transaction that must be launched by the timestamp - use nextDelayedTransaction();
 *
 *
 * Throws NotFoundError in case if no transactions are delayed.
 */
pair<BaseTransaction::Shared, TransactionState::AwakeTimestamp>
TransactionsScheduler::transactionWithMinimalAwakeningTimestamp() const {

    if (mTransactions->empty()) {
        throw NotFoundError(
            "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
                "there is no any delayed transactions are present.");
    }

    BaseTransaction::Shared transaction(nullptr);
    TransactionState::AwakeTimestamp timestamp;

    for (auto &it : *mTransactions) {
        auto transactionState = it.second;
        if (transactionState != nullptr) {
            if (transactionState->awakeningTimestamp() > 0) {
                timestamp = transactionState->awakeningTimestamp();
                transaction = it.first;
            }
        }
    }

    return make_pair(transaction, timestamp);
}

void TransactionsScheduler::adjustAwakeningToNextTransaction() {
    
    try {
        asyncWaitUntil(
            transactionWithMinimalAwakeningTimestamp().second);

    } catch (NotFoundError &) {

#ifdef DEBUG
        auto debug = mLog->debug("TransactionsScheduler");
        debug << "No transactions are left. Next awakening is not set.";
#endif

    }
}

void TransactionsScheduler::rescheduleNextInterruption(
    Duration delay) {

    mProcessingTimer->cancel();
    mProcessingTimer->expires_from_now(delay);
    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleAwakening,
            this,
            as::placeholders::error));
}

void TransactionsScheduler::handleAwakening(
    const boost::system::error_code &error) {

    if (error &&
        error != as::error::operation_aborted) {

        // todo: add production log here

#ifdef DEBUG
        auto debug = mLog->debug("TransactionsScheduler");
        debug << "Scheduler handleAwakening received an error. "
              << error.message().c_str();
#endif

        throw RuntimeError(
            string("TransactionsScheduler::handleAwakening: "
                "error occurred: ") + error.message());
    }

    try {
        auto transactionAndState = transactionWithMinimalAwakeningTimestamp();
        launchTransaction(transactionAndState.first);

    } catch (NotFoundError &) {
        // There are no transactions left.
        // This cycle of awakenings cycle reached the end.
        return;
    }
}

bool TransactionsScheduler::isTransactionInScheduler(
    BaseTransaction::Shared transaction) {

    return mTransactions->count(transaction) != 0;
}

MicrosecondsTimestamp TransactionsScheduler::now() const {

    return microsecondsTimestamp(
        boost::posix_time::microsec_clock::universal_time());
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
    TransactionsScheduler *scheduler) {

    return scheduler->mTransactions;
}

void TransactionsScheduler::asyncWaitUntil(
    TransactionState::AwakeTimestamp nextAwakeningTimestamp) {

    boost::posix_time::ptime t =
        TransactionState::GEOEpoch() +
            datetime::microseconds(nextAwakeningTimestamp);

    mProcessingTimer->cancel();
    mProcessingTimer->expires_at(t);
    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleAwakening,
            this,
            as::placeholders::error));
}
