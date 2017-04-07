#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../interface/results_interface/interface/ResultsInterface.h"
#include "../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../io/storage/StorageHandler.h"
#include "../../paths/PathsManager.h"
#include "../../logger/Logger.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"
#include "../scheduler/TransactionsScheduler.h"

#include "../../interface/commands_interface/commands/BaseUserCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"
#include "../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"
#include "../../interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"
#include "../../interface/commands_interface/commands/total_balances/TotalBalancesRemouteNodeCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryPaymentsCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"
#include "../../interface/commands_interface/commands/find_path/FindPathCommand.h"

#include "../../network/messages/Message.hpp"
#include "../../network/messages/incoming/trust_lines/AcceptTrustLineMessage.h"
#include "../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"
#include "../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../network/messages/response/Response.h"

#include "../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"

#include "../../resources/resources/BaseResource.h"

#include "../transactions/base/BaseTransaction.h"
#include "../transactions/base/UniqueTransaction.h"

#include "../transactions/unique/trust_lines/OpenTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/AcceptTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/CloseTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/RejectTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/SetTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/UpdateTrustLineTransaction.h"
#include "../transactions/unique/routing_tables/propagate/FromInitiatorToContractorRoutingTablesPropagationTransaction.h"
#include "../transactions/unique/routing_tables/accept/FromInitiatorToContractorRoutingTablesAcceptTransaction.h"
#include "../transactions/unique/routing_tables/accept/FromContractorToFirstLevelRoutingTablesAcceptTransaction.h"
#include "../transactions/unique/routing_tables/accept/FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction.h"
#include "../transactions/unique/routing_tables/update/RoutingTablesUpdateTransactionsFactory.h"
#include "../transactions/unique/routing_tables/update/AcceptRoutingTablesUpdatesTransaction.h"

#include "../transactions/cycles/FiveAndSixNodes/CyclesFiveNodesInitTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesSixNodesInitTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesFiveNodesReceiverTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesSixNodesReceiverTransaction.h"

#include "../transactions/cycles/ThreeNodes/CyclesThreeNodesInitTransaction.h"
#include "../transactions/cycles/ThreeNodes/CyclesThreeNodesReceiverTransaction.h"


#include "../transactions/regular/payments/CoordinatorPaymentTransaction.h"
#include "../transactions/regular/payments/ReceiverPaymentTransaction.h"
#include "../transactions/regular/payments/IntermediateNodePaymentTransaction.h"

#include "../transactions/max_flow_calculation/InitiateMaxFlowCalculationTransaction.h"
#include "../transactions/max_flow_calculation/ReceiveMaxFlowCalculationOnTargetTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationSourceFstLevelTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationTargetFstLevelTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationSourceSndLevelTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationTargetSndLevelTransaction.h"
#include "../transactions/max_flow_calculation/ReceiveResultMaxFlowCalculationTransaction.h"

#include "../transactions/total_balances/TotalBalancesTransaction.h"
#include "../transactions/total_balances/TotalBalancesFromRemoutNodeTransaction.h"

#include "../transactions/history/HistoryPaymentsTransaction.h"
#include "../transactions/history/HistoryTrustLinesTransaction.h"

#include "../transactions/find_path/FindPathTransaction.h"
#include "../transactions/find_path/GetRoutingTablesTransaction.h"

#include <boost/signals2.hpp>

#include <string>

