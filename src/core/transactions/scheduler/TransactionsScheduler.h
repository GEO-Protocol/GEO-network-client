#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../../common/Types.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/result/TransactionResult.h"
#include "../../network/messages/response/Response.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../logger/Logger.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <map>
#include <memory>


using namespace std;

namespace as = boost::asio;
namespace storage = db::uuid_map_block_storage;

typedef boost::function<void(CommandResult::SharedConst)> ManagerCallback;

class TransactionsScheduler {
    // todo: hsc: tests?
public:
    TransactionsScheduler(
        as::io_service &IOService,
        storage::UUIDMapBlockStorage *storage,
        ManagerCallback managerCallback,
        Logger *logger);

    ~TransactionsScheduler();

    void run();

    void scheduleTransaction(
        BaseTransaction::Shared transaction);

    void postponeRoutingTableTransaction(
        BaseTransaction::Shared transaction);

    void killTransaction(
        const TransactionUUID &transactionUUID);

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

    pair<BaseTransaction::Shared, Duration> findTransactionWithMinimalTimeout();

    void sleepFor(
        Duration delay);

    void handleSleep(
        const boost::system::error_code &error,
        Duration delay);

    bool isTransactionInScheduler(
        BaseTransaction::Shared transaction);

private:
    const uint64_t kPostponeMillisecondsTime = 500;

    as::io_service &mIOService;
    storage::UUIDMapBlockStorage *mStorage;
    ManagerCallback mManagerCallback;
    Logger *mLog;

    as::deadline_timer *mProcessingTimer;
    map<BaseTransaction::Shared, TransactionState::SharedConst> *mTransactions;

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
