#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../scheduler/TransactionsScheduler.h"

#include "../../common/NodeUUID.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../interface/results_interface/interface/ResultsInterface.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../topology/manager/TopologyTrustLineManager.h"
#include "../../topology/cashe/TopologyCacheManager.h"
#include "../../topology/cashe/MaxFlowCacheManager.h"
#include "../../io/storage/StorageHandler.h"
#include "../../paths/PathsManager.h"
#include "../../logger/Logger.h"
#include "../../cycles/CyclesManager.h"
#include "../../subsystems_controller/SubsystemsController.h"

/*
 * Interface commands
 */
#include "../../interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"
#include "../../interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"
#include "../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"
#include "../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationFullyCommand.h"
#include "../../interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryPaymentsCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryAdditionalPaymentsCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryAdditionalPaymentsCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryWithContractorCommand.h"
#include "../../interface/commands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommand.h"
#include "../../interface/commands_interface/commands/trust_lines_list/GetTrustLinesCommand.h"
#include "../../interface/commands_interface/commands/trust_lines_list/GetTrustLineCommand.h"
#include "../../interface/commands_interface/commands/blacklist/AddNodeToBlackListCommand.h"
#include "../../interface/commands_interface/commands/blacklist/CheckIfNodeInBlackListCommand.h"
#include "../../interface/commands_interface/commands/blacklist/RemoveNodeFromBlackListCommand.h"
#include "../../interface/commands_interface/commands/blacklist/GetBlackListCommand.h"
#include "../../interface/commands_interface/commands/transactions/PaymentTransactionByCommandUUIDCommand.h"

/*
 * Network messages
 */
#include "../../network/messages/Message.hpp"
#include "../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../network/messages/routing_table/RoutingTableRequestMessage.h"

#include "../../resources/manager/ResourcesManager.h"
#include "../../resources/resources/BaseResource.h"

/*
 * Transactions
 */
#include "../transactions/trust_lines/SetOutgoingTrustLineTransaction.h"
#include "../transactions/trust_lines/SetIncomingTrustLineTransaction.h"
#include "../transactions/trust_lines/CloseIncomingTrustLineTransaction.h"
#include "../transactions/trust_lines/CloseOutgoingTrustLineTransaction.h"
#include "../transactions/trust_lines/RejectOutgoingTrustLineTransaction.h"

#include "../transactions/cycles/ThreeNodes/CyclesThreeNodesInitTransaction.h"
#include "../transactions/cycles/ThreeNodes/CyclesThreeNodesReceiverTransaction.h"
#include "../transactions/cycles/FourNodes/CyclesFourNodesInitTransaction.h"
#include "../transactions/cycles/FourNodes/CyclesFourNodesReceiverTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesFiveNodesInitTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesFiveNodesReceiverTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesSixNodesInitTransaction.h"
#include "../transactions/cycles/FiveAndSixNodes/CyclesSixNodesReceiverTransaction.h"

#include "../transactions/regular/payments/CoordinatorPaymentTransaction.h"
#include "../transactions/regular/payments/ReceiverPaymentTransaction.h"
#include "../transactions/regular/payments/IntermediateNodePaymentTransaction.h"
#include "../transactions/regular/payments/VotesStatusResponsePaymentTransaction.h"
#include "../transactions/regular/payments/CycleCloserInitiatorTransaction.h"
#include "../transactions/regular/payments/CycleCloserIntermediateNodeTransaction.h"


#include "../transactions/max_flow_calculation/InitiateMaxFlowCalculationTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationFullyTransaction.h"
#include "../transactions/max_flow_calculation/ReceiveMaxFlowCalculationOnTargetTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationSourceFstLevelTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationTargetFstLevelTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationSourceSndLevelTransaction.h"
#include "../transactions/max_flow_calculation/MaxFlowCalculationTargetSndLevelTransaction.h"
#include "../transactions/max_flow_calculation/ReceiveResultMaxFlowCalculationTransaction.h"

