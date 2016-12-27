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

    } catch (exception &) {
        throw MemoryError(
                "TransactionsManager::TransactionsScheduler: "
                        "Can't allocate enough space for processing timer.");
    }

    mStorage = new storage::UUIDMapBlockStorage("storage", "transactions.bin");
}

TransactionsScheduler::~TransactionsScheduler() {
    if (mStorage != nullptr) {
        delete mStorage;
    }
}

void TransactionsScheduler::addTransaction(
        BaseTransaction::Shared transaction) {

    try{
        auto transactionContext = transaction.get()->serializeContext();
        mStorage->write(storage::uuids::uuid(transaction.get()->uuid()), transactionContext.first, transactionContext.second);
        launchTransaction(transaction);

    } catch (std::exception &e) {
        mLog->logError("Transactions scheduler", e.what());
        sleepFor(findTransactionWithMinimalTimeout().second);
    }
}

void TransactionsScheduler::run() {

    if (!mTransactions.empty()) {
        try{
            launchTransaction(findTransactionWithMinimalTimeout().first);

        } catch (std::exception &e) {
            mLog->logError("Transactions scheduler", e.what());
            sleepFor(findTransactionWithMinimalTimeout().second);
        }

    } else {
        sleepFor(kDefaultDelay);
    }
}

void TransactionsScheduler::launchTransaction(
        BaseTransaction::Shared transaction) {

    pair<CommandResult::SharedConst, TransactionState::SharedConst> transactionResult = transaction.get()->run();
    mTransactions.insert(make_pair(transaction, transactionResult.second));
    handleTransactionResult(transaction, transactionResult);
}

void TransactionsScheduler::handleTransactionResult(
        BaseTransaction::Shared transaction,
        pair<CommandResult::SharedConst, TransactionState::SharedConst> result) {

    if (result.first.get() == nullptr) {
        if (isTransactionInScheduler(transaction)){
            auto it = mTransactions.find(transaction);
            it->second = result.second;

            auto transactionContext = transaction.get()->serializeContext();
            mStorage->rewrite(storage::uuids::uuid(transaction.get()->uuid()), transactionContext.first, transactionContext.second);

        } else {
            throw ValueError("TransactionsManager::TransactionsScheduler"
                                     "Transaction reference must be store in memory");
        }
        sleepFor(findTransactionWithMinimalTimeout().second);

    } else if (result.second.get() == nullptr) {
        mMangerCallback(result.first);
        if (isTransactionInScheduler(transaction)) {
            mTransactions.erase(transaction);

            mStorage->erase(storage::uuids::uuid(transaction.get()->uuid()));

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
    timeout minimalTimeout = posix_time::milliseconds(0);
    for (auto &it : mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue.get()->timeout() > 0) {
                minimalTimeout = posix_time::milliseconds(transactionStateValue.get()->timeout());
                transaction = it.first;
            }
        }
    }

    if (minimalTimeout == posix_time::milliseconds(0)) {
        minimalTimeout = kDefaultDelay;
    }

    for (auto &it : mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue.get()->timeout() > 0 && posix_time::milliseconds(transactionStateValue.get()->timeout()) < minimalTimeout) {
                minimalTimeout = posix_time::milliseconds(transactionStateValue.get()->timeout());
                transaction = it.first;
            }
        }
    }

    return make_pair(transaction, minimalTimeout);

}

void TransactionsScheduler::sleepFor(
        timeout delay) {

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
        timeout delay) {

    if (error) {
        return;

    } else {
        if (delay > kDefaultDelay) {
            run();
        }
    }
}

bool TransactionsScheduler::isTransactionInScheduler(
        BaseTransaction::Shared transaction) {

    return mTransactions.count(transaction) > 0;
}
