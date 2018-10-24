#include "CloseIncomingTrustLineTransaction.h"

CloseIncomingTrustLineTransaction::CloseIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseIncomingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept :

    BaseTrustLineTransaction(
        BaseTransaction::CloseIncomingTrustLineTransactionType,
        nodeUUID,
        command->equivalent(),
        command->contractorUUID(),
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCommand(command),
    mCountSendingAttempts(0),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

CloseIncomingTrustLineTransaction::CloseIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::CloseIncomingTrustLineTransactionType,
        nodeUUID,
        equivalent,
        contractorUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCountSendingAttempts(0),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{
    mStep = Stages::AddToBlackList;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        case Stages::AddToBlackList: {
            return runAddToBlackListStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runInitializationStage()
{
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    try {
        if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
            return resultProtocolError();
        }
    } catch (NotFoundError &e) {
        warning() << "Attempt to change not existing TL";
        return resultProtocolError();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorUUID)) {
        warning() << "There are no own keys";
        return resultKeysError();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorUUID)) {
        warning() << "There are no contractor keys";
        return resultKeysError();
    }

    mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorUUID);
    mPreviousState = mTrustLines->trustLineState(mContractorUUID);

    // Trust line must be updated in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.

    mTrustLines->closeIncoming(
        mContractorUUID);

    mTrustLines->setTrustLineState(
        mContractorUUID,
        TrustLine::AuditPending);

    // remove this TL from Topology TrustLines Manager
    mTopologyTrustLinesManager->addTrustLine(
        make_shared<TopologyTrustLine>(
            mNodeUUID,
            mContractorUUID,
            make_shared<const TrustLineAmount>(0)));
    mTopologyCacheManager->resetInitiatorCache();
    mMaxFlowCacheManager->clearCashes();
    info() << "Incoming trust line from the node " << mContractorUUID
           << " successfully closed.";

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto serializedAuditData = getOwnSerializedAuditData();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        keyChain.saveOwnAuditPart(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            mTrustLines->incomingTrustAmount(
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(
                mContractorUUID));

        mTrustLines->setTrustLineAuditNumber(
            mContractorUUID,
            mAuditNumber);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceInitializationStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceInitializationStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->setIncoming(
            mContractorUUID,
            mPreviousIncomingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        warning() << "Attempt to close incoming TL from the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mTrustLines->incomingTrustAmount(mContractorUUID),
        mTrustLines->outgoingTrustAmount(mContractorUUID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    mCountSendingAttempts++;

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<AuditMessage>(
                mContractorUUID,
                mEquivalent,
                mNodeUUID,
                mTransactionUUID,
                mContractorUUID,
                mTrustLines->incomingTrustAmount(mContractorUUID),
                mTrustLines->outgoingTrustAmount(mContractorUUID),
                mOwnSignatureAndKeyNumber.second,
                mOwnSignatureAndKeyNumber.first);
            mCountSendingAttempts++;
            info() << "Send message " << mCountSendingAttempts << " times";
            return resultWaitForMessageTypes(
                {Message::TrustLines_AuditConfirmation},
                kWaitMillisecondsForResponse);
        }
        sendMessage<PingMessage>(
            mCommand->contractorUUID(),
            0,
            mNodeUUID);
        info() << "Transaction will be closed and send ping";
        return resultDone();
    }
    auto message = popNextMessage<AuditResponseMessage>();
    info() << "contractor " << message->senderUUID << " confirmed closing incoming TL.";
    if (message->senderUUID != mContractorUUID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }

    if (message->state() == ConfirmationMessage::ReservationsPresentOnTrustLine) {
        info() << "Contractor's TL is not ready for audit yet";
        // message on communicator queue, wait for audit response after reservations committing or cancelling
        // todo add timeout or count failed attempts for running conflict resolver TA
        return resultWaitForMessageTypes(
            {Message::TrustLines_AuditConfirmation},
            kWaitMillisecondsForResponse);
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {

        // todo process ConfirmationMessage::OwnKeysAbsent and ConfirmationMessage::ContractorKeysAbsent

        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept closing incoming TL. Response code: " << message->state();
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceProcessingResponseStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceProcessingResponseStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
#endif

        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                message->signature(),
                message->keyNumber())) {
            warning() << "Contractor didn't sign message correct";
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolver TA
            return resultDone();
        }

        keyChain.saveContractorAuditPart(
            ioTransaction,
            mAuditNumber,
            message->keyNumber(),
            message->signature());

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);

        populateHistory(ioTransaction, TrustLineRecord::ClosingIncoming);

    } catch (ValueError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::ConflictResolving,
            ioTransaction);
        // todo need correct reaction
        error() << "Attempt to save audit from contractor " << mContractorUUID << " failed. "
                << "Details are: " << e.what();
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::ConflictResolving);
        // todo need correct reaction
        error() << "Attempt to process confirmation from contractor " << message->senderUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        return resultDone();
    }

    trustLineActionSignal(
        mContractorUUID,
        mEquivalent,
        false);

    return resultDone();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runAddToBlackListStage()
{
    info() << "Close incoming TL to " << mContractorUUID << " after adding to black list";
    try {
        mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorUUID);
        mPreviousState = mTrustLines->trustLineState(mContractorUUID);
    } catch (NotFoundError &e) {
        warning() << "Attempt to change not existing TL";
        return resultDone();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorUUID)) {
        warning() << "There are no own keys";
        return resultDone();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorUUID)) {
        warning() << "There are no contractor keys";
        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    mTrustLines->closeIncoming(
        mContractorUUID);

    mTrustLines->setTrustLineState(
        mContractorUUID,
        TrustLine::AuditPending);

    // remove this TL from Topology TrustLines Manager
    mTopologyTrustLinesManager->addTrustLine(
        make_shared<TopologyTrustLine>(
            mNodeUUID,
            mContractorUUID,
            make_shared<const TrustLineAmount>(0)));
    mTopologyCacheManager->resetInitiatorCache();
    mMaxFlowCacheManager->clearCashes();
    info() << "Incoming trust line from the node " << mContractorUUID
           << " successfully closed.";

    // note: io transaction would commit automatically on destructor call.
    // there is no need to call commit manually.
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto serializedAuditData = getOwnSerializedAuditData();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));

    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        keyChain.saveOwnAuditPart(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            mTrustLines->incomingTrustAmount(
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(
                mContractorUUID));

        mTrustLines->setTrustLineAuditNumber(
            mContractorUUID,
            mAuditNumber);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceInitializationStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceInitializationStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->setIncoming(
            mContractorUUID,
            mPreviousIncomingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        warning() << "Attempt to close incoming TL from the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mTrustLines->incomingTrustAmount(mContractorUUID),
        mTrustLines->outgoingTrustAmount(mContractorUUID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    mCountSendingAttempts++;

    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultKeysError()
{
    return transactionResultFromCommand(
        mCommand->responseThereAreNoKeys());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string CloseIncomingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[CloseIncomingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void CloseIncomingTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mContractorUUID);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
