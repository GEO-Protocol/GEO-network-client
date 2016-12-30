#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../transactions/BaseTransaction.h"
#include "../../interface/results/result/CommandResult.h"
#include "../transactions/state/TransactionState.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

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


// todo: all this masks must be private for the TransactionsScheduler
// todo: what is the purpose of this callback? can it be obvious or commented?
typedef boost::function<void(CommandResult::SharedConst)> function_callback; // todo: CamelCase?
typedef boost::posix_time::time_duration timeout; // todo: CamelCase?
typedef storage::byte byte; // see Types.h

namespace posix_time = boost::posix_time;


class TransactionsManager;

class TransactionsScheduler {
    // todo: hsc: tests?

    // todo: please, no friend-based OOP
    friend class TransactionsManager;

    // public members must be at the top, and then - private
private:
    // there is no default delay should be present in the system
    const timeout kDefaultDelay = posix_time::milliseconds(200);

private:
    // todo: separate internal and external objects and pointers
    // may be there some order?
    as::io_service &mIOService;
    as::deadline_timer *mProcessingTimer;


    function_callback mMangerCallback; // todo: man(A)gerCallback?

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
    void addTransaction( // todo: rename to scheduleTransaction()
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
