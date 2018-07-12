#include "AuditSourceTransaction.h"

AuditSourceTransaction::AuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::AuditSourceTransactionType,
        nodeUUID,
        equivalent,
        manager,
        storageHandler,
        keystore,
        logger)
{
    mStep = Stages::AuditInitialisation;
    mContractorUUID = contractorUUID;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

AuditSourceTransaction::AuditSourceTransaction(
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
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        &mAuditNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(AuditNumber));
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

TransactionResult::SharedConst AuditSourceTransaction::run()
{
    switch (mStep) {
        case Stages::AuditInitialisation: {
            return runAuditInitializationStage();
        }
        case Stages::AuditResponseProcessing: {
            return runAuditResponseProcessingStage();
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
    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::AuditPending) {
        warning() << "Invalid TL state for this TA state: "
                  << mTrustLines->trustLineState(mContractorUUID);
        return resultDone();
    }
    mStep = AuditResponseProcessing;
    return runAuditResponseProcessingStage();
}

pair<BytesShared, size_t> AuditSourceTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(AuditNumber)
                        + lamport::Signature::signatureSize()
                        + sizeof(KeyNumber);

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