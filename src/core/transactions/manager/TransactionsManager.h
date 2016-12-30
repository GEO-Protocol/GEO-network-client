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

#include "../../trust_lines/manager/TrustLinesManager.h"
#include "../../trust_lines/interface/TrustLinesInterface.h"
#include "../scheduler/TransactionsScheduler.h"
#include "../transactions/BaseTransaction.h"
#include "../transactions/OpenTrustLineTransaction.h"
#include "../transactions/CloseTrustLineTransaction.h"
#include "../transactions/UpdateTrustLineTransaction.h"
#include "../transactions/MaximalAmountTransaction.h"
#include "../transactions/ContractorsListTransaction.h"
#include "../transactions/TotalBalanceTransaction.h"
#include "../transactions/UseCreditTransaction.h"

#include "../../interface/results/interface/ResultsInterface.h"

#include <string>


using namespace std;


class TransactionsManager {
    // todo: hsc: tests?

    // todo: public methods should be at the top, and then - private
private:

    // todo: separate members that would be created into this class from the pointers to the external objects.
    as::io_service &mIOService;
    TrustLinesManager *mTrustLinesManager;
    TrustLinesInterface *mTrustLinesInterface;
    TransactionsScheduler *mTransactionsScheduler;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

public:
    TransactionsManager(
        as::io_service &IOService,
        TrustLinesManager *trustLinesManager,
        ResultsInterface *resultsInterface,
        Logger *logger);
    ~TransactionsManager();

    void processCommand(
        BaseUserCommand::Shared command);

private:
    void openTrustLine(
        BaseUserCommand::Shared command);

    void closeTrustLine(
        BaseUserCommand::Shared command);

    void updateTrustLine(
        BaseUserCommand::Shared command);

    void maximalTransactionAmount( // todo: rename to calculateMaxTransactionAmount (method name should describe the action)
        BaseUserCommand::Shared command);

    void useCredit(
        BaseUserCommand::Shared command);

    void totalBalance( // todo: rename to calculateTotalBalance (method name should describe the action)
         BaseUserCommand::Shared command);

    void contractorsList( // todo: rename to formContractorsList (method name should describe the action)
        BaseUserCommand::Shared command);

    // todo: why this public method is separated?
public:
    void acceptCommandResult(CommandResult::SharedConst result);

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
