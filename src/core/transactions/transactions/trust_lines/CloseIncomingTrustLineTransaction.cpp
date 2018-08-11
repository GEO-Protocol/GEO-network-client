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
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController)
{
    mPreviousState = TrustLine::Active;
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
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{
    mStep = Stages::AddToBlackList;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

CloseIncomingTrustLineTransaction::CloseIncomingTrustLineTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :

    BaseTrustLineTransaction(
        buffer,
        nodeUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Recovery;

    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    auto *trustLineState = new (buffer.get()) TrustLine::SerializedTrustLineState;
    mPreviousState = (TrustLine::TrustLineState) *trustLineState;
    bytesBufferOffset += sizeof(TrustLine::SerializedTrustLineState);

    vector<byte> previousAmountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mPreviousIncomingAmount = bytesToTrustLineAmount(previousAmountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    memcpy(
        &mAuditNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(AuditNumber));

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesBufferOffset += sizeof(AuditNumber);

        // todo signature can be taken from storage on recovery stage
        auto signature = make_shared<lamport::Signature>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += lamport::Signature::signatureSize();

        KeyNumber keyNumber;
        memcpy(
            &keyNumber,
            buffer.get() + bytesBufferOffset,
            sizeof(KeyNumber));

        mOwnSignatureAndKeyNumber = make_pair(
            signature,
            keyNumber);
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesBufferOffset += sizeof(AuditNumber);

        auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
        mCurrentKeyNumber = (KeyNumber) *keyNumber;
    }
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::run()
{
    switch (mStep) {
        case Stages::TrustLineInitialization: {
            return runInitializationStage();
        }
        case Stages::TrustLineResponseProcessing: {
            return runResponseProcessingStage();
        }
        case Stages::AuditInitialization: {
            return runAuditInitializationStage();
        }
        case Stages::AuditResponseProcessing: {
            return runAuditResponseProcessingStage();
        }
        case Stages::KeysSharingInitialization: {
            return runPublicKeysSharingInitializationStage();
        }
        case Stages::NextKeyProcessing: {
            return runPublicKeysSendNextKeyStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
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

    mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorUUID);
    mPreviousState = mTrustLines->trustLineState(mContractorUUID);

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->closeIncoming(
            mContractorUUID);

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Modify,
            ioTransaction);

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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";

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
    sendMessage<CloseOutgoingTrustLineMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID);

    mStep = TrustLineResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
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
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept closing incoming TL. Response code: " << message->state();
            mTrustLines->setIncoming(
                mContractorUUID,
                mPreviousIncomingAmount);
            mTrustLines->setTrustLineState(
                mContractorUUID,
                mPreviousState,
                ioTransaction);
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());
            processConfirmationMessage(message);
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLProcessingResponseStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLProcessingResponseStage();
#endif

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending,
            ioTransaction);

        populateHistory(ioTransaction, TrustLineRecord::ClosingIncoming);

        // delete this transaction from storage
        ioTransaction->transactionHandler()->deleteRecord(
            currentTransactionUUID());
    } catch (IOError &e) {
        ioTransaction->rollback();
        // todo : need return intermediate state of TL
        error() << "Attempt to process confirmation from contractor " << message->senderUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        return resultDone();
    }

    processConfirmationMessage(message);
    mStep = AuditInitialization;
    return resultAwakeAsFastAsPossible();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Modify) {
        info() << "Modify stage";
        mTrustLines->closeIncoming(
            mContractorUUID);
        mStep = TrustLineResponseProcessing;
        return runResponseProcessingStage();
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        info() << "Audit stage";
        mTrustLines->closeIncoming(
            mContractorUUID);
        mStep = AuditResponseProcessing;
        return runAuditResponseProcessingStage();
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        info() << "Keys pending state, current key number " << mCurrentKeyNumber;
        auto keyChain = mKeysStore->keychain(mTrustLines->trustLineID(mContractorUUID));
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            mCurrentPublicKey = keyChain.publicKey(
                ioTransaction,
                mCurrentKeyNumber);
            if (mCurrentPublicKey == nullptr) {
                warning() << "Can't get own public key with number " << mCurrentKeyNumber;
                return resultDone();
            }
            mStep = NextKeyProcessing;
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't get own public key from storage. Details: " << e.what();
            return resultDone();
        }
        return resultAwakeAsFastAsPossible();
    }

    warning() << "Invalid TL state for this TA state: "
              << mTrustLines->trustLineState(mContractorUUID);
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

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->closeIncoming(
            mContractorUUID);

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Modify,
            ioTransaction);

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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";

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
    sendMessage<CloseOutgoingTrustLineMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID);

    mStep = TrustLineResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
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

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

pair<BytesShared, size_t> CloseIncomingTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(TrustLine::SerializedTrustLineState)
                        + kTrustLineAmountBytesCount
                        + sizeof(AuditNumber);
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesCount += lamport::Signature::signatureSize()
                      + sizeof(KeyNumber);
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesCount += sizeof(KeyNumber);
    }

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize);
    dataBytesOffset += NodeUUID::kBytesSize;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mPreviousState,
        sizeof(TrustLine::SerializedTrustLineState));
    dataBytesOffset += sizeof(TrustLine::SerializedTrustLineState);

    vector<byte> buffer = trustLineAmountToBytes(mPreviousIncomingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        dataBytesOffset += sizeof(AuditNumber);

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mOwnSignatureAndKeyNumber.first->data(),
            lamport::Signature::signatureSize());
        dataBytesOffset += lamport::Signature::signatureSize();

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mOwnSignatureAndKeyNumber.second,
            sizeof(KeyNumber));
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        dataBytesOffset += sizeof(AuditNumber);

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mCurrentKeyNumber,
            sizeof(KeyNumber));
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
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
