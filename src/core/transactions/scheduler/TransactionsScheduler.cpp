#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    storage::UUIDMapBlockStorage *storage,
    Logger *logger) :

    mIOService(IOService),
    mStorage(storage),
    mLog(logger),

    mTransactions(new map<BaseTransaction::Shared, TransactionState::SharedConst>()),

    mProcessingTimer(new as::deadline_timer(
        mIOService,
        posix::milliseconds(2 * 1000))
    ) {
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
        return;
    }

    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::scheduleTransaction(
    BaseTransaction::Shared transaction) {

    mTransactions->insert(
        make_pair(
            transaction,
            TransactionState::awakeAsFastAsPossible()));

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
            mTransactions->erase(transactionAndState.first);
            mStorage->erase(
                storage::uuids::uuid(
                    transactionAndState.first.get()->UUID()
                )
            );
        }
    }
}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction) {

    try {

        auto result = transaction->run();

        handleTransactionResult(
            transaction,
            result
        );

    } catch (exception &e) {

        auto errors = mLog->error("TransactionsScheduler");
        errors << "Transaction interrupted with exception: "
               << "transaction type: " << transaction->transactionType()
               << e.what();
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
        }

        case TransactionResult::ResultType::MessageResultType: {
            processMessageResult(
                transaction,
                result->messageResult()
            );
        }

        case TransactionResult::ResultType::TransactionStateType: {
            processTransactionState(
                transaction,
                result->state()
            );
        }

        default: {
            break;
        }
    }

}

void TransactionsScheduler::processCommandResult(
    BaseTransaction::Shared transaction,
    CommandResult::SharedConst result) {

    if (isTransactionScheduled(transaction)) {
        commandResultIsReadySignal(result);
        if (mStorage->isExist(storage::uuids::uuid(transaction->UUID()))) {
            mStorage->erase(
                storage::uuids::uuid(transaction->UUID())
            );
        }
        mTransactions->erase(transaction);

    } else {
        throw ValueError("TransactionsManager::handleTransactionResult. "
                             "Transaction reference must be store in memory.");
    }
}

void TransactionsScheduler::processMessageResult(
    BaseTransaction::Shared transaction,
    MessageResult::SharedConst result) {

    if (isTransactionScheduled(transaction)) {
        if (mStorage->isExist(storage::uuids::uuid(transaction->UUID()))) {
            mStorage->erase(
                storage::uuids::uuid(transaction->UUID())
            );
        }
        mTransactions->erase(transaction);

    } else {
        throw ValueError("TransactionsManager::processMessageResult. "
                             "Transaction reference must be store in memory.");
    }
}

void TransactionsScheduler::processTransactionState(
    BaseTransaction::Shared transaction,
    TransactionState::SharedConst state) {

    if (isTransactionScheduled(transaction)) {
        auto transactionAndState = mTransactions->find(transaction);
        transactionAndState->second = state;
        if (transactionAndState->second->needSerialize()) {

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

    } else {
        throw ValueError("TransactionsManager::TransactionsScheduler"
                             "Transaction reference must be store in memory");
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
pair<BaseTransaction::Shared, MicrosecondsTimestamp> TransactionsScheduler::transactionWithMinimalAwakeningTimestamp() const {

    if (mTransactions->empty()) {
        throw NotFoundError(
            "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
                "There is no delayed any transactions.");
    }

    BaseTransaction::Shared transaction(nullptr);
    MicrosecondsTimestamp awakeningTimestamp = 0;

    for (auto &transactionAndState : *mTransactions) {
        auto transactionState = transactionAndState.second;
        if (transactionState != nullptr) {
            if (transactionState->awakeningTimestamp() > 0) {
                awakeningTimestamp = transactionState->awakeningTimestamp();
                transaction = transactionAndState.first;
            }
        }
    }

    return make_pair(
        transaction,
        awakeningTimestamp
    );
}

void TransactionsScheduler::asyncWaitUntil(
    MicrosecondsTimestamp nextAwakeningTimestamp) {

    mProcessingTimer->cancel();
    mProcessingTimer->expires_at(
        timestamp() +
        posix::microseconds(nextAwakeningTimestamp)
    );
    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleAwakening,
            this,
            as::placeholders::error
        )
    );
}

void TransactionsScheduler::handleAwakening(
    const boost::system::error_code &error) {

    if (error) {
        throw RuntimeError(
            string("TransactionsScheduler::handleAwakening: "
                       "Error occurred: ") + error.message());
    }

    try {
        auto transactionAndState = transactionWithMinimalAwakeningTimestamp();
        launchTransaction(transactionAndState.first);

    } catch (NotFoundError&) {
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