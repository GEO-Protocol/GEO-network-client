#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../../common/time/TimeUtils.h"

#include "../../network/messages/Message.hpp"
#include "../../network/messages/base/transaction/TransactionMessage.h"
#include "../../network/messages/response/Response.h"


#include "../transactions/base/BaseTransaction.h"
#include "../transactions/result/TransactionResult.h"

#include "../../resources/resources/BaseResource.h"

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
namespace signals = boost::signals2;


class TransactionsScheduler {
public:
    typedef signals::signal<void(CommandResult::SharedConst)> CommandResultSignal;
    typedef signals::signal<void(BaseTransaction::Shared)> SerializeTransactionSignal;
    typedef signals::signal<void()> CycleCloserTransactionWasFinishedSignal;


public:
    TransactionsScheduler(
        as::io_service &IOService,
        Logger &logger);

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

    void tryAttachResourceToTransaction(
        BaseResource::Shared resource);

    friend const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
        TransactionsScheduler *scheduler);

    void addTransactionAndState(BaseTransaction::Shared transaction, TransactionState::SharedConst state);

    const BaseTransaction::Shared transactionByUUID(
        const TransactionUUID &transactionUUID) const;

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
    mutable SerializeTransactionSignal serializeTransactionSignal;
    // this signal used for notofication of CyclesMAnager that CycleCloser transaction was finished
    // and it can try launch new CycleCloser transaction
    mutable CycleCloserTransactionWasFinishedSignal cycleCloserTransactionWasFinishedSignal;

private:
    as::io_service &mIOService;
    Logger &mLog;

    unique_ptr<as::steady_timer> mProcessingTimer;
    unique_ptr<map<BaseTransaction::Shared, TransactionState::SharedConst>> mTransactions;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
