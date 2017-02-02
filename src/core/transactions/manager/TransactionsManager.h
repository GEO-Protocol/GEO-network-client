#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../common/NodeUUID.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../interface/results_interface/interface/ResultsInterface.h"
#include "../../logger/Logger.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"
#include "../scheduler/TransactionsScheduler.h"

#include "../../interface/commands_interface/commands/BaseUserCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"

#include "../../network/messages/Message.h"
#include "../../network/messages/incoming/trust_lines/AcceptTrustLineMessage.h"
#include "../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"
#include "../../network/messages/response/Response.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/unique/UniqueTransaction.h"
#include "../transactions/unique/trust_lines/OpenTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/AcceptTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/CloseTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/RejectTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/SetTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/UpdateTrustLineTransaction.h"
#include "../transactions/unique/routing_tables/SendRoutingTablesTransaction.h"

#include <boost/signals2.hpp>

#include <string>

using namespace std;
namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class TransactionsManager {
    // todo: hsc: tests?
public:
    signals::signal<void(Message::Shared, const NodeUUID&)> sendMessageSignal;

public:
    TransactionsManager(
        NodeUUID &nodeUUID,
        as::io_service &IOService,
        TrustLinesManager *trustLinesManager,
        ResultsInterface *resultsInterface,
        Logger *logger);

    ~TransactionsManager();

    void processCommand(
        BaseUserCommand::Shared command);

    void processMessage(
        Message::Shared message);

    void acceptCommandResult(
        CommandResult::SharedConst result);

    void startRoutingTablesExchange(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

private:
    void loadTransactions();

    void createOpenTrustLineTransaction(
        BaseUserCommand::Shared command);

    void createAcceptTrustLineTransaction(
        Message::Shared message);

    void createCloseTrustLineTransaction(
        BaseUserCommand::Shared command);

    void createRejectTrustLineTransaction(
        Message::Shared message);

    void createSetTrustLineTransaction(
        BaseUserCommand::Shared command);

    void createUpdateTrustLineTransaction(
        Message::Shared message);

    void onMessageSend(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void zeroPointers();

    void cleanupMemory();

private:
    NodeUUID &mNodeUUID;
    as::io_service &mIOService;
    TrustLinesManager *mTrustLinesManager;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

    storage::UUIDMapBlockStorage *mStorage;
    TransactionsScheduler *mTransactionsScheduler;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
