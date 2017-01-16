#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../logger/Logger.h"

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

#include "../../network/communicator/Communicator.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../trust_lines/interface/TrustLinesInterface.h"
#include "../scheduler/TransactionsScheduler.h"

#include "../transactions/BaseTransaction.h"
#include "../transactions/OpenTrustLineTransaction.h"
#include "../transactions/AcceptTrustLineTransaction.h"
#include "../transactions/CloseTrustLineTransaction.h"
#include "../transactions/UpdateTrustLineTransaction.h"
#include "../transactions/MaximalAmountTransaction.h"
#include "../transactions/ContractorsListTransaction.h"
#include "../transactions/TotalBalanceTransaction.h"
#include "../transactions/UseCreditTransaction.h"

#include "../../interface/results/interface/ResultsInterface.h"

#include <string>


using namespace std;

class Communicator;
class TransactionsManager {
    // todo: hsc: tests?

public:
    TransactionsManager(
        as::io_service &IOService,
        Communicator *communicator,
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
    void openTrustLine(
        BaseUserCommand::Shared command);

    void acceptTrustLine(
        Message::Shared message);

    void closeTrustLine(
        BaseUserCommand::Shared command);

    void updateTrustLine(
        BaseUserCommand::Shared command);

    void calculateMaxTransactionAmount(
        BaseUserCommand::Shared command);

    void useCredit(
        BaseUserCommand::Shared command);

    void calculateTotalBalance(
        BaseUserCommand::Shared command);

    void formContractorsList(
        BaseUserCommand::Shared command);

private:

    as::io_service &mIOService;
    Communicator *mCommunicator;
    TrustLinesManager *mTrustLinesManager;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

    TrustLinesInterface *mTrustLinesInterface;
    TransactionsScheduler *mTransactionsScheduler;

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
