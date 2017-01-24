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

    if (command->commandIdentifier() == OpenTrustLineCommand::identifier()) {
        createOpenTrustLineTransaction(command);

    } else if (command->commandIdentifier() == CloseTrustLineCommand::identifier()) {
        createCloseTrustLineTransaction(command);

    } else {
        throw ConflictError("TransactionsManager::processCommand. "
                                "Unexpected command identifier.");
    }
}

void TransactionsManager::processMessage(
    Message::Shared message) {

    if (message->typeID() == Message::MessageTypeID::AcceptTrustLineMessageType) {
        createAcceptTrustLineTransaction(message);

    } else if (message->typeID() == Message::MessageTypeID::RejectTrustLineMessageType) {
        createRejectTrustLineTransaction(message);

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

        baseTransaction->addOnMessageSendSlot(
            boost::bind(
                &TransactionsManager::onMessageSend,
                this,
                _1,
                _2
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

        baseTransaction->addOnMessageSendSlot(
            boost::bind(
                &TransactionsManager::onMessageSend,
                this,
                _1,
                _2
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
            mNodeUUID,
            closeTrustLineCommand,
            mTransactionsScheduler,
            mTrustLinesManager);

        baseTransaction->addOnMessageSendSlot(
            boost::bind(
                &TransactionsManager::onMessageSend,
                this,
                _1,
                _2
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createTrustLineTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::createRejectTrustLineTransaction(
    Message::Shared message) {

    RejectTrustLineMessage::Shared rejectTrustLineMessage = static_pointer_cast<RejectTrustLineMessage>(message);

    try {
        BaseTransaction *baseTransaction = new RejectTrustLineTransaction(
            mNodeUUID,
            rejectTrustLineMessage,
            mTransactionsScheduler,
            mTrustLinesManager);

        baseTransaction->addOnMessageSendSlot(
            boost::bind(
                &TransactionsManager::onMessageSend,
                this,
                _1,
                _2
            )
        );

        mTransactionsScheduler->scheduleTransaction(BaseTransaction::Shared(baseTransaction));

    } catch (std::bad_alloc &e) {
        throw MemoryError("TransactionsManager::createTrustLineTransaction. "
                              "Can not allocate memory for transaction instance.");
    }
}

void TransactionsManager::onMessageSend(
    Message::Shared message,
    const NodeUUID &contractorUUID) {

    try {
        sendMessageSignal(
            message,
            contractorUUID
        );

    } catch (exception &e) {
        mLog->logException("TransactionsManager", e);
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
        mLog->logError("Transactions manager::acceptCommandResult: ",
                       "Error occurred when command result has accepted");
    }
}

void TransactionsManager::zeroPointers() {

    mTransactionsScheduler = nullptr;
}

void TransactionsManager::cleanupMemory() {

    if (mTransactionsScheduler != nullptr) {
        delete mTransactionsScheduler;
    }
}