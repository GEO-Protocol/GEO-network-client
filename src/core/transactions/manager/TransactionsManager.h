#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../logger/Logger.h"

#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../interface/results_interface/interface/ResultsInterface.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"
#include "../scheduler/TransactionsScheduler.h"

#include "../../interface/commands_interface/commands/BaseUserCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"
#include "../../interface/commands_interface/commands/payments/CreditUsageCommand.h"

#include "../../network/messages/Message.hpp"
#include "../../network/messages/incoming/trust_lines/AcceptTrustLineMessage.h"
#include "../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"
#include "../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../network/messages/response/Response.h"

#include "../transactions/base/BaseTransaction.h"
#include "../transactions/base/UniqueTransaction.h"
#include "../transactions/unique/trust_lines/OpenTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/AcceptTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/CloseTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/RejectTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/SetTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/UpdateTrustLineTransaction.h"
#include "../transactions/unique/routing_tables/FromInitiatorToContractorRoutingTablePropagationTransaction.h"
#include "../transactions/unique/routing_tables/FromInitiatorToContractorRoutingTablesAcceptTransaction.h"
#include "../transactions/regular/payments/CoordinatorPaymentTransaction.h"
#include "../transactions/regular/payments/ReceiverPaymentTransaction.h"

#include <boost/signals2.hpp>

#include <string>

using namespace std;
namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class TransactionsManager {
    // todo: hsc: tests?
public:
    signals::signal<void(Message::Shared, const NodeUUID&)> transactionOutgoingMessageReadySignal;

public:
    TransactionsManager(
        NodeUUID &nodeUUID,
        as::io_service &IOService,
        TrustLinesManager *trustLinesManager,
        ResultsInterface *resultsInterface,
        Logger *logger);

    void processCommand(
        BaseUserCommand::Shared command);

    void processMessage(
        Message::Shared message);

    // Invokes from Core
    void launchFromInitiatorToContractorRoutingTablePropagationTransaction(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

private:
    void loadTransactions();

    // Trust line transactions
    void launchOpenTrustLineTransaction(
        OpenTrustLineCommand::Shared command);

    void launchSetTrustLineTransaction(
        SetTrustLineCommand::Shared command);

    void launchCloseTrustLineTransaction(
        CloseTrustLineCommand::Shared command);

    void launchAcceptTrustLineTransaction(
        AcceptTrustLineMessage::Shared message);

    void launchUpdateTrustLineTransaction(
        UpdateTrustLineMessage::Shared message);

    void launchRejectTrustLineTransaction(
        RejectTrustLineMessage::Shared message);

    // Payment transactions
    void launchCoordinatorPaymentTransaction(
        CreditUsageCommand::Shared command);

    void launchReceiverPaymentTransaction(
        ReceiverInitPaymentMessage::Shared message);

    void launchAcceptFromInitiatorToContractorRoutingTablesTransaction(
        FirstLevelRoutingTableIncomingMessage::Shared message);

    // Signals connection to manager's slots
    void subscribeForOutgoingMessages(
        BaseTransaction::SendMessageSignal &signal);

    void subscribeForCommandResult(
        TransactionsScheduler::CommandResultSignal &signal);

    // Slots
    void onTransactionOutgoingMessageReady(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void onCommandResultReady(
        CommandResult::SharedConst result);

private:
    NodeUUID &mNodeUUID;
    as::io_service &mIOService;
    TrustLinesManager *mTrustLines;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

    unique_ptr<storage::UUIDMapBlockStorage> mStorage;
    unique_ptr<TransactionsScheduler> mScheduler;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
