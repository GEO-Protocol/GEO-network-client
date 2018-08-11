#include "CloseOutgoingTrustLineTransaction.h"

CloseOutgoingTrustLineTransaction::CloseOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseOutgoingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore* keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::CloseOutgoingTrustLineTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

CloseOutgoingTrustLineTransaction::CloseOutgoingTrustLineTransaction(
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

    memcpy(
        &mAuditNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(AuditNumber));

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesBufferOffset += sizeof(AuditNumber);

        auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
        mCurrentKeyNumber = (KeyNumber) *keyNumber;
    }
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::run()
{
    info() << mStep;
    switch (mStep) {
        case Stages::TrustLineInitialization: {
            return runInitializationStage();
        }
        case Stages::AuditTarget: {
            return runReceiveAuditStage();
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
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::runInitializationStage()
{
    info() << "sender: " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    TrustLine::ConstShared previousTL = mTrustLines->trustLineReadOnly(mContractorUUID);
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending,
            ioTransaction);

        mTrustLines->closeOutgoing(
            mContractorUUID);

        populateHistory(ioTransaction, TrustLineRecord::RejectingOutgoing);
        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();
        info() << "Outgoing trust line to the node " << mContractorUUID
               << " has been successfully closed by remote node.";

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->setOutgoing(
            mContractorUUID,
            previousTL->outgoingTrustAmount());
        mTrustLines->setTrustLineState(
            mContractorUUID,
            previousTL->state());
        warning() << "Attempt to close outgoing trust line to the node " << mContractorUUID << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();
        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Sending confirmation back.
    sendMessageWithCaching<TrustLineConfirmationMessage>(
        mContractorUUID,
        Message::TrustLines_CloseOutgoing,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        false,
        ConfirmationMessage::OK);

    info() << "Wait for audit";
    mStep = AuditTarget;
    return resultWaitForMessageTypes(
        {Message::TrustLines_Audit},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::runReceiveAuditStage()
{
    info() << "runReceiveAuditStage";
    if (mContext.empty()) {
        warning() << "No audit message received. Transaction will be closed, and wait for message";
        return resultDone();
    }
    mAuditMessage = popNextMessage<AuditMessage>();
    return runAuditTargetStage();
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent " << mContractorUUID;
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        info() << "Audit pending stage";
        mTrustLines->closeOutgoing(mContractorUUID);
        mStep = AuditTarget;
        return runReceiveAuditStage();
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

    warning() << "Invalid TL state for this TA: "
              << mTrustLines->trustLineState(mContractorUUID);
    return resultDone();
}

pair<BytesShared, size_t> CloseOutgoingTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(AuditNumber);
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
        &mAuditNumber,
        sizeof(AuditNumber));

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

const string CloseOutgoingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[CloseOutgoingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void CloseOutgoingTrustLineTransaction::populateHistory(
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
