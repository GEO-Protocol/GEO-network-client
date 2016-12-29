#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../transactions/BaseTransaction.h"
#include "../../interface/results/result/CommandResult.h"
#include "../transactions/state/TransactionState.h"

#include "../../db/UUIDMapBlockStorage.h"

#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../logger/Logger.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <map>

using namespace std;

namespace as = boost::asio;

namespace storage = db::uuid_map_block_storage;

typedef boost::function<void(CommandResult::SharedConst)> function_callback;
typedef boost::posix_time::time_duration timeout;
typedef storage::byte byte;

namespace posix_time = boost::posix_time;

class TransactionsManager;

class TransactionsScheduler {
    friend class TransactionsManager;

private:
    const timeout kDefaultDelay = posix_time::milliseconds(200);

private:
    as::io_service &mIOService;
    as::deadline_timer *mProcessingTimer;

    function_callback mMangerCallback;

    map<BaseTransaction::Shared, TransactionState::SharedConst> mTransactions;

    storage::UUIDMapBlockStorage *mStorage;

    Logger *mLog;

private:
    TransactionsScheduler(
            as::io_service &IOService,
            function_callback managerCallback,
            Logger *logger);

    ~TransactionsScheduler();
private:
    void addTransaction(
            BaseTransaction::Shared transaction);

    void run();

    void launchTransaction(
            BaseTransaction::Shared transaction);

    void handleTransactionResult(
            BaseTransaction::Shared transaction,
            pair<CommandResult::SharedConst, TransactionState::SharedConst> result);

    pair<BaseTransaction::Shared, timeout> findTransactionWithMinimalTimeout();

    void sleepFor(
            timeout delay);

    void handleSleep(
            const boost::system::error_code &error,
            timeout delay);

    bool isTransactionInScheduler(
            BaseTransaction::Shared transaction);

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
