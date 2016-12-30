#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    function_callback managerCallback,
    Logger *logger) :

    mIOService(IOService),
    mMangerCallback(managerCallback),
    mLog(logger) {

    try {
        mProcessingTimer = new as::deadline_timer(
            mIOService, boost::posix_time::milliseconds(2 * 1000));

    } catch (exception &) { // todo: -> bad_alloc
        // todo: call delete mProcessingTimer
        throw MemoryError(
            "TransactionsScheduler::TransactionsScheduler");
    }

    // todo: this code can also throw bad_alloc
    mStorage = new storage::UUIDMapBlockStorage("storage", "transactions.bin"); // todo: the path should be io/transactions/transactions.dat
}

TransactionsScheduler::~TransactionsScheduler() {
    //todo: mStorage would not be nullptr by default.
    if (mStorage != nullptr) {
        delete mStorage;
    }
}

void TransactionsScheduler::addTransaction(
    BaseTransaction::Shared transaction) {

    try {
        // todo: is this code needed?
        /*auto transactionContext = transaction.get()->serializeContext();
        mStorage->write(storage::uuids::uuid(transaction.get()->uuid()), transactionContext.first, transactionContext.second);*/
        launchTransaction(transaction);

    } catch (std::exception &e) {
        mLog->logError("Transactions scheduler", e.what());
        sleepFor(findTransactionWithMinimalTimeout().second);
    }
}

void TransactionsScheduler::run() {

    if (!mTransactions.empty()) {
        try {
            launchTransaction(findTransactionWithMinimalTimeout().first);

        } catch (std::exception &e) {
            mLog->logError("Transactions scheduler", e.what());
            sleepFor(findTransactionWithMinimalTimeout().second);
        }

    } else {
        sleepFor(kDefaultDelay); // todo: there should be no default delay!
    }
}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction) {

    // todo: pair<CommandResult::SharedConst, TransactionState::SharedConst> transactionResult -> auto transactionResult
    pair<CommandResult::SharedConst, TransactionState::SharedConst> transactionResult = transaction.get()->run(); // todo: use ->
    if (!isTransactionInScheduler(transaction)) {
        // todo: would the transaction be saved to the storage?
        mTransactions.insert(make_pair(transaction, transactionResult.second));
    }
    handleTransactionResult(transaction, transactionResult);
}

// todo: document all the exceptions may be thrown
void TransactionsScheduler::handleTransactionResult(
    BaseTransaction::Shared transaction,
    pair<CommandResult::SharedConst, TransactionState::SharedConst> result) {

    // todo: please, document the logic of the method.
    if (result.first.get() == nullptr) {
        if (isTransactionInScheduler(transaction)) {
            auto it = mTransactions.find(transaction);
            it->second = result.second;
            /*auto transactionContext = transaction.get()->serializeContext();
            mStorage->rewrite(storage::uuids::uuid(transaction.get()->uuid()), transactionContext.first, transactionContext.second);*/

        } else {
            throw ValueError("TransactionsManager::TransactionsScheduler"
                                 "Transaction reference must be store in memory");
        }

        sleepFor(findTransactionWithMinimalTimeout().second); // todo: what if no more transactions?

    } else if (result.second.get() == nullptr) {
        mMangerCallback(result.first);
        if (isTransactionInScheduler(transaction)) {
            mTransactions.erase(transaction);
            //mStorage->erase(storage::uuids::uuid(transaction.get()->uuid()));

        } else {
            throw ValueError("TransactionsManager::TransactionsScheduler"
                                 "Transaction reference must be store in memory");
        }
        sleepFor(findTransactionWithMinimalTimeout().second);

    } else if (result.first.get() == nullptr && result.second.get() == nullptr) {
        throw ConflictError("TransactionsManager::TransactionsScheduler"
                                "Command result and transaction state may not be null pointer references at the same time");

    } else if (result.first.get() != nullptr && result.second.get() != nullptr) {
        throw ConflictError("TransactionsManager::TransactionsScheduler"
                                "Transaction cat not has command result and transaction state at the same time");
    }
}

pair<BaseTransaction::Shared, timeout> TransactionsScheduler::findTransactionWithMinimalTimeout() {

    BaseTransaction::Shared transaction;
    timeout minimalTimeout = posix_time::milliseconds(0); // todo: may it be const?
    for (auto &it : mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue.get()->timeout() > 0) {
                // todo: there is no comparison with previous timeout
                // is it calculates the minimum timeout correct?
                minimalTimeout = posix_time::milliseconds(transactionStateValue.get()->timeout());
                transaction = it.first;
            }
        }
    }

    // todo: there is no default timeout in the system.
    if (minimalTimeout == posix_time::milliseconds(0)) {
        minimalTimeout = kDefaultDelay;

    }

    for (auto &it : mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue.get()->timeout() > 0 &&
                posix_time::milliseconds(transactionStateValue.get()->timeout()) < minimalTimeout) {
                minimalTimeout = posix_time::milliseconds(transactionStateValue.get()->timeout());
                transaction = it.first;
            }
        }
    }

    return make_pair(transaction, minimalTimeout);

}

void TransactionsScheduler::sleepFor(
    timeout delay) {

    // todo: add previous operation cancelling.
    mProcessingTimer->expires_from_now(delay);
    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleSleep, this,
            as::placeholders::error, delay));
}

void TransactionsScheduler::handleSleep(
    const boost::system::error_code &error,
    timeout delay) {

    if (error) {
        // todo: no log - no problems?
        return;

    } else {
        // todo: there is no default timeout
        if (delay > kDefaultDelay) {
            run();
        }
    }
}

bool TransactionsScheduler::isTransactionInScheduler(
    BaseTransaction::Shared transaction) {

    return mTransactions.count(transaction) > 0;
}
