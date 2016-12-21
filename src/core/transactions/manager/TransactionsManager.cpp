#include <chrono>
#include "TransactionsManager.h"

TransactionsManager::TransactionsManager(
    as::io_service &IOService,
    ResultsInterface *resultsInterface,
    Logger *logger):

    mIOService(IOService),
    mResultsInterface(resultsInterface),
    mLog(logger){}

TransactionsManager::~TransactionsManager() {
}

void TransactionsManager::processCommand(shared_ptr<BaseUserCommand> commandPointer) {
    pair<bool, shared_ptr<const CommandResult>> parsingResult;

    cout << commandPointer.get()->derivedIdentifier() + string(" in processing") << endl;

    if (commandPointer.get()->derivedIdentifier() == OpenTrustLineCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (openTrustLine(commandPointer));
    } else if (commandPointer.get()->derivedIdentifier() == CloseTrustLineCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (closeTrustLine(commandPointer));
    } else if (commandPointer.get()->derivedIdentifier() == UpdateTrustLineCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (updateTrustLine(commandPointer));
    } else if (commandPointer.get()->derivedIdentifier() == UseCreditCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (useCredit(commandPointer));
    } else if (commandPointer.get()->derivedIdentifier() == MaximalTransactionAmountCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (maximalTransactionAmount(commandPointer));
    } else if (commandPointer.get()->derivedIdentifier() == TotalBalanceCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (totalBalance(commandPointer));
    } else if (commandPointer.get()->derivedIdentifier() == ContractorsListCommand::identifier()) {
        parsingResult = pair<bool, shared_ptr<const CommandResult>> (contractorsList(commandPointer));
    }


    if(parsingResult.first){
        try {
            auto result = parsingResult.second.get()->serialize();

            cout << string("Command UUID ") + parsingResult.second.get()->commandUUID().stringUUID() +
                    string(" Result code: ") + to_string(parsingResult.second.get()->resultCode())
                 << endl;

            /*struct sigaction sa;
            memset(&sa, 0, sizeof(struct sigaction));
            sigemptyset(&sa.sa_mask);
            sa.sa_sigaction = TransactionsManager::segfaultSigaction;
            sa.sa_flags = SA_SIGINFO;
            sigaction(SIGSEGV, &sa, NULL);*/

            mResultsInterface->writeResult(result.c_str(), result.size());
        } catch(std::exception &e){
            cout << e.what() << endl;
        }
    }
}

pair<bool, shared_ptr<const CommandResult>> TransactionsManager::openTrustLine(shared_ptr <BaseUserCommand> commandPointer) {
    OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(commandPointer.get());

    const CommandResult *result = openTrustLineCommand->resultOk();

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}

pair<bool, shared_ptr<const CommandResult>> TransactionsManager::closeTrustLine(shared_ptr <BaseUserCommand> commandPointer) {
    CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(commandPointer.get());

    const CommandResult *result = closeTrustLineCommand->resultOk();

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}

pair<bool, shared_ptr<const CommandResult>> TransactionsManager::updateTrustLine(shared_ptr <BaseUserCommand> commandPointer) {
    UpdateTrustLineCommand *updateTrustLineCommand = dynamic_cast<UpdateTrustLineCommand *>(commandPointer.get());

    const CommandResult *result = updateTrustLineCommand->resultOk();

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}

pair<bool, shared_ptr<const CommandResult>> TransactionsManager::maximalTransactionAmount(shared_ptr <BaseUserCommand> commandPointer) {
    MaximalTransactionAmountCommand *maximalTransactionAmountCommand = dynamic_cast<MaximalTransactionAmountCommand *>(commandPointer.get());

    trust_amount maximal(500);
    const CommandResult *result = maximalTransactionAmountCommand->resultOk(maximal);

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}

pair<bool, shared_ptr<const CommandResult>> TransactionsManager::useCredit(shared_ptr <BaseUserCommand> commandPointer) {
    UseCreditCommand *useCreditCommand = dynamic_cast<UseCreditCommand *>(commandPointer.get());

    TransactionUUID transactionUUID;
    Timestamp received = boost::posix_time::microsec_clock::universal_time();
    Timestamp proceed = boost::posix_time::microsec_clock::universal_time();
    const CommandResult *result = useCreditCommand->resultOk(
            transactionUUID,
            200,
            received,
            proceed);

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}


pair<bool, shared_ptr<const CommandResult>> TransactionsManager::totalBalance(shared_ptr <BaseUserCommand> commandPointer) {
    TotalBalanceCommand *totalBalanceCommand = dynamic_cast<TotalBalanceCommand *>(commandPointer.get());

    trust_amount incoming(100);
    trust_amount incomingUsed(50);
    trust_amount outgoing(200);
    trust_amount outgoingUsed(100);

    const CommandResult *result = totalBalanceCommand->resultOk(
            incoming,
            incomingUsed,
            outgoing,
            outgoingUsed);

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}

pair<bool, shared_ptr<const CommandResult>> TransactionsManager::contractorsList(shared_ptr <BaseUserCommand> commandPointer) {
    ContractorsListCommand *contractorsListCommand = dynamic_cast<ContractorsListCommand *>(commandPointer.get());

    NodeUUID uuid1 = boost::uuids::random_generator()();
    NodeUUID uuid2 = boost::uuids::random_generator()();
    NodeUUID uuid3 = boost::uuids::random_generator()();
    vector<NodeUUID> contractorList;
    contractorList.push_back(uuid1);
    contractorList.push_back(uuid2);
    contractorList.push_back(uuid3);

    const CommandResult *result = contractorsListCommand->resultOk(contractorList);

    return pair<bool, shared_ptr<const CommandResult>> (true, shared_ptr<const CommandResult>(result));
}

/*void TransactionsManager::segfaultSigaction(int signal, siginfo_t *si, void *arg) {
    printf("Caught segfault at address %p\n", si->si_addr);
    _exit(0);
}*/
