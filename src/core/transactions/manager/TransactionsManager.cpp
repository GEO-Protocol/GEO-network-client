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
        mStorage = new storage::UUIDMapBlockStorage(
            "io/transactions",
            "transactions.dat");

        mTransactionsScheduler = new TransactionsScheduler(
            mIOService,
            mStorage,
            boost::bind(&TransactionsManager::acceptCommandResult, this, ::_1),
            mLog
        );

    } catch (std::bad_alloc &e) {
        cleanupMemory();
        throw MemoryError("TransactionsManager::TransactionsManager. "
                              "Can not allocate memory for one of the transactions manager's component.");
    }

    try {
        loadTransactions();

    } catch (std::exception &e) {
        mLog->logException("TransactionsManager", e);
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

    } else if (command->commandIdentifier() == SetTrustLineCommand::identifier()) {
        createSetTrustLineTransaction(command);

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

    } else if (message->typeID() == Message::MessageTypeID::UpdateTrustLineMessageType) {
        createUpdateTrustLineTransaction(message);

    } else {
        mTransactionsScheduler->handleMessage(message);
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

void TransactionsManager::startRoutingTablesExchange(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

    /*BaseTransaction *baseTransaction = new SendRoutingTablesTransaction(
        mNodeUUID,
        const_cast<NodeUUID&> (contractorUUID),
        mTransactionsScheduler
    );

    baseTransaction->addOnMessageSendSlot(
        boost::bind(
            &TransactionsManager::onMessageSend,
            this,
            _1,
            _2
        )
    );

    mTransactionsScheduler->postponeRoutingTableTransaction(BaseTransaction::Shared(baseTransaction));*/

}

void TransactionsManager::loadTransactions() {

    unique_ptr<const vector<storage::uuids::uuid>> uuidKeys = unique_ptr<const vector<storage::uuids::uuid>> (mStorage->keys());
    if (uuidKeys->size() > 0) {
        for (auto const &uuidKey : *uuidKeys) {
            storage::Record::Shared record;
            try {
                record = mStorage->readFromFile(storage::uuids::uuid(uuidKey));

            } catch(std::exception &e) {
                throw IOError(e.what());
            }

            BytesShared transactionBuffer(const_cast<byte *>(record->data()));

            BaseTransaction *baseTransaction = nullptr;
            try{
                uint16_t *type = new (transactionBuffer.get()) uint16_t;
                BaseTransaction::TransactionType transactionType = (BaseTransaction::TransactionType) *type;
                switch (transactionType) {

                    case BaseTransaction::TransactionType::OpenTrustLineTransactionType: {
                        baseTransaction = new OpenTrustLineTransaction(
                            transactionBuffer,
                            mTransactionsScheduler,
                            mTrustLinesManager
                        );
                    }

                    case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                        baseTransaction = new SetTrustLineTransaction(
                            transactionBuffer,
                            mTransactionsScheduler,
                            mTrustLinesManager
                        );
                    }

                    case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                        baseTransaction = new CloseTrustLineTransaction(
                            transactionBuffer,
                            mTransactionsScheduler,
                            mTrustLinesManager
                        );
                    }

                    case BaseTransaction::TransactionType::AcceptTrustLineTransactionType: {
                        baseTransaction = new AcceptTrustLineTransaction(
                            transactionBuffer,
                            mTransactionsScheduler,
                            mTrustLinesManager
                        );
                    }

                    case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
                        baseTransaction = new UpdateTrustLineTransaction(
                            transactionBuffer,
                            mTransactionsScheduler,
                            mTrustLinesManager
                        );
                    }

                    case BaseTransaction::TransactionType::RejectTrustLineTransactionType: {
                        baseTransaction = new RejectTrustLineTransaction(
                            transactionBuffer,
                            mTransactionsScheduler,
                            mTrustLinesManager
                        );
                    }

                    default: {
                        throw ConflictError("TrustLinesManager::loadTransactions. "
                                                "Unexpected transaction type identifier.");
                    }

                }

            } catch (...) {
                throw Exception("TrustLinesManager::loadTransactions. "
                                    "Unable to create transaction instance from buffer.");
            }
            baseTransaction->addOnMessageSendSlot(
                boost::bind(
                    &TransactionsManager::onMessageSend,
                    this,
                    _1,
                    _2
                )
            );
            BaseTransaction::Shared baseTransactionShared(baseTransaction);
            mTransactionsScheduler->scheduleTransaction(baseTransactionShared);
        }
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

void TransactionsManager::createSetTrustLineTransaction(
    BaseUserCommand::Shared command) {

    SetTrustLineCommand::Shared setTrustLineCommand = static_pointer_cast<SetTrustLineCommand>(command);

    try {
        BaseTransaction *baseTransaction = new SetTrustLineTransaction(
            mNodeUUID,
            setTrustLineCommand,
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

void TransactionsManager::createUpdateTrustLineTransaction(
    Message::Shared message) {

    UpdateTrustLineMessage::Shared updateTrustLineMessage = static_pointer_cast<UpdateTrustLineMessage>(message);

    try {
        BaseTransaction *baseTransaction = new UpdateTrustLineTransaction(
            mNodeUUID,
            updateTrustLineMessage,
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

void TransactionsManager::zeroPointers() {

    mStorage = nullptr;
    mTransactionsScheduler = nullptr;
}

void TransactionsManager::cleanupMemory() {

    if (mTransactionsScheduler != nullptr) {
        delete mTransactionsScheduler;
    }

    if (mStorage != nullptr) {
        delete mStorage;
    }
}