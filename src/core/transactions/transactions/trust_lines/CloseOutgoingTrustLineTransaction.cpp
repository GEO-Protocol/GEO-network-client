#include "CloseOutgoingTrustLineTransaction.h"

CloseOutgoingTrustLineTransaction::CloseOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseOutgoingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore* keystore,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::CloseOutgoingTrustLineTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        manager,
        storageHandler,
        keystore,
        logger),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{
    mContractorUUID = message->senderUUID;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

CloseOutgoingTrustLineTransaction::CloseOutgoingTrustLineTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :

    BaseTrustLineTransaction(
        buffer,
        nodeUUID,
        manager,
        storageHandler,
        keystore,
        logger)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Recovery;

    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::run()
{
    info() << mStep;
    switch (mStep) {
        case Stages::TrustLineInitialisation: {
            return runInitializationStage();
        }
        case Stages::AuditTarget: {
            return runReceiveAuditStage();
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
        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
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

        // Sending confirmation back.
        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
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
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Modify) {
        info() << "waiting for audit";
        mStep = AuditTarget;
        return resultWaitForMessageTypes(
            {Message::TrustLines_Audit},
            kWaitMillisecondsForResponse);
    } else {
        warning() << "Invalid TL state for this TA: "
                  << mTrustLines->trustLineState(mContractorUUID);
        return resultDone();
    }
}

pair<BytesShared, size_t> CloseOutgoingTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize;

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
