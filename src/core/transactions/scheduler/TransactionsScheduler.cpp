#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    ManagerCallback managerCallback,
    Logger *logger) :

    mIOService(IOService),
    mManagerCallback(managerCallback),
    mLog(logger) {

    try {
        mTransactions = new map<BaseTransaction::Shared, TransactionState::SharedConst>();

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsScheduler::TransactionsScheduler."
                              "Can not allocate memory for transactions map.");
    }

    try {
        mProcessingTimer = new as::deadline_timer(
            mIOService,
            boost::posix_time::milliseconds(2 * 1000));

    } catch (std::bad_alloc &e) {
        delete mTransactions;
        throw MemoryError("TransactionsScheduler::TransactionsScheduler."
                              "Can not allocate memory for deadline timer instance.");
    }

    try{
        mStorage = new storage::UUIDMapBlockStorage(
            "io/transactions",
            "transactions.dat");

    } catch (std::bad_alloc &e) {
        delete mTransactions;
        delete mProcessingTimer;
        throw MemoryError("TransactionsScheduler::TransactionsScheduler."
                              "Can not allocate memory for UUIDMapBlockStorage instance.");
    }
}

TransactionsScheduler::~TransactionsScheduler() {

    delete mTransactions;
    delete mProcessingTimer;
    delete mStorage;
}

void TransactionsScheduler::scheduleTransaction(
    BaseTransaction::Shared transaction) {

    try {
        /*auto transactionContext = transaction->serializeContext();
        mStorage->write(
         storage::uuids::uuid(transaction->uuid()),
         transactionContext.first,
         transactionContext.second);*/

        launchTransaction(transaction);

    } catch (std::exception &e) {
        mLog->logError(
            "TransactionsScheduler",
            e.what());
        sleepFor(findTransactionWithMinimalTimeout().second);
    }
}

void TransactionsScheduler::run() {

    if (!mTransactions->empty()) {
        try {
            launchTransaction(findTransactionWithMinimalTimeout().first);

        } catch (std::exception &e) {
            mLog->logError(
                "TransactionsScheduler",
                e.what());
            sleepFor(findTransactionWithMinimalTimeout().second);
        }
    }
}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction) {

    auto transactionResult = transaction->run();
    if (!isTransactionInScheduler(transaction)) {
        mTransactions->insert(
            make_pair(
                transaction,
                transactionResult->transactionState()
            )
        );
    }
    handleTransactionResult(transaction, transactionResult);
}

/**
 * Handling transaction executing result.
 * Result consist of command result and transaction state.
 * If transaction executed successfully - result has reference to command result,
 * else - result has reference to transaction state.
 * If handler has transaction state, he checking if transaction is present in scheduler's map, change
 * her state in RAM and writing context in storage. Then looking for transaction, which state has minimal timeout for delay.
 * If handler has command result, he return result via transactions manager's callback and remove transaction from RAM and storage.
 * Then looking for transaction which state has minimal timeout for delay.
 * In both cases, if map is empty - scheduler loose control.
 *
 * throw ValueError - transaction has executed, but she's absent in map
 * throw ConflictError - transaction's result and state will could not been nullptr or ptr at the same time
 */
void TransactionsScheduler::handleTransactionResult(
    BaseTransaction::Shared transaction,
    TransactionResult::Shared result) {

    if (result->messageResult().get() != nullptr) {
        //TODO:: journal

    } else if (result->commandResult().get() == nullptr) {
        if (isTransactionInScheduler(transaction)) {
            auto it = mTransactions->find(transaction);
            it->second = result->transactionState();
            /*auto transactionContext = transaction->serializeContext();
             mStorage->rewrite(
             storage::uuids::uuid(transaction->uuid()),
             transactionContext.first,
             transactionContext.second);*/

        } else {
            throw ValueError("TransactionsManager::TransactionsScheduler"
                                 "Transaction reference must be store in memory");
        }
        Timeout minimalDelay = findTransactionWithMinimalTimeout().second;
        if (minimalDelay > posix_time::milliseconds(0)) {
            sleepFor(minimalDelay);
        }

    } else if (result->transactionState().get() == nullptr) {
        mManagerCallback(result->commandResult());
        if (isTransactionInScheduler(transaction)) {
            mTransactions->erase(transaction);
            /*mStorage->erase(
                storage::uuids::uuid(transaction->uuid()));*/

        } else {
            throw ValueError("TransactionsManager::handleTransactionResult. "
                                 "Transaction reference must be store in memory.");
        }
        Timeout minimalDelay = findTransactionWithMinimalTimeout().second;
        if (minimalDelay > posix_time::milliseconds(0)) {
            sleepFor(minimalDelay);
        }

    } else if (result->commandResult().get() == nullptr && result->transactionState().get() == nullptr) {
        throw ConflictError("TransactionsManager::handleTransactionResult. "
                                "Command result and transaction state may not be null pointer references at the same time.");

    } else if (result->commandResult().get() != nullptr && result->transactionState().get() != nullptr) {
        throw ConflictError("TransactionsManager::handleTransactionResult. "
                                "Transaction cat not has command result and transaction state at the same time.");
    }
}

pair<BaseTransaction::Shared, Timeout> TransactionsScheduler::findTransactionWithMinimalTimeout() {

    BaseTransaction::Shared transaction(nullptr);
    Timeout minimalTimeout = posix_time::milliseconds(0);

    for (auto &it : *mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue->timeout() > 0) {
                minimalTimeout = posix_time::milliseconds(transactionStateValue->timeout());
                transaction = it.first;
            }
        }
    }

    for (auto &it : *mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue->timeout() > 0 && posix_time::milliseconds(transactionStateValue->timeout()) < minimalTimeout) {
                minimalTimeout = posix_time::milliseconds(transactionStateValue->timeout());
                transaction = it.first;
            }
        }
    }

    return make_pair(transaction, minimalTimeout);

}

void TransactionsScheduler::sleepFor(
    Timeout delay) {

    mProcessingTimer->cancel();
    mProcessingTimer->expires_from_now(delay);
    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleSleep,
            this,
            as::placeholders::error,
            delay));
}

void TransactionsScheduler::handleSleep(
    const boost::system::error_code &error,
    Timeout delay) {

    if (error) {
        mLog->logError(
            "TransactionsScheduler",
            error.message());
        return;

    } else {
        if (delay > posix_time::milliseconds(0)) {
            run();
        }
    }
}

bool TransactionsScheduler::isTransactionInScheduler(
    BaseTransaction::Shared transaction) {

    return mTransactions->count(transaction) != 0;
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
    TransactionsScheduler *scheduler) {

    return scheduler->mTransactions;
}
