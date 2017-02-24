#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../../common/Types.h"
#include "../../common/time/TimeUtils.h"

#include "../transactions/base/BaseTransaction.h"
#include "../transactions/result/TransactionResult.h"

#include "../../network/messages/response/Response.h"
#include "../../network/messages/response/RoutingTablesResponse.h"
#include "../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"

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

        // TODO: uint16_t -> uint32_t, otherwise max delay would be 1 minute.
        uint16_t millisecondsDelay);

    // TODO: rename to "routeIncommingMessage"
    void handleMessage(
        Message::Shared message);

    void killTransaction(
        const TransactionUUID &transactionUUID);

    friend const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
        TransactionsScheduler *scheduler);

private:
    void launchTransaction(
        BaseTransaction::Shared transaction);

    void processResponse(
        Response::Shared response);

    void processRoutingTableResponse(
        RoutingTablesResponse::Shared response);

    void processSecondLevelRoutingTableMessage(
        SecondLevelRoutingTableIncomingMessage::Shared message);

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

    bool isTransactionScheduled(
        BaseTransaction::Shared transaction);

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