#include "../transactions/total_balances/TotalBalancesTransaction.h"

#include "../transactions/history/HistoryPaymentsTransaction.h"
#include "../transactions/history/HistoryAdditionalPaymentsTransaction.h"
#include "../transactions/history/HistoryTrustLinesTransaction.h"
#include "../transactions/history/HistoryWithContractorTransaction.h"

#include "../transactions/trustlines_list/GetFirstLevelContractorsTransaction.h"
#include "../transactions/trustlines_list/GetFirstLevelContractorsBalancesTransaction.h"
#include "../transactions/trustlines_list/GetFirstLevelContractorBalanceTransaction.h"

#include "../transactions/routing_table/RoutingTableInitTransaction.h"
#include "../transactions/routing_table/RoutingTableResponseTransaction.h"

#include "../transactions/blacklist/AddNodeToBlackListTransaction.h"
#include "../transactions/blacklist/CheckIfNodeInBlackListTransaction.h"
#include "../transactions/blacklist/RemoveNodeFromBlackListTransaction.h"
#include "../transactions/blacklist/GetBlackListTransaction.h"


#include "../transactions/find_path/FindPathByMaxFlowTransaction.h"

#include "../transactions/transaction/PaymentTransactionByCommandUUIDTransaction.h"

#include "../transactions/gateway_notification/GatewayNotificationSenderTransaction.h"
#include "../transactions/gateway_notification/GatewayNotificationReceiverTransaction.h"

#include <boost/signals2.hpp>

#include <string>

using namespace std;
namespace signals = boost::signals2;


class TransactionsManager {
public:
    signals::signal<void(Message::Shared, const NodeUUID&)> transactionOutgoingMessageReadySignal;
    signals::signal<void(const NodeUUID&, ConfirmationMessage::Shared)> ProcessConfirmationMessageSignal;

public:
    TransactionsManager(
        NodeUUID &nodeUUID,
        as::io_service &IOService,
        TrustLinesManager *trustLinesManager,
        ResourcesManager *ResourcesManager,
        TopologyTrustLineManager *topologyTrustLineManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        ResultsInterface *resultsInterface,
        StorageHandler *storageHandler,
        PathsManager *pathsManager,
        RoutingTableManager *routingTable,
        Logger &logger,
        SubsystemsController *subsystemsController,
        bool iAmGateway);

    void processCommand(
        BaseUserCommand::Shared command);

    void processMessage(
        Message::Shared message);

    //  Cycles Transactions
    void launchFourNodesCyclesInitTransaction(
        const NodeUUID &creditorUUID);

    void launchFourNodesCyclesResponseTransaction(
        CyclesFourNodesBalancesRequestMessage::Shared message);

    void launchThreeNodesCyclesInitTransaction(
        const NodeUUID &contractorUUID);

    void launchThreeNodesCyclesResponseTransaction(
        CyclesThreeNodesBalancesRequestMessage::Shared message);

    void launchSixNodesCyclesInitTransaction();

    void launchSixNodesCyclesResponseTransaction(
        CyclesSixNodesInBetweenMessage::Shared message);

    void launchFiveNodesCyclesInitTransaction();

    void launchFiveNodesCyclesResponseTransaction(
        CyclesFiveNodesInBetweenMessage::Shared message);

    // Resources transactions handlers
    void attachResourceToTransaction(
        BaseResource::Shared resource);

    void launchFindPathByMaxFlowTransaction(
        const TransactionUUID &requestedTransactionUUID,
        const NodeUUID &destinationNodeUUID);

    void launchGatewayNotificationSenderTransaction();

#ifdef TESTS
    void setMeAsGateway();
#endif

protected:
    void loadTransactionsFromStorage();

protected: // Transactions
    /*
     * Trust lines transactions
     */

    /**
     * Starts transaction that would processes locally received command
     * and try to set outgoing trust line to the remote node.
     */
    void launchSetOutgoingTrustLineTransaction(
        SetOutgoingTrustLineCommand::Shared command);

    void launchCloseIncomingTrustLineTransaction(
        CloseIncomingTrustLineCommand::Shared command);

    /**
     * Starts transaction that would processes received message
     * and attempts to set incoming trust line from the remote node.
     */
    void launchSetIncomingTrustLineTransaction(
        SetIncomingTrustLineMessage::Shared message);

    void launchSetIncomingTrustLineTransaction(
        SetIncomingTrustLineFromGatewayMessage::Shared message);

    void launchCloseOutgoingTrustLineTransaction(
        CloseOutgoingTrustLineMessage::Shared message);

    void launchRejectOutgoingTrustLineTransaction(
        ConfirmationMessage::Shared message);

    /*
     * Max flow transactions
     */
    void launchInitiateMaxFlowCalculatingTransaction(
        InitiateMaxFlowCalculationCommand::Shared command);

    void launchMaxFlowCalculationFullyTransaction(
        InitiateMaxFlowCalculationFullyCommand::Shared command);

    void launchReceiveMaxFlowCalculationOnTargetTransaction(
        InitiateMaxFlowCalculationMessage::Shared message);

    void launchReceiveResultMaxFlowCalculationTransaction(
        ResultMaxFlowCalculationMessage::Shared message);

    void launchReceiveResultMaxFlowCalculationTransactionFromGateway(
        ResultMaxFlowCalculationGatewayMessage::Shared message);

    void launchMaxFlowCalculationSourceFstLevelTransaction(
        MaxFlowCalculationSourceFstLevelMessage::Shared message);

    void launchMaxFlowCalculationTargetFstLevelTransaction(
        MaxFlowCalculationTargetFstLevelMessage::Shared message);

    void launchMaxFlowCalculationSourceSndLevelTransaction(
        MaxFlowCalculationSourceSndLevelMessage::Shared message);

    void launchMaxFlowCalculationTargetSndLevelTransaction(
        MaxFlowCalculationTargetSndLevelMessage::Shared message);

    /*
     * Payment transactions
     */
    void launchCoordinatorPaymentTransaction(
        CreditUsageCommand::Shared command);

    void launchReceiverPaymentTransaction(
        ReceiverInitPaymentRequestMessage::Shared message);

    void launchIntermediateNodePaymentTransaction(
        IntermediateNodeReservationRequestMessage::Shared message);

    void launchCycleCloserIntermediateNodeTransaction(
        IntermediateNodeCycleReservationRequestMessage::Shared message);

    void launchVotesResponsePaymentsTransaction(
        VotesStatusRequestMessage::Shared message);

    /*
     * Total balances transaction
     */
    void launchTotalBalancesTransaction(
        TotalBalancesCommand::Shared command);

    /*
     * History transactions
     */
    void launchHistoryPaymentsTransaction(
        HistoryPaymentsCommand::Shared command);

    void launchAdditionalHistoryPaymentsTransaction(
        HistoryAdditionalPaymentsCommand::Shared command);

    void launchHistoryTrustLinesTransaction(
        HistoryTrustLinesCommand::Shared command);

    void launchHistoryWithContractorTransaction(
        HistoryWithContractorCommand::Shared command);

    /*
     * Find path transactions
     */
    void launchGetFirstLevelContractorsTransaction(
        GetFirstLevelContractorsCommand::Shared command);

    void launchGetTrustlinesTransaction(
        GetTrustLinesCommand::Shared command);

    void launchGetTrustlineTransaction(
        GetTrustLineCommand::Shared command);

    /*
     * BlackList
     */
public:
    void launchAddNodeToBlackListTransaction(
        AddNodeToBlackListCommand::Shared command);

    void launchCheckIfNodeInBlackListTransaction(
        CheckIfNodeInBlackListCommand::Shared command);

    void launchRemoveNodeFromBlackListTransaction(
        RemoveNodeFromBlackListCommand::Shared command);

    void launchGetBlackListTransaction(
        GetBlackListCommand::Shared command);

    /*
     * Transaction
     */
    void launchPaymentTransactionByCommandUUIDTransaction(
        PaymentTransactionByCommandUUIDCommand::Shared command);

    /*
     * RoutingTable
     */
    void launchRoutingTableResponseTransaction(
        RoutingTableRequestMessage::Shared message);
public:
    void launchRoutingTableRequestTransaction();

protected:
    /*
     * Gateway notification transactions
     */
    void launchGatewayNotificationReceiverTransaction(
        GatewayNotificationMessage::Shared message);

protected:
    // Signals connection to manager's slots
    void subscribeForSubsidiaryTransactions(
        BaseTransaction::LaunchSubsidiaryTransactionSignal &signal);

    void subscribeForOutgoingMessages(
        BaseTransaction::SendMessageSignal &signal);

    void subscribeForCommandResult(
        TransactionsScheduler::CommandResultSignal &signal);

    void subscribeForSerializeTransaction(
        TransactionsScheduler::SerializeTransactionSignal &signal);

    void subscribeForBuildCyclesThreeNodesTransaction(
        BasePaymentTransaction::BuildCycleThreeNodesSignal &signal);

    void subscribeForBuildCyclesFourNodesTransaction(
        BasePaymentTransaction::BuildCycleFourNodesSignal &signal);

    void subscribeForBuildCyclesFiveNodesTransaction(
        CyclesManager::BuildFiveNodesCyclesSignal &signal);

    void subscribeForBuildCyclesSixNodesTransaction(
        CyclesManager::BuildSixNodesCyclesSignal &signal);

    void subscribeForCloseCycleTransaction(
        CyclesManager::CloseCycleSignal &signal);

    void subscribeForTryCloseNextCycleSignal(
        TransactionsScheduler::CycleCloserTransactionWasFinishedSignal &signal);

    void subscribeForProcessingConfirmationMessage(
        BaseTransaction::ProcessConfirmationMessageSignal &signal);

    // Slots
    void onSubsidiaryTransactionReady(
        BaseTransaction::Shared transaction);

    void onTransactionOutgoingMessageReady(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void onCommandResultReady(
        CommandResult::SharedConst result);

    void onSerializeTransaction(
        BaseTransaction::Shared transaction);

    void onBuidCycleThreeNodesTransaction(
        set<NodeUUID> &contractorsUUID);

    void onBuildCycleFourNodesTransaction(
        set<NodeUUID> &creditors);

    void onBuildCycleFiveNodesTransaction();

    void onBuildCycleSixNodesTransaction();

    void onCloseCycleTransaction(
        Path::ConstShared cycle);

    void onTryCloseNextCycleSlot();

    void onProcessConfirmationMessageSlot(
        const NodeUUID &contractorUUID,
        ConfirmationMessage::Shared confirmationMessage);

protected:
    void prepareAndSchedule(
        BaseTransaction::Shared transaction,
        bool regenerateUUID=false,
        bool subsidiaryTransactionSubscribe=false,
        bool outgoingMessagesSubscribe=false);

protected:
    static string logHeader()
    noexcept;

    LoggerStream error() const
    noexcept;

    LoggerStream warning() const
    noexcept;

    LoggerStream info() const
    noexcept;

private:
    NodeUUID &mNodeUUID;
    bool mIAmGateway;
    as::io_service &mIOService;
    TrustLinesManager *mTrustLines;
    ResourcesManager *mResourcesManager;
    TopologyTrustLineManager *mTopologyTrustLineManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    ResultsInterface *mResultsInterface;
    PathsManager *mPathsManager;
    StorageHandler *mStorageHandler;
    RoutingTableManager *mRoutingTable;
    Logger &mLog;

    SubsystemsController *mSubsystemsController;

    unique_ptr<TransactionsScheduler> mScheduler;
    unique_ptr<CyclesManager> mCyclesManager;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
