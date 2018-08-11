#include "AuditTargetTransaction.h"

AuditTargetTransaction::AuditTargetTransaction(
    const NodeUUID &nodeUUID,
    AuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):
    BaseTrustLineTransaction(
        BaseTransaction::AuditTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger)
{
    mAuditNumber = mTrustLines->auditNumber(message->senderUUID) + 1;
    mAuditMessage = message;
    mStep = Stages::AuditTarget;
}

AuditTargetTransaction::AuditTargetTransaction(
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

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        //todo implement reaction on this case
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesBufferOffset += sizeof(AuditNumber);

        auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
        mCurrentKeyNumber = (KeyNumber) *keyNumber;
    }
}

TransactionResult::SharedConst AuditTargetTransaction::run()
{
    switch (mStep) {
        case Stages::AuditTarget: {
            return runInitializationStage();
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

TransactionResult::SharedConst AuditTargetTransaction::runInitializationStage()
{
    info() << "runInitializationStage";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust Line is absent";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (mTrustLines->isReservationsPresentOnTrustLine(mContractorUUID)) {
        info() << "there are reservations on TL";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ReservationsPresentOnTrustLine);
    }
    return runAuditTargetStage();
}

TransactionResult::SharedConst AuditTargetTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return resultDone();
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

pair<BytesShared, size_t> AuditTargetTransaction::serializeToBytes() const
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

const string AuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}