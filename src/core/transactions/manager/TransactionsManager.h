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
#include "../../interface/results/ResultsInterface.h"
#include "../../interface/results/CommandResult.h"

#include "../../trust_lines/TrustLine.h"

#include <boost/lexical_cast.hpp>

#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

class TransactionsManager {

private:

    as::io_service &mIOService;
    ResultsInterface *mResultsInterface;
    Logger *mLog;

public:
    TransactionsManager(
        as::io_service &IOService,
        ResultsInterface *resultsInterface,
        Logger *logger);

    ~TransactionsManager();

    void processCommand(shared_ptr<BaseUserCommand> commandPointer);

private:
    pair<bool, shared_ptr<const CommandResult>> openTrustLine(shared_ptr<BaseUserCommand> commandPointer);

    pair<bool, shared_ptr<const CommandResult>> closeTrustLine(shared_ptr<BaseUserCommand> commandPointer);

    pair<bool, shared_ptr<const CommandResult>> updateTrustLine(shared_ptr<BaseUserCommand> commandPointer);

    pair<bool, shared_ptr<const CommandResult>> maximalTransactionAmount(shared_ptr<BaseUserCommand> commandPointer);

    pair<bool, shared_ptr<const CommandResult>> useCredit(shared_ptr<BaseUserCommand> commandPointer);

    pair<bool, shared_ptr<const CommandResult>> totalBalance(shared_ptr<BaseUserCommand> commandPointer);

    pair<bool, shared_ptr<const CommandResult>> contractorsList(shared_ptr<BaseUserCommand> commandPointer);

    //static void segfaultSigaction(int signal, siginfo_t *si, void *arg);

};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
