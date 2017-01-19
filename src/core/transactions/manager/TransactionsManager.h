#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../common/NodeUUID.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../interface/results/interface/ResultsInterface.h"
#include "../../logger/Logger.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../interface/commands/commands/BaseUserCommand.h"
#include "../../interface/commands/commands/trust_lines/OpenTrustLineCommand.h"

#include "../../network/messages/Message.h"
#include "../../network/messages/incoming/AcceptTrustLineMessage.h"
#include "../../network/messages/response/Response.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/unique/UniqueTransaction.h"
#include "../transactions/unique/trust_lines/OpenTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/AcceptTrustLineTransaction.h"

#include <boost/signals2.hpp>

#include <string>

using namespace std;
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

private:
    void createOpenTrustLineTransaction(
        BaseUserCommand::Shared command);

    void createAcceptTrustLineTransaction(
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

    TransactionsScheduler *mTransactionsScheduler;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
