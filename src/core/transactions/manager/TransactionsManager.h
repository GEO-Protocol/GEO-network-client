#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include "../../logger/Logger.h"

#include "../../trust_lines/TrustLine.h"

#include "../../interface/commands/commands/BaseUserCommand.h"
#include "../../interface/commands/commands/OpenTrustLineCommand.h"
#include "../../interface/commands/commands/CloseTrustLineCommand.h"
#include "../../interface/commands/commands/UpdateTrustLineCommand.h"
#include "../../interface/commands/commands/MaximalTransactionAmountCommand.h"
#include "../../interface/commands/commands/UseCreditCommand.h"
#include "../../interface/commands/commands/TotalBalanceCommand.h"
#include "../../interface/commands/commands/ContractorsListCommand.h"

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
#include "../../interface/results/result/CommandResult.h"

#include <boost/lexical_cast.hpp>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class TransactionsManager {

private:

    as::io_service &mIOService;
    TransactionsScheduler *mTransactionsScheduler;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

public:
    TransactionsManager(
        as::io_service &IOService,
        ResultsInterface *resultsInterface,
        Logger *logger);

    ~TransactionsManager();

    void processCommand(
            shared_ptr<BaseUserCommand> commandPointer);

private:
    void openTrustLine(
            BaseUserCommand::Shared commandPointer);

    void closeTrustLine(
            BaseUserCommand::Shared commandPointer);

    void updateTrustLine(
            BaseUserCommand::Shared commandPointer);

    void maximalTransactionAmount(
            BaseUserCommand::Shared commandPointer);

    void useCredit(
            BaseUserCommand::Shared commandPointer);

    void totalBalance(
            BaseUserCommand::Shared commandPointer);

    void contractorsList(
            BaseUserCommand::Shared commandPointer);

public:
    void acceptCommandResult(CommandResult::SharedConst commandResult);

};



#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
