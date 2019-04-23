
#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "common/Types.h"

#include "settings/Settings.h"
#include "network/communicator/Communicator.h"
#include "interface/commands_interface/interface/CommandsInterface.h"
#include "interface/results_interface/interface/ResultsInterface.h"
#include "interface/events_interface/interface/EventsInterface.h"
#include "resources/manager/ResourcesManager.h"
#include "transactions/manager/TransactionsManager.h"
#include "io/storage/StorageHandler.h"
#include "equivalents/EquivalentsSubsystemsRouter.h"
#include "crypto/keychain.h"
#include "contractors/ContractorsManager.h"
#include "observing/ObservingHandler.h"
#include "delayed_tasks/TopologyEventDelayedTask.h"
#include "features/FeaturesManager.h"

#include "logger/Logger.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include "subsystems_controller/SubsystemsController.h"
#include "subsystems_controller/TrustLinesInfluenceController.h"

#include <sodium.h>
#include <sys/prctl.h>


using namespace std;
using namespace crypto;

namespace as = boost::asio;
namespace signals = boost::signals2;

class Core {

public:
    Core(
        char* pArgv)
        noexcept;

    ~Core();

    int run();

private:
    int initSubsystems();

    int initSettings();

    int initLogger();

    int initTailManager();

    int initCommunicator();

    int initObservingHandler(
        const json &conf);

    int initCommandsInterface();

    int initResultsInterface();

    int initEventsInterface();

    int initEquivalentsSubsystemsRouter(
        vector<SerializedEquivalent> equivalentIAmGateway);

    int initResourcesManager();

    int initTransactionsManager();

    int initStorageHandler();

    int initContractorsManager(
        const json &conf);

    int initSubsystemsController();

    int initTrustLinesInfluenceController();

    int initKeysStore();

    int initTopologyEventDelayedTask();

    int initFeaturesManager(
        const json &conf);

    void connectCommunicatorSignals();

    void connectObservingSignals();

    void connectCommandsInterfaceSignals();

    void connectResourcesManagerSignals();

    void connectSignalsToSlots();

    void onCommandReceivedSlot(
        bool success,
        BaseUserCommand::Shared command);

    void onClearTopologyCacheSlot(
        const SerializedEquivalent equivalent,
        BaseAddress::Shared nodeAddress);

    void onMessageReceivedSlot(
        Message::Shared message);

    void onMessageSendSlot(
        Message::Shared message,
        const ContractorID contractorID);

    void onMessageSendToAddressSlot(
        Message::Shared message,
        BaseAddress::Shared address);

    void onMessageSendWithCachingSlot(
        TransactionMessage::Shared message,
        ContractorID contractorID,
        Message::MessageType incomingMessageTypeFilter,
        uint32_t cacheTimeLiving);

    void onClaimSendToObserverSlot(
        ObservingClaimAppendRequestMessage::Shared message);

    void onAddTransactionToObservingCheckingSlot(
        const TransactionUUID& transactionUUID,
        BlockNumber maxBlockNumberForClaiming);

    void onObservingParticipantsVotesSlot(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber,
        map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures);

    void onObservingTransactionRejectSlot(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

    void onObservingTransactionUncertainSlot(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

    void onObservingTransactionCancelingSlot(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

    void onObservingAllowPaymentTransactionsSlot(
        bool allowPaymentTransactions);

    void onProcessConfirmationMessageSlot(
        ConfirmationMessage::Shared confirmationMessage);

    void onProcessPongMessageSlot(
        ContractorID contractorID);

    void onPathsResourceRequestedSlot(
        const TransactionUUID &transactionUUID,
        BaseAddress::Shared destinationNodeAddress,
        const SerializedEquivalent equivalent);

    void onObservingBlockNumberRequestSlot(
        const TransactionUUID &transactionUUID);

    void onResourceCollectedSlot(
        BaseResource::Shared resource);

    void writePIDFile();

    void updateProcessName();

protected:
    static string logHeader()
    noexcept;

    LoggerStream warning() const
    noexcept;

    LoggerStream error() const
    noexcept;

    LoggerStream info() const
    noexcept;

    LoggerStream debug() const
    noexcept;

protected:
    // This pointer is used to modify executable command description.
    // By default, it would point to the standard argv[0] char sequence;
    char* mCommandDescriptionPtr;

    as::io_service mIOService;

    unique_ptr<Logger> mLog;
    unique_ptr<Settings> mSettings;
    unique_ptr<Communicator> mCommunicator;
    unique_ptr<CommandsInterface> mCommandsInterface;
    unique_ptr<ResultsInterface> mResultsInterface;
    unique_ptr<EventsInterface> mEventsInterface;
    unique_ptr<ResourcesManager> mResourcesManager;
    unique_ptr<TransactionsManager> mTransactionsManager;
    unique_ptr<StorageHandler> mStorageHandler;
    unique_ptr<SubsystemsController> mSubsystemsController;
    unique_ptr<TrustLinesInfluenceController> mTrustLinesInfluenceController;
    unique_ptr<EquivalentsSubsystemsRouter> mEquivalentsSubsystemsRouter;
    unique_ptr<Keystore> mKeysStore;
    unique_ptr<ContractorsManager> mContractorsManager;
    unique_ptr<ObservingHandler> mObservingHandler;
    unique_ptr<TopologyEventDelayedTask> mTopologyEventDelayedTask;
    unique_ptr<TailManager> mTailManager;
    unique_ptr<FeaturesManager> mFeaturesManager;
};

#endif //GEO_NETWORK_CLIENT_CORE_H
