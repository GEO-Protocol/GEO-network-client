#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../common/NodeUUID.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../interface/results/interface/ResultsInterface.h"
#include "../../logger/Logger.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../interface/commands/commands/BaseUserCommand.h"
#include "../../interface/commands/commands/OpenTrustLineCommand.h"
#include "../../interface/commands/commands/CloseTrustLineCommand.h"
#include "../../interface/commands/commands/UpdateTrustLineCommand.h"
#include "../../interface/commands/commands/MaximalTransactionAmountCommand.h"
#include "../../interface/commands/commands/UseCreditCommand.h"
#include "../../interface/commands/commands/TotalBalanceCommand.h"
#include "../../interface/commands/commands/ContractorsListCommand.h"

#include "../../network/messages/Message.h"
#include "../../network/messages/incoming/AcceptTrustLineMessage.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/unique/trust_lines/OpenTrustLineTransaction.h"
#include "../transactions/unique/trust_lines/AcceptTrustLineTransaction.h"
#include "../transactions/CloseTrustLineTransaction.h"
#include "../transactions/UpdateTrustLineTransaction.h"
#include "../transactions/MaximalAmountTransaction.h"
#include "../transactions/ContractorsListTransaction.h"
#include "../transactions/TotalBalanceTransaction.h"
#include "../transactions/unique/payment/UseCreditTransaction.h"

#include "../../network/messages/response/Response.h"

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

    void createCloseTrustLineTransaction(
        BaseUserCommand::Shared command);

    void createUpdateTrustLineTransaction(
        BaseUserCommand::Shared command);

    void createCalculateMaximalAmountTransaction(
        BaseUserCommand::Shared command);

    void createUseCreditTransaction(
        BaseUserCommand::Shared command);

    void createCalculateTotalBalanceTransaction(
        BaseUserCommand::Shared command);

    void createFormContractorsListTransaction(
        BaseUserCommand::Shared command);

    void zeroPointers();

    void cleanupMemory();

private:
    NodeUUID &mNodeUUID;
    as::io_service &mIOService;
    TrustLinesManager *mTrustLinesManager;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

    TransactionsScheduler *mTransactionsScheduler;

    TransactionsMessagesSlot *transactionsMessagesSlot;

private:

    class TransactionsMessagesSlot {

    public:
        TransactionsMessagesSlot(
          TransactionsManager *transactionsManager,
          Logger *logger);

        void sendTransactionMessageSlot(
          Message::Shared message,
          const NodeUUID &contractorUUID);

    private:
        TransactionsManager *mTransactionsManager;
        Logger *mLog;
    };

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
