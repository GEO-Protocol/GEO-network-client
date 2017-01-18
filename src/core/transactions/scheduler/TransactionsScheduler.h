#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../../common/Types.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/result/TransactionResult.h"
#include "../../network/messages/response/Response.h"

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
namespace posix_time = boost::posix_time;

typedef boost::function<void(CommandResult::SharedConst)> ManagerCallback;
typedef boost::posix_time::time_duration Timeout;

class TransactionsScheduler {
    // todo: hsc: tests?
public:
    TransactionsScheduler(
        as::io_service &IOService,
        ManagerCallback managerCallback,
        Logger *logger);

    ~TransactionsScheduler();

    void scheduleTransaction(
        BaseTransaction::Shared transaction);

    void run();

    void handleMessage(
        Message::Shared message);

    friend const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
        TransactionsScheduler *scheduler);

private:
    void launchTransaction(
        BaseTransaction::Shared transaction);

    void handleTransactionResult(
        BaseTransaction::Shared transaction,
        TransactionResult::Shared result);

    pair<BaseTransaction::Shared, Timeout> findTransactionWithMinimalTimeout();

    void sleepFor(
        Timeout delay);

    void handleSleep(
        const boost::system::error_code &error,
        Timeout delay);

    bool isTransactionInScheduler(
        BaseTransaction::Shared transaction);

private:
    as::io_service &mIOService;
    ManagerCallback mManagerCallback;
    Logger *mLog;

    as::deadline_timer *mProcessingTimer;
    storage::UUIDMapBlockStorage *mStorage;
    map<BaseTransaction::Shared, TransactionState::SharedConst> *mTransactions;

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
