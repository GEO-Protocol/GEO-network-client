#include "AuditSourceTransaction.h"

AuditSourceTransaction::AuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::AuditSourceTransactionType,
        nodeUUID,
        equivalent,
        contractorUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger)
{
    mStep = Stages::AuditInitialization;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

AuditSourceTransaction::AuditSourceTransaction(
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
        bytesBufferOffset += sizeof(AuditNumber);

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

TransactionResult::SharedConst AuditSourceTransaction::run()
{
    switch (mStep) {
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
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst AuditSourceTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        info() << "Audit pending stage";
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

pair<BytesShared, size_t> AuditSourceTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
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

const string AuditSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}