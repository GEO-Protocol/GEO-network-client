#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../../common/Types.h"
#include "../../common/time/TimeUtils.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/result/TransactionResult.h"
#include "../../network/messages/response/Response.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../common/exceptions/RuntimeError.h"
#include "../../logger/Logger.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <map>
#include <memory>


using namespace std;

namespace as = boost::asio;
namespace storage = db::uuid_map_block_storage;

// todo: (DM) is it really must be pubic for all the components?
typedef boost::function<void(CommandResult::SharedConst)> ManagerCallback;

// todo: hsc: thnk how to implement transactions uniqueness in scheduler logic.
class TransactionsScheduler {
public:
    TransactionsScheduler(
        as::io_service &IOService,
        storage::UUIDMapBlockStorage *storage,
        ManagerCallback managerCallback,
        Logger *logger);

    // todo: (DM) remove this in favour of unique_ptr
    ~TransactionsScheduler();

    void run();

    void scheduleTransaction(
        BaseTransaction::Shared transaction);

    void postponeRoutingTableTransaction(
        BaseTransaction::Shared transaction);

    // todo: (DM) is it really must be public?
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

    void forgetTransaction(
        BaseTransaction::Shared transaction);

    void serializeTransaction(
        BaseTransaction::Shared transaction);

    void processNextTransactions();

    // todo: it should be removed in favour of transactionWithMinimalAwakeningTimestamp();
    pair<BaseTransaction::Shared, Duration> nextDelayedTransaction();

    pair<BaseTransaction::Shared, TransactionState::AwakeTimestamp> transactionWithMinimalAwakeningTimestamp() const;

    void adjustAwakeningToNextTransaction();

    // todo: (DM) remove this (see asyncWaitUntil);
    // todo: (DM) this is renamed sleepFor()
    void rescheduleNextInterruption(
        Duration delay);

    void asyncWaitUntil(
        TransactionState::AwakeTimestamp nextAwakeningTimestamp);

    void handleAwakening(
        const boost::system::error_code &error);

    bool isTransactionInScheduler(
        BaseTransaction::Shared transaction);

    MicrosecondsTimestamp now() const;

private:
    const uint64_t kPostponeMillisecondsTime = 500;

    as::io_service &mIOService;
    storage::UUIDMapBlockStorage *mStorage;
    ManagerCallback mManagerCallback;
    Logger *mLog;

    as::deadline_timer *mProcessingTimer; // todo: make unique_ptr
    map<BaseTransaction::Shared, TransactionState::SharedConst> *mTransactions; // todo: make unique_ptr
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
