#include "TransactionsManager.h"

TransactionsManager::TransactionsManager(
    as::io_service &IOService,
    ResultsInterface *resultsInterface,
    Logger *logger):

    mIOService(IOService),
    mResultsInterface(resultsInterface),
    mLog(logger){

    /*mTransactionsScheduler = new TransactionsScheduler(
            mIOService,
            boost::bind(&TransactionsManager::acceptCommandResult, this, ::_1),
            mLog);*/
}

TransactionsManager::~TransactionsManager() {

    if (mTransactionsScheduler != nullptr) {
        delete mTransactionsScheduler;
    }
}

void TransactionsManager::processCommand(
        BaseUserCommand::Shared commandPointer) {

    CommandResult::SharedConst result;

    if (commandPointer.get()->derivedIdentifier() == OpenTrustLineCommand::identifier()) {
        result = openTrustLine(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == CloseTrustLineCommand::identifier()) {
        result = closeTrustLine(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == UpdateTrustLineCommand::identifier()) {
        result = updateTrustLine(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == UseCreditCommand::identifier()) {
        result = useCredit(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == MaximalTransactionAmountCommand::identifier()) {
        result = maximalTransactionAmount(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == TotalBalanceCommand::identifier()) {
        result = totalBalance(commandPointer);

    } else if (commandPointer.get()->derivedIdentifier() == ContractorsListCommand::identifier()) {
        result = contractorsList(commandPointer);
    }

    if (result.get() != nullptr) {
        const string res = result.get()->serialize();
        mResultsInterface->writeResult(res.c_str(), res.size());
    }
}

CommandResult::SharedConst TransactionsManager::openTrustLine(
        BaseUserCommand::Shared commandPointer) {

    OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(commandPointer.get());

    return CommandResult::SharedConst(openTrustLineCommand->resultOk());

    //BaseTransaction *baseTransaction = new OpenTrustLineTransaction(OpenTrustLineCommand::Shared(openTrustLineCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

CommandResult::SharedConst TransactionsManager::closeTrustLine(
        BaseUserCommand::Shared commandPointer) {

    CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(commandPointer.get());

    return CommandResult::SharedConst(closeTrustLineCommand->resultOk());

    //BaseTransaction *baseTransaction = new CloseTrustLineTransaction(CloseTrustLineCommand::Shared(closeTrustLineCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

CommandResult::SharedConst TransactionsManager::updateTrustLine(
        BaseUserCommand::Shared commandPointer) {

    UpdateTrustLineCommand *updateTrustLineCommand = dynamic_cast<UpdateTrustLineCommand *>(commandPointer.get());

    return CommandResult::SharedConst(updateTrustLineCommand->resultOk());

    //BaseTransaction *baseTransaction = new UpdateTrustLineTransaction(UpdateTrustLineCommand::Shared(updateTrustLineCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

CommandResult::SharedConst TransactionsManager::maximalTransactionAmount(
        BaseUserCommand::Shared commandPointer) {

    MaximalTransactionAmountCommand *maximalTransactionAmountCommand = dynamic_cast<MaximalTransactionAmountCommand *>(commandPointer.get());

    trust_amount maximalAmount(500);

    return CommandResult::SharedConst(maximalTransactionAmountCommand->resultOk(
            maximalAmount));

    //BaseTransaction *baseTransaction = new MaximalAmountTransaction(MaximalTransactionAmountCommand::Shared(maximalTransactionAmountCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

CommandResult::SharedConst TransactionsManager::useCredit(
        BaseUserCommand::Shared commandPointer) {

    UseCreditCommand *useCreditCommand = dynamic_cast<UseCreditCommand *>(commandPointer.get());

    TransactionUUID u;
    Timestamp proceed = boost::posix_time::microsec_clock::universal_time();
    Timestamp complete = boost::posix_time::microsec_clock::universal_time();

    return CommandResult::SharedConst(useCreditCommand->resultOk(
            u,
            200,
            proceed,
            complete
    ));

    //BaseTransaction *baseTransaction = new UseCreditTransaction(UseCreditCommand::Shared(useCreditCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}


CommandResult::SharedConst TransactionsManager::totalBalance(
        BaseUserCommand::Shared commandPointer) {

    TotalBalanceCommand *totalBalanceCommand = dynamic_cast<TotalBalanceCommand *>(commandPointer.get());

    trust_amount incoming(100);
    trust_amount incomingUsed(57);
    trust_amount outgoing(200);
    trust_amount outgoingUsed(120);

    return CommandResult::SharedConst(totalBalanceCommand->resultOk(
            incoming,
            incomingUsed,
            outgoing,
            outgoingUsed
    ));

    //BaseTransaction *baseTransaction = new TotalBalanceTransaction(TotalBalanceCommand::Shared(totalBalanceCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
}

CommandResult::SharedConst TransactionsManager::contractorsList(
        BaseUserCommand::Shared commandPointer) {

    ContractorsListCommand *contractorsListCommand = dynamic_cast<ContractorsListCommand *>(commandPointer.get());

    NodeUUID c1;
    NodeUUID c2;
    vector<NodeUUID> contractors;
    contractors.push_back(c1);
    contractors.push_back(c2);

    return CommandResult::SharedConst(contractorsListCommand->resultOk(
            contractors
    ));

    //BaseTransaction *baseTransaction = new ContractorsListTransaction(ContractorsListCommand::Shared(contractorsListCommand));

    //mTransactionsScheduler->addTransaction(BaseTransaction::Shared(baseTransaction));
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
