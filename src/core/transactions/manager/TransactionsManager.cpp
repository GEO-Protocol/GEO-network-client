#include "TransactionsManager.h"

TransactionsManager::TransactionsManager(
    as::io_service &IOService,
    Communicator *communicator,
    TrustLinesManager *trustLinesManager,
    ResultsInterface *resultsInterface,
    Logger *logger) :

    mIOService(IOService),
    mCommunicator(communicator),
    mTrustLinesManager(trustLinesManager),
    mResultsInterface(resultsInterface),
    mLog(logger) {

    try {
        mTrustLinesInterface = new TrustLinesInterface(mTrustLinesManager);

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::TransactionsManager. "
                              "Can not allocate memory for trust lines parseInterface.");
    }

    try {
        mTransactionsScheduler = new TransactionsScheduler(
            mIOService,
            boost::bind(&TransactionsManager::acceptCommandResult, this, ::_1),
            mLog);

    } catch (std::bad_alloc &e) {
        delete mTrustLinesInterface;
        throw MemoryError("TransactionsManager::TransactionsManager. "
                              "Can not allocate memory for transactions scheduler.");
    }
}

TransactionsManager::~TransactionsManager() {

    delete mTrustLinesInterface;
    delete mTransactionsScheduler;
}

void TransactionsManager::processCommand(
    BaseUserCommand::Shared command) {

    if (command->derivedIdentifier() == OpenTrustLineCommand::identifier()) {
        openTrustLine(command);

    } else if (command->derivedIdentifier() == CloseTrustLineCommand::identifier()) {
        closeTrustLine(command);

    } else if (command->derivedIdentifier() == UpdateTrustLineCommand::identifier()) {
        updateTrustLine(command);

    } else if (command->derivedIdentifier() == UseCreditCommand::identifier()) {
        useCredit(command);

    } else if (command->derivedIdentifier() == MaximalTransactionAmountCommand::identifier()) {
        calculateMaxTransactionAmount(command);

    } else if (command->derivedIdentifier() == TotalBalanceCommand::identifier()) {
        calculateTotalBalance(command);

    } else if (command->derivedIdentifier() == ContractorsListCommand::identifier()) {
        formContractorsList(command);

    } else {
        throw ConflictError("TransactionsManager::processCommand. "
                                "Unexpected command identifier.");
    }
}

void TransactionsManager::processMessage(
    Message::Shared message) {

    if (message->typeID() == Message::MessageTypeID::AcceptTrustLineMessageType) {
        acceptTrustLine(message);

    } else {
        mTransactionsScheduler->handleMessage(message);
    }
}

void TransactionsManager::openTrustLine(
    BaseUserCommand::Shared command) {

    OpenTrustLineCommand::Shared openTrustLineCommand = static_pointer_cast<OpenTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new OpenTrustLineTransaction(
            openTrustLineCommand,
            mCommunicator,
            mTransactionsScheduler,
            mTrustLinesInterface);

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::openTrustLine. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::acceptTrustLine(
    Message::Shared message) {

    AcceptTrustLineMessage::Shared acceptTrustLineMessage = static_pointer_cast<AcceptTrustLineMessage>(message);

    try {
        BaseTransaction *baseTransaction = new AcceptTrustLineTransaction(
            acceptTrustLineMessage, mCommunicator, mTransactionsScheduler, mTrustLinesInterface);

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::acceptTrustLine. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::closeTrustLine(
    BaseUserCommand::Shared command) {

    CloseTrustLineCommand::Shared closeTrustLineCommand = static_pointer_cast<CloseTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new CloseTrustLineTransaction(
            closeTrustLineCommand,
            mTransactionsScheduler,
            mTrustLinesInterface);

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::closeTrustLine. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::updateTrustLine(
    BaseUserCommand::Shared command) {

    UpdateTrustLineCommand::Shared updateTrustLineCommand = static_pointer_cast<UpdateTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new UpdateTrustLineTransaction(
            updateTrustLineCommand,
            mTransactionsScheduler,
            mTrustLinesInterface);

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::updateTrustLine. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::calculateMaxTransactionAmount(
    BaseUserCommand::Shared command) {

    MaximalTransactionAmountCommand::Shared maximalTransactionAmountCommand = static_pointer_cast<MaximalTransactionAmountCommand>(
        command);

    try {
        BaseTransaction *baseTransaction = new MaximalAmountTransaction(maximalTransactionAmountCommand);
        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::calculateMaxTransactionAmount. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::useCredit(
    BaseUserCommand::Shared command) {

    UseCreditCommand::Shared useCreditCommand = static_pointer_cast<UseCreditCommand>(command);

    try {
        BaseTransaction *baseTransaction = new UseCreditTransaction(useCreditCommand);
        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::useCredit. "
                              "Can not allocate memory for transaction instance.");
    }
}


void TransactionsManager::calculateTotalBalance(
    BaseUserCommand::Shared command) {

    TotalBalanceCommand::Shared totalBalanceCommand = static_pointer_cast<TotalBalanceCommand>(command);

    try {
        BaseTransaction *baseTransaction = new TotalBalanceTransaction(totalBalanceCommand);
        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::calculateTotalBalance. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::formContractorsList(
    BaseUserCommand::Shared command) {

    ContractorsListCommand::Shared contractorsListCommand = static_pointer_cast<ContractorsListCommand>(command);

    try {
        BaseTransaction *baseTransaction = new ContractorsListTransaction(contractorsListCommand);
        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::formContractorsList. "
                              "Can not allocate memory for transaction instance.");
    }
}


void TransactionsManager::acceptCommandResult(
    CommandResult::SharedConst result) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(result != nullptr);
#endif

    try {
        string message = result->serialize();
        mResultsInterface->writeResult(message.c_str(), message.size());

    } catch (...) {
        mLog->logError("Transactions manager",
                       "Error occurred when command result has accepted");
    }
}
