#include "TransactionsManager.h"

TransactionsManager::TransactionsManager(
    as::io_service &IOService,
    TrustLinesManager *trustLinesManager,
    ResultsInterface *resultsInterface,
    Logger *logger):

    mIOService(IOService),
    mTrustLinesManager(trustLinesManager),
    mResultsInterface(resultsInterface),
    mLog(logger){

    // todo: memory error handling?
    mTrustLinesInterface = new TrustLinesInterface(mTrustLinesManager);
    mTransactionsScheduler = new TransactionsScheduler(
            mIOService,
            boost::bind(&TransactionsManager::acceptCommandResult, this, ::_1),
            mLog);
}

TransactionsManager::~TransactionsManager() {

    // todo: there is no default pointers zeroing.
    // so the pointers will never be equal to nullptr
    if (mTransactionsScheduler != nullptr) {
        delete mTransactionsScheduler;
    }

    if (mTrustLinesInterface != nullptr) {
        delete mTrustLinesInterface;
    }
}

void TransactionsManager::processCommand(
    BaseUserCommand::Shared command) {

    if (command.get()->derivedIdentifier() == OpenTrustLineCommand::identifier()) { // todo: use ->
        openTrustLine(command);

    } else if (command.get()->derivedIdentifier() == CloseTrustLineCommand::identifier()) {
        closeTrustLine(command);

    } else if (command.get()->derivedIdentifier() == UpdateTrustLineCommand::identifier()) {
        updateTrustLine(command);

    } else if (command.get()->derivedIdentifier() == UseCreditCommand::identifier()) {
        useCredit(command);

    } else if (command.get()->derivedIdentifier() == MaximalTransactionAmountCommand::identifier()) {
        maximalTransactionAmount(command);

    } else if (command.get()->derivedIdentifier() == TotalBalanceCommand::identifier()) {
        totalBalance(command);

    } else if (command.get()->derivedIdentifier() == ContractorsListCommand::identifier()) {
        contractorsList(command);
    }

    // todo: what if we will receive not expected command?
}

void TransactionsManager::openTrustLine(
    BaseUserCommand::Shared command) {

    OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new OpenTrustLineTransaction(OpenTrustLineCommand::Shared(openTrustLineCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::closeTrustLine(
        BaseUserCommand::Shared command) {

    CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new CloseTrustLineTransaction(CloseTrustLineCommand::Shared(closeTrustLineCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::updateTrustLine(
        BaseUserCommand::Shared command) {

    UpdateTrustLineCommand *updateTrustLineCommand = dynamic_cast<UpdateTrustLineCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new UpdateTrustLineTransaction(UpdateTrustLineCommand::Shared(updateTrustLineCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::maximalTransactionAmount(
        BaseUserCommand::Shared command) {

    MaximalTransactionAmountCommand *maximalTransactionAmountCommand = dynamic_cast<MaximalTransactionAmountCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new MaximalAmountTransaction(MaximalTransactionAmountCommand::Shared(maximalTransactionAmountCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::useCredit(
        BaseUserCommand::Shared command) {

    UseCreditCommand *useCreditCommand = dynamic_cast<UseCreditCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new UseCreditTransaction(UseCreditCommand::Shared(useCreditCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}


void TransactionsManager::totalBalance(
        BaseUserCommand::Shared command) {

    TotalBalanceCommand *totalBalanceCommand = dynamic_cast<TotalBalanceCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new TotalBalanceTransaction(TotalBalanceCommand::Shared(totalBalanceCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::contractorsList(
        BaseUserCommand::Shared command) {

    ContractorsListCommand *contractorsListCommand = dynamic_cast<ContractorsListCommand *>(command.get());

    // todo: possible heap corruption
    // todo: http://stackoverflow.com/questions/1358143/downcasting-shared-ptrbase-to-shared-ptrderived
    // counters of "command" shared pointer and newly created shared pointer are not the same.
    // This two pointers knows nothing about each one, and "command" will drop the memory even if newly created one -
    // will still be present
    BaseTransaction *baseTransaction = new ContractorsListTransaction(ContractorsListCommand::Shared(contractorsListCommand));
    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::acceptCommandResult(
    CommandResult::SharedConst result) {

    try {
        if (result != nullptr) {
            string message = result.get()->serialize(); // todo: use ->
            mResultsInterface->writeResult(message.c_str(), message.size());

        }
    } catch(...) {
        // todo: add info about the command and the result
        // how wy should fix it if we even don't know what command created this situation?
        mLog->logError("Transactions manager", "Error occurred when command result has accepted");
    }
}
