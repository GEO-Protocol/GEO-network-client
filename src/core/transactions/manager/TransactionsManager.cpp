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

    mTrustLinesInterface = new TrustLinesInterface(mTrustLinesManager);
    mTransactionsScheduler = new TransactionsScheduler(
            mIOService,
            boost::bind(&TransactionsManager::acceptCommandResult, this, ::_1),
            mLog);
}

TransactionsManager::~TransactionsManager() {

    if (mTransactionsScheduler != nullptr) {
        delete mTransactionsScheduler;
    }

    if (mTrustLinesInterface != nullptr) {
        delete mTrustLinesInterface;
    }
}

void TransactionsManager::processCommand(
        BaseUserCommand::Shared commandPointer) {

    if (commandPointer.get()->derivedIdentifier() == OpenTrustLineCommand::identifier()) {
        openTrustLine(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == CloseTrustLineCommand::identifier()) {
        closeTrustLine(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == UpdateTrustLineCommand::identifier()) {
        updateTrustLine(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == UseCreditCommand::identifier()) {
        useCredit(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == MaximalTransactionAmountCommand::identifier()) {
        maximalTransactionAmount(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == TotalBalanceCommand::identifier()) {
        totalBalance(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == ContractorsListCommand::identifier()) {
        contractorsList(commandPointer);
    }

}

void TransactionsManager::openTrustLine(
        BaseUserCommand::Shared commandPointer) {

    OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new OpenTrustLineTransaction(OpenTrustLineCommand::Shared(openTrustLineCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::closeTrustLine(
        BaseUserCommand::Shared commandPointer) {

    CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new CloseTrustLineTransaction(CloseTrustLineCommand::Shared(closeTrustLineCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::updateTrustLine(
        BaseUserCommand::Shared commandPointer) {

    UpdateTrustLineCommand *updateTrustLineCommand = dynamic_cast<UpdateTrustLineCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new UpdateTrustLineTransaction(UpdateTrustLineCommand::Shared(updateTrustLineCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::maximalTransactionAmount(
        BaseUserCommand::Shared commandPointer) {

    MaximalTransactionAmountCommand *maximalTransactionAmountCommand = dynamic_cast<MaximalTransactionAmountCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new MaximalAmountTransaction(MaximalTransactionAmountCommand::Shared(maximalTransactionAmountCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::useCredit(
        BaseUserCommand::Shared commandPointer) {

    UseCreditCommand *useCreditCommand = dynamic_cast<UseCreditCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new UseCreditTransaction(UseCreditCommand::Shared(useCreditCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}


void TransactionsManager::totalBalance(
        BaseUserCommand::Shared commandPointer) {

    TotalBalanceCommand *totalBalanceCommand = dynamic_cast<TotalBalanceCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new TotalBalanceTransaction(TotalBalanceCommand::Shared(totalBalanceCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::contractorsList(
        BaseUserCommand::Shared commandPointer) {

    ContractorsListCommand *contractorsListCommand = dynamic_cast<ContractorsListCommand *>(commandPointer.get());

    BaseTransaction *baseTransaction = new ContractorsListTransaction(ContractorsListCommand::Shared(contractorsListCommand));

    mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

void TransactionsManager::acceptCommandResult(
        CommandResult::SharedConst commandResult) {

    try{
        if (commandResult.get() != nullptr) {
            string result = commandResult.get()->serialize();
            mResultsInterface->writeResult(result.c_str(), result.size());

        }
    } catch(...) {
        mLog->logError("Transactions manager", "Error occurred when command result has accepted");
    }
}
