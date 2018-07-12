#include "PublicKeysSharingSourceTransaction.h"

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::PublicKeysSharingSourceTransactionType,
        nodeUUID,
        equivalent,
        manager,
        storageHandler,
        keystore,
        logger)
{
    mContractorUUID = contractorUUID;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
    mStep = KeysSharingInitialization;
}

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
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

    auto publicKey = make_shared<lamport::PublicKey>(
        buffer.get() + bytesBufferOffset);
    mCurrentPublicKey = publicKey;
    bytesBufferOffset += mCurrentPublicKey->keySize();

    auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
    mCurrentKeyNumber = (KeyNumber) *keyNumber;
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::run()
{
    switch (mStep) {
        case Stages::KeysSharingInitialization: {
            return runPublicKeysSharingInitialisationStage();
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

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent " << mContractorUUID;
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::KeysPending) {
        warning() << "Invalid TL state for this TA state: "
                  << mTrustLines->trustLineState(mContractorUUID);
        return resultDone();
    }
    mStep = NextKeyProcessing;
    return runPublicKeysSendNextKeyStage();
}

pair<BytesShared, size_t> PublicKeysSharingSourceTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + lamport::PublicKey::keySize()
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

    // todo save only key number and read public key from storage on recovery stage
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mCurrentPublicKey->data(),
        mCurrentPublicKey->keySize());
    dataBytesOffset += mCurrentPublicKey->keySize();

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mCurrentKeyNumber,
        sizeof(KeyNumber));

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const string PublicKeysSharingSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeysSharingSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}