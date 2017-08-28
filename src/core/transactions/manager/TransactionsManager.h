#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../scheduler/TransactionsScheduler.h"

#include "../../common/NodeUUID.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../interface/results_interface/interface/ResultsInterface.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../io/storage/StorageHandler.h"
#include "../../paths/PathsManager.h"
#include "../../logger/Logger.h"
#include "../../cycles/CyclesManager.h"
#include "../../subsystems_controller/SubsystemsController.h"

/*
 * Interface commands
 */
#include "../../interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"

#include "../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"
#include "../../interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"
#include "../../interface/commands_interface/commands/total_balances/TotalBalancesRemouteNodeCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryPaymentsCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"
#include "../../interface/commands_interface/commands/history/HistoryWithContractorCommand.h"
#include "../../interface/commands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommand.h"
#include "../../interface/commands_interface/commands/trust_lines_list/GetTrustLinesCommand.h"
#include "../../interface/commands_interface/commands/trust_lines_list/GetTrustLineCommand.h"

/*
 * Network messages
 */
#include "../../network/messages/Message.hpp"
#include "../../network/messages/response/Response.h"
#include "../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../network/messages/payments/VotesStatusRequestMessage.hpp"

#include "../../resources/manager/ResourcesManager.h"
#include "../../resources/resources/BaseResource.h"

/*
 * Transactions
 */
#include "../transactions/trust_lines/SetOutgoingTrustLineTransaction.h"
#include "../transactions/trust_lines/SetIncomingTrustLineTransaction.h"

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
#include "../transactions/history/HistoryWithContractorTransaction.h"

#include "../transactions/trustlines_list/GetFirstLevelContractorsTransaction.h"
#include "../transactions/trustlines_list/GetFirstLevelContractorsBalancesTransaction.h"
#include "../transactions/trustlines_list/GetFirstLevelContractorBalanceTransaction.h"

#include "../transactions/find_path/FindPathByMaxFlowTransaction.h"

#include <boost/signals2.hpp>

#include <string>

using namespace std;
namespace signals = boost::signals2;


class TransactionsManager {
public:
    signals::signal<void(Message::Shared, const NodeUUID&)> transactionOutgoingMessageReadySignal;

public:
    TransactionsManager(
        NodeUUID &nodeUUID,
        as::io_service &IOService,
        TrustLinesManager *trustLinesManager,
        ResourcesManager *ResourcesManager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        ResultsInterface *resultsInterface,
        StorageHandler *storageHandler,
        PathsManager *pathsManager,
        Logger &logger,
        SubsystemsController *subsystemsController);

    void processCommand(
        BaseUserCommand::Shared command);

    void processMessage(
        Message::Shared message);

    //  Cycles Transactions
    void launchFourNodesCyclesInitTransaction(
        const NodeUUID &debtorUUID,
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

    /**
     * Starts transaction that would orocesses received message
     * and attempts to set incoming trust line from the remote node.
     */
    void launchSetIncomingTrustLineTransaction(
        SetIncomingTrustLineMessage::Shared message);

    /*
     * Max flow transactions
     */
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

    void launchTotalBalancesTransaction(
        InitiateTotalBalancesMessage::Shared message);

    void launchTotalBalancesRemoteNodeTransaction(
        TotalBalancesRemouteNodeCommand::Shared command);

    /*
     * History transactions
     */
    void launchHistoryPaymentsTransaction(
        HistoryPaymentsCommand::Shared command);

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

    void subscribeForBuidCyclesThreeNodesTransaction(
        BasePaymentTransaction::BuildCycleThreeNodesSignal &signal);

    void subscribeForBuidCyclesFourNodesTransaction(
        BasePaymentTransaction::BuildCycleFourNodesSignal &signal);

    void subscribeForBuidCyclesFiveNodesTransaction(
        CyclesManager::BuildFiveNodesCyclesSignal &signal);

    void subscribeForBuidCyclesSixNodesTransaction(
        CyclesManager::BuildSixNodesCyclesSignal &signal);

    void subscribeForCloseCycleTransaction(
        CyclesManager::CloseCycleSignal &signal);

    void subscribeForTryCloseNextCycleSignal(
        TransactionsScheduler::CycleCloserTransactionWasFinishedSignal &signal);

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
        vector<NodeUUID> &contractorsUUID);

    void onBuidCycleFourNodesTransaction(
        vector<pair<NodeUUID, NodeUUID>> &debtorsAndCreditors);

    void onBuidCycleFiveNodesTransaction();

    void onBuidCycleSixNodesTransaction();

    void onCloseCycleTransaction(
        Path::ConstShared cycle);

    void onTryCloseNextCycleSlot();

protected:
    void prepareAndSchedule(
        BaseTransaction::Shared transaction,
        bool regenerateUUID=false,
        bool subsidiaryTransactionSubscribe=false,
        bool outgoingMessagesSubscribe=false);

private:
    NodeUUID &mNodeUUID;
    as::io_service &mIOService;
    TrustLinesManager *mTrustLines;
    ResourcesManager *mResourcesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    ResultsInterface *mResultsInterface;
    PathsManager *mPathsManager;
    StorageHandler *mStorageHandler;
    Logger &mLog;

    SubsystemsController *mSubsystemsController;

    unique_ptr<TransactionsScheduler> mScheduler;
    unique_ptr<CyclesManager> mCyclesManager;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
