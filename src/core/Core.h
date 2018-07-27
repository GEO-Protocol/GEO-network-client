
#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "common/Types.h"
#include "common/NodeUUID.h"

#include "settings/Settings.h"
#include "network/communicator/Communicator.h"
#include "interface/commands_interface/interface/CommandsInterface.h"
#include "interface/results_interface/interface/ResultsInterface.h"
#include "resources/manager/ResourcesManager.h"
#include "transactions/manager/TransactionsManager.h"
#include "io/storage/StorageHandler.h"
#include "equivalents/EquivalentsSubsystemsRouter.h"
#include "crypto/keychain.h"

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

    int initCommunicator(
        const json &conf);

    int initCommandsInterface();

    int initResultsInterface();

    int initEquivalentsSubsystemsRouter(
        vector<SerializedEquivalent> equivalentIAmGateway);

    int initResourcesManager();

    int initTransactionsManager();

    int initStorageHandler();

    int initSubsystemsController();

    int initTrustLinesInfluenceController();

    int initKeysStore();

    void connectCommunicatorSignals();

    void connectCommandsInterfaceSignals();

    void connectResourcesManagerSignals();

    void connectSignalsToSlots();

    void onCommandReceivedSlot(
        BaseUserCommand::Shared command);

    void onClearTopologyCacheSlot(
        const SerializedEquivalent equivalent,
        const NodeUUID &nodeUUID);

    void onMessageReceivedSlot(
        Message::Shared message);

    void onMessageSendSlot(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void onProcessConfirmationMessageSlot(
        ConfirmationMessage::Shared confirmationMessage);

    void onPathsResourceRequestedSlot(
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationNodeUUID,
        const SerializedEquivalent equivalent);

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

    NodeUUID mNodeUUID;
    as::io_service mIOService;

    unique_ptr<Logger> mLog;
    unique_ptr<Settings> mSettings;
    unique_ptr<Communicator> mCommunicator;
    unique_ptr<CommandsInterface> mCommandsInterface;
    unique_ptr<ResultsInterface> mResultsInterface;
    unique_ptr<ResourcesManager> mResourcesManager;
    unique_ptr<TransactionsManager> mTransactionsManager;
    unique_ptr<StorageHandler> mStorageHandler;
    unique_ptr<SubsystemsController> mSubsystemsController;
    unique_ptr<TrustLinesInfluenceController> mTrustLinesInfluenceController;
    unique_ptr<EquivalentsSubsystemsRouter> mEquivalentsSubsystemsRouter;
    unique_ptr<Keystore> mKeysStore;
};

#endif //GEO_NETWORK_CLIENT_CORE_H
