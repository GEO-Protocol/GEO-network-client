#include "TransactionsManager.h"

TransactionsManager::TransactionsManager(
    NodeUUID &nodeUUID,
    as::io_service &IOService,
    TrustLinesManager *trustLinesManager,
    ResultsInterface *resultsInterface,
    Logger *logger) :

    mNodeUUID(nodeUUID),
    mIOService(IOService),
    mTrustLinesManager(trustLinesManager),
    mResultsInterface(resultsInterface),
    mLog(logger) {

    zeroPointers();

    try {
        mTransactionsScheduler = new TransactionsScheduler(
            mIOService,
            boost::bind(&TransactionsManager::acceptCommandResult, this, ::_1),
            mLog
        );

        transactionsMessagesSlot = new TransactionsMessagesSlot(
          this,
          mLog
        );

    } catch (std::bad_alloc &e) {
        cleanupMemory();
        throw MemoryError("TransactionsManager::TransactionsManager. "
                              "Can not allocate memory for one of the transactions manager's component.");
    }
}

TransactionsManager::~TransactionsManager() {

    cleanupMemory();
}

void TransactionsManager::processCommand(
    BaseUserCommand::Shared command) {

    if (command->derivedIdentifier() == OpenTrustLineCommand::identifier()) {
        createOpenTrustLineTransaction(command);

    } else if (command->derivedIdentifier() == CloseTrustLineCommand::identifier()) {
        createCloseTrustLineTransaction(command);

    } else if (command->derivedIdentifier() == UpdateTrustLineCommand::identifier()) {
        createUpdateTrustLineTransaction(command);

    } else if (command->derivedIdentifier() == UseCreditCommand::identifier()) {
        createUseCreditTransaction(command);

    } else if (command->derivedIdentifier() == MaximalTransactionAmountCommand::identifier()) {
        createCalculateMaximalAmountTransaction(command);

    } else if (command->derivedIdentifier() == TotalBalanceCommand::identifier()) {
        createCalculateTotalBalanceTransaction(command);

    } else if (command->derivedIdentifier() == ContractorsListCommand::identifier()) {
        createFormContractorsListTransaction(command);

    } else {
        throw ConflictError("TransactionsManager::processCommand. "
                                "Unexpected command identifier.");
    }
}

void TransactionsManager::processMessage(
    Message::Shared message) {

    if (message->typeID() == Message::MessageTypeID::AcceptTrustLineMessageType) {
        createAcceptTrustLineTransaction(message);

    } else {
        mTransactionsScheduler->handleMessage(message);
    }
}

void TransactionsManager::createOpenTrustLineTransaction(
    BaseUserCommand::Shared command) {

    OpenTrustLineCommand::Shared openTrustLineCommand = static_pointer_cast<OpenTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new OpenTrustLineTransaction(
            mNodeUUID,
            openTrustLineCommand,
            mTransactionsScheduler,
            mTrustLinesManager);

        baseTransaction->sendMessageSignal.connect(
          boost::bind(
              &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
              transactionsMessagesSlot
          )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createTrustLineTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createAcceptTrustLineTransaction(
    Message::Shared message) {

    AcceptTrustLineMessage::Shared acceptTrustLineMessage = static_pointer_cast<AcceptTrustLineMessage>(message);

    try {
        BaseTransaction *baseTransaction = new AcceptTrustLineTransaction(
            mNodeUUID,
            acceptTrustLineMessage,
            mTransactionsScheduler,
            mTrustLinesManager);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createTrustLineTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createCloseTrustLineTransaction(
    BaseUserCommand::Shared command) {

    CloseTrustLineCommand::Shared closeTrustLineCommand = static_pointer_cast<CloseTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new CloseTrustLineTransaction(
            closeTrustLineCommand,
            mTransactionsScheduler,
            mTrustLinesInterface);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createCloseTrustLineTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createUpdateTrustLineTransaction(
    BaseUserCommand::Shared command) {

    UpdateTrustLineCommand::Shared updateTrustLineCommand = static_pointer_cast<UpdateTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new UpdateTrustLineTransaction(
            updateTrustLineCommand,
            mTransactionsScheduler,
            mTrustLinesInterface);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createUpdateTrustLineTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createCalculateMaximalAmountTransaction(
    BaseUserCommand::Shared command) {

    MaximalTransactionAmountCommand::Shared maximalTransactionAmountCommand = static_pointer_cast<MaximalTransactionAmountCommand>(command);

    try {
        BaseTransaction *baseTransaction = new MaximalAmountTransaction(maximalTransactionAmountCommand);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createCalculateMaximalAmountTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createUseCreditTransaction(
    BaseUserCommand::Shared command) {

    UseCreditCommand::Shared useCreditCommand = static_pointer_cast<UseCreditCommand>(command);

    try {
        BaseTransaction *baseTransaction = new UseCreditTransaction(useCreditCommand);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createUseCreditTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}


void TransactionsManager::createCalculateTotalBalanceTransaction(
    BaseUserCommand::Shared command) {

    TotalBalanceCommand::Shared totalBalanceCommand = static_pointer_cast<TotalBalanceCommand>(command);

    try {
        BaseTransaction *baseTransaction = new TotalBalanceTransaction(totalBalanceCommand);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createCalculateTotalBalanceTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createFormContractorsListTransaction(
    BaseUserCommand::Shared command) {

    ContractorsListCommand::Shared contractorsListCommand = static_pointer_cast<ContractorsListCommand>(command);

    try {
        BaseTransaction *baseTransaction = new ContractorsListTransaction(contractorsListCommand);

        baseTransaction->sendMessageSignal.connect(
            boost::bind(
                &TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot,
                transactionsMessagesSlot
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createFormContractorsListTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}


void TransactionsManager::acceptCommandResult(
    CommandResult::SharedConst result) {

    try {
        string message = result->serialize();
        mResultsInterface->writeResult(
            message.c_str(),
            message.size()
        );

    } catch (...) {
        mLog->logError("Transactions manager",
                       "Error occurred when command result has accepted");
    }
}

void TransactionsManager::zeroPointers() {

    mTransactionsScheduler = nullptr;
    transactionsMessagesSlot = nullptr;
}

void TransactionsManager::cleanupMemory() {

    if (mTransactionsScheduler != nullptr) {
        delete mTransactionsScheduler;
    }

    if (transactionsMessagesSlot != nullptr) {
        delete transactionsMessagesSlot;
    }
}

TransactionsManager::TransactionsMessagesSlot::TransactionsMessagesSlot(
    TransactionsManager *transactionsManager,
    Logger *logger) :

    mTransactionsManager(transactionsManager),
    mLog(logger){}

void TransactionsManager::TransactionsMessagesSlot::sendTransactionMessageSlot(
    Message::Shared message,
    const NodeUUID &contractorUUID) {

    try {
        mTransactionsManager->sendMessageSignal(
            message,
            contractorUUID
        );

    } catch (exception &e) {
        mLog->logException("TransactionsManager", e);
    }
}
