#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../../common/Types.h"
#include "../../common/time/TimeUtils.h"

#include "../../network/messages/Message.hpp"
#include "../../network/messages/base/transaction/TransactionMessage.h"
#include "../../network/messages/base/routing_tables/RoutingTablesMessage.h"
#include "../../network/messages/response/Response.h"
#include "../../network/messages/response/RoutingTablesResponse.h"
#include "../../network/messages/cycles/BoundaryNodeTopologyMessage.h"


#include "../transactions/base/BaseTransaction.h"
#include "../transactions/unique/routing_tables/RoutingTablesTransaction.h"
#include "../transactions/result/TransactionResult.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../common/exceptions/RuntimeError.h"
#include "../../logger/Logger.h"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <chrono>
#include <map>
#include <memory>
#include <stdint.h>


using namespace std;

namespace as = boost::asio;
namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class TransactionsScheduler {
public:
    typedef signals::signal<void(CommandResult::SharedConst)> CommandResultSignal;

public:
    TransactionsScheduler(
        as::io_service &IOService,
        storage::UUIDMapBlockStorage *storage,
        Logger *logger);

    void run();

    void scheduleTransaction(
        BaseTransaction::Shared transaction);

    void postponeTransaction(
        BaseTransaction::Shared transaction,
        uint32_t millisecondsDelay);

    void killTransaction(
        const TransactionUUID &transactionUUID);

    // TODO: add error log if unsuccessful
    // TODO: throw exception on failure,
    //       so the communicator would be able to know
    //       if contractor node doesn't follows the protocol.
    void tryAttachMessageToTransaction(
        Message::Shared message);

    friend const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
        TransactionsScheduler *scheduler);

private:
    void launchTransaction(
        BaseTransaction::Shared transaction);

    void handleTransactionResult(
        BaseTransaction::Shared transaction,
        TransactionResult::SharedConst result);

    void processCommandResult(
        BaseTransaction::Shared transaction,
        CommandResult::SharedConst result);

    void processMessageResult(
        BaseTransaction::Shared transaction,
        MessageResult::SharedConst result);

    void processTransactionState(
        BaseTransaction::Shared transaction,
        TransactionState::SharedConst state);

    void serializeTransaction(
        BaseTransaction::Shared transaction);

    void forgetTransaction(
        BaseTransaction::Shared transaction);

    void adjustAwakeningToNextTransaction();

    pair<BaseTransaction::Shared, GEOEpochTimestamp> transactionWithMinimalAwakeningTimestamp() const;

    void asyncWaitUntil(
        GEOEpochTimestamp nextAwakeningTimestamp);

    void handleAwakening(
        const boost::system::error_code &error);

public:
    mutable CommandResultSignal commandResultIsReadySignal;

private:
    as::io_service &mIOService;
    storage::UUIDMapBlockStorage *mStorage;
    Logger *mLog;

    unique_ptr<as::steady_timer> mProcessingTimer;
    unique_ptr<map<BaseTransaction::Shared, TransactionState::SharedConst>> mTransactions;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
