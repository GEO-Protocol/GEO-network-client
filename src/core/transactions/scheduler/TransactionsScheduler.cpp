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
                mIOService, boost::posix_time::seconds(2));

    } catch (exception &) {
        throw MemoryError(
                "TransactionsManager::TransactionsScheduler: "
                        "Can't allocate enough space for mReadTimeoutTimer.");
    }
}

void TransactionsScheduler::addTransaction(
        BaseTransaction::Shared transaction) {

    mTransactions.insert(make_pair(transaction, nullptr));
    launchTransaction(transaction,
                      boost::bind(&TransactionsScheduler::handleTransactionResult, this, transaction, mTemporaryTransactionResult));
}

void TransactionsScheduler::run() {


}

void TransactionsScheduler::launchTransaction(
        BaseTransaction::Shared transaction,
        boost::_bi::bind_t<void, boost::_mfi::mf2<void, TransactionsScheduler, BaseTransaction::Shared, pair<CommandResult::SharedConst, TransactionState::SharedConst>>, boost::_bi::list_av_3<TransactionsScheduler *, std::shared_ptr<BaseTransaction>, std::pair<std::shared_ptr<CommandResult>, std::shared_ptr<TransactionState>>>::type> handler) {

    mTemporaryTransactionResult = transaction.get()->run();
}

void TransactionsScheduler::handleTransactionResult(
        BaseTransaction::Shared transaction,
        pair<CommandResult::SharedConst, TransactionState::SharedConst> result) {

    if (result.first.get() == nullptr) {
        if (isTransactionInScheduler(transaction)){
            auto it = mTransactions.find(transaction);
            it->second = result.second;
        } else {
            throw ValueError("TransactionsManager::TransactionsScheduler"
                                     "Transaction reference must be store in memory");
        }
        sleepForMinimalTimeout();

    } else if (result.second.get() == nullptr) {
        mMangerCallback(result.first);
        if (isTransactionInScheduler(transaction)) {
            mTransactions.erase(transaction);
        } else {
            throw ValueError("TransactionsManager::TransactionsScheduler"
                                     "Transaction reference must be store in memory");
        }
        sleepForMinimalTimeout();

    } else if (result.first.get() == nullptr && result.second.get() == nullptr) {
        throw ValueError("TransactionsManager::TransactionsScheduler"
                                 "Command result and transaction state may not be null pointer references at the same time");
    }
}

bool TransactionsScheduler::isTransactionInScheduler(
        BaseTransaction::Shared transaction) {
    return mTransactions.count(transaction) > 0;
}

void TransactionsScheduler::sleepForMinimalTimeout() {
    timeout minimalTimeout = posix_time::seconds(0);
    for (auto &it : mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue.get()->timeout() > 0) {
                minimalTimeout = posix_time::seconds(transactionStateValue.get()->timeout());
            }
        }
    }

    if (minimalTimeout == posix_time::seconds(0)) {
        minimalTimeout = kDefaultDelay;
    }

    for (auto &it : mTransactions) {
        auto transactionStateValue = it.second;
        if (transactionStateValue.get() != nullptr) {
            if (transactionStateValue.get()->timeout() > 0 && posix_time::seconds(transactionStateValue.get()->timeout()) < minimalTimeout) {
                minimalTimeout = posix_time::seconds(transactionStateValue.get()->timeout());
            }
        }
    }

    mProcessingTimer->expires_from_now(minimalTimeout);
    mProcessingTimer->async_wait(
            boost::bind(
                    &TransactionsScheduler::handleTimeout, this, as::placeholders::error));

}

void TransactionsScheduler::handleTimeout(
        const boost::system::error_code &error) {

    if (error) {
        mLog->logError("CommandsInterface::handleTimeout", error.message());
    }

    run();
}