using namespace std;
namespace storage = db::uuid_map_block_storage;
namespace history = db::operations_history_storage;
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
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        ResultsInterface *resultsInterface,
        history::OperationsHistoryStorage *operationsHistoryStorage,
        StorageHandler *storageHandler,
        PathsManager *pathsManager,
        Logger *logger);

    void processCommand(
        BaseUserCommand::Shared command);

    void processMessage(
        Message::Shared message);

    // Invokes from Core
    void launchFromInitiatorToContractorRoutingTablePropagationTransaction(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

    //  Cycles Transactions
// -------------------------------------------------------------------------------
//    void launchGetTopologyAndBalancesTransaction(InBetweenNodeTopologyMessage::Shared message);
//
//    void launchGetTopologyAndBalancesTransactionFiveNodes();
//    void launchGetTopologyAndBalancesTransactionSixNodes();
//
//    void launchGetThreeNodesNeighborBalancesTransaction(NodeUUID &contractorUUID);
//    void launchGetThreeNodesNeighborBalancesTransaction(CyclesThreeNodesBalancesRequestMessage::Shared message);
//
//    void launchGetFourNodesNeighborBalancesTransaction(NodeUUID &contractorUUID);
//    void launchGetFourNodesNeighborBalancesTransaction(CyclesFourNodesBalancesRequestMessage::Shared message);

    void launchThreeNodesCyclesInitTransaction(const NodeUUID &contractorUUID);
    void launchThreeNodesCyclesResponseTransaction(CyclesThreeNodesBalancesRequestMessage::Shared message);

    void launchSixNodesCyclesInitTransaction();
    void launchSixNodesCyclesResponseTransaction(CyclesSixNodesInBetweenMessage::Shared message);

    void launchFiveNodesCyclesInitTransaction();
    void launchFiveNodesCyclesResponseTransaction(CyclesFiveNodesInBetweenMessage::Shared message);

//    ----------------------------------------------------------

    void launchRoutingTablesUpdatingTransactionsFactory(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

    void launchPathsResourcesCollectTransaction(
        const TransactionUUID &requestedTransactionUUID,
        const NodeUUID &destinationNodeUUID);

    void attachResourceToTransaction(
        BaseResource::Shared resource);

private:
    // Transactions from storage
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

    // Routing tables transactions
    void launchAcceptRoutingTablesTransaction(
        FirstLevelRoutingTableIncomingMessage::Shared message);

    void launchAcceptRoutingTablesUpdatesTransaction(
        RoutingTableUpdateIncomingMessage::Shared message);

    // Max flow transactions
    void launchInitiateMaxFlowCalculatingTransaction(
        InitiateMaxFlowCalculationCommand::Shared command);

    void launchReceiveMaxFlowCalculationOnTargetTransaction(
            InitiateMaxFlowCalculationMessage::Shared message);

    void launchReceiveResultMaxFlowCalculationTransaction(
        ResultMaxFlowCalculationMessage::Shared message);

    void launchMaxFlowCalculationSourceFstLevelTransaction(
        MaxFlowCalculationSourceFstLevelMessage::Shared message);

    void launchMaxFlowCalculationTargetFstLevelTransaction(
        MaxFlowCalculationTargetFstLevelMessage::Shared message);

    void launchMaxFlowCalculationSourceSndLevelTransaction(
        MaxFlowCalculationSourceSndLevelMessage::Shared message);

    void launchMaxFlowCalculationTargetSndLevelTransaction(
        MaxFlowCalculationTargetSndLevelMessage::Shared message);

    // Payment transactions
    void launchCoordinatorPaymentTransaction(
        CreditUsageCommand::Shared command);

    void launchReceiverPaymentTransaction(
        ReceiverInitPaymentRequestMessage::Shared message);

    void launchIntermediateNodePaymentTransaction(
        IntermediateNodeReservationRequestMessage::Shared message);

    // Total balances transaction
    void launchTotalBalancesTransaction(
            TotalBalancesCommand::Shared command);

    void launchTotalBalancesTransaction(
        InitiateTotalBalancesMessage::Shared message);

    void launchTotalBalancesRemoteNodeTransaction(
        TotalBalancesRemouteNodeCommand::Shared command);

    // History transactions
    void launchHistoryPaymentsTransaction(
        HistoryPaymentsCommand::Shared command);

    void launchHistoryTrustLinesTransaction(
        HistoryTrustLinesCommand::Shared command);

    // Find path transactions
    void launchFindPathTransaction(
        FindPathCommand::Shared command);

    void launchGetRoutingTablesTransaction(
        RequestRoutingTablesMessage::Shared message);

    // Signals connection to manager's slots
    void subscribeForSubsidiaryTransactions(
        BaseTransaction::LaunchSubsidiaryTransactionSignal &signal);

    void subscribeForOutgoingMessages(
        BaseTransaction::SendMessageSignal &signal);

    void subscribeForCommandResult(
        TransactionsScheduler::CommandResultSignal &signal);

    // Slots
    void onSubsidiaryTransactionReady(
        BaseTransaction::Shared transaction);

    void onTransactionOutgoingMessageReady(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void onCommandResultReady(
        CommandResult::SharedConst result);

    void prepareAndSchedule(
        BaseTransaction::Shared transaction);

private:
    NodeUUID &mNodeUUID;
    as::io_service &mIOService;
    TrustLinesManager *mTrustLines;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    ResultsInterface *mResultsInterface;
    history::OperationsHistoryStorage *mOperationsHistoryStorage;
    PathsManager *mPathsManager;
    StorageHandler *mStorageHandler;
    Logger *mLog;

    unique_ptr<storage::UUIDMapBlockStorage> mStorage;
    unique_ptr<TransactionsScheduler> mScheduler;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
