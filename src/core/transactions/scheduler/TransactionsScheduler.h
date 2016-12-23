#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../transactions/BaseTransaction.h"
#include "../../interface/results/result/CommandResult.h"
#include "../transactions/state/TransactionState.h"

#include "../../common/exceptions/ValueError.h"
#include "../../logger/Logger.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <map>

using namespace std;

namespace as = boost::asio;

typedef boost::function<void(CommandResult::SharedConst)> function_callback;
typedef boost::function<void(BaseTransaction::Shared, pair<CommandResult::SharedConst, TransactionState::SharedConst>)> handler_callback;
typedef boost::posix_time::time_duration timeout;

namespace posix_time = boost::posix_time;

class TransactionsManager;

class TransactionsScheduler {
    friend class TransactionsManager;

private:
    const timeout kDefaultDelay = posix_time::seconds(60 * 3);

private:
    as::io_service &mIOService;
    as::deadline_timer *mProcessingTimer;

    function_callback mMangerCallback;

    map<BaseTransaction::Shared, TransactionState::SharedConst> mTransactions;
    pair<CommandResult::SharedConst, TransactionState::SharedConst> mTemporaryTransactionResult;

    Logger *mLog;

private:
    TransactionsScheduler(
            as::io_service &IOService,
            function_callback managerCallback,
            Logger *logger);

private:
    void addTransaction(
            BaseTransaction::Shared transaction);

    void run();

    void launchTransaction(
            BaseTransaction::Shared transaction,
            boost::_bi::bind_t<void, boost::_mfi::mf2<void, TransactionsScheduler, BaseTransaction::Shared, pair<CommandResult::SharedConst, TransactionState::SharedConst>>, boost::_bi::list_av_3<TransactionsScheduler *, std::shared_ptr<BaseTransaction>, std::pair<std::shared_ptr<CommandResult>, std::shared_ptr<TransactionState>>>::type> handler);

    void handleTransactionResult(
            BaseTransaction::Shared transaction,
            pair<CommandResult::SharedConst, TransactionState::SharedConst> result);

    bool isTransactionInScheduler(
            BaseTransaction::Shared transaction);

    void sleepForMinimalTimeout();

    void handleTimeout(
            const boost::system::error_code &error);

};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
