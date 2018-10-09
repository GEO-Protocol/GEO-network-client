#include "PublicKeysSharingTargetTransaction.h"

PublicKeysSharingTargetTransaction::PublicKeysSharingTargetTransaction(
    const NodeUUID &nodeUUID,
    PublicKeysSharingInitMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::PublicKeysSharingTargetTransactionType,
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
    mContractorKeysCount = message->keysCount();
    mCurrentKeyNumber = message->number();
    mCurrentPublicKey = message->publicKey();
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
    mStep = KeysSharingTargetInitialization;
}

PublicKeysSharingTargetTransaction::PublicKeysSharingTargetTransaction(
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

    auto *contractorKeysCount = new (buffer.get() + bytesBufferOffset) KeysCount;
    mContractorKeysCount = (KeysCount) *contractorKeysCount;
    bytesBufferOffset += sizeof(KeysCount);

    auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
    mCurrentKeyNumber = (KeyNumber) *keyNumber;

    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::run()
{
    switch (mStep) {
        case Stages::KeysSharingTargetInitialization: {
            mShouldPopPublicKeyMessage = false;
            return runPublicKeyReceiverInitStage();
        }
        case Stages::KeysSharingTargetNextKey: {
            return runReceiveNextKeyStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runReceiveNextKeyStage()
{
    info() << "runReceiveNextKeyStage";
    if (!mShouldPopPublicKeyMessage) {
        mShouldPopPublicKeyMessage = true;
        return runPublicKeyReceiverStage();
    }
    if (mContext.empty()) {
        warning() << "No next public key message received. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeyMessage>();
    mCurrentPublicKey = message->publicKey();
    mCurrentKeyNumber = message->number();
    return runPublicKeyReceiverStage();
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runRecoveryStage()
{
    info() << "Recovery. Contractor " << mContractorUUID;
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent " << mContractorUUID;
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending
            or mTrustLines->trustLineState(mContractorUUID) == TrustLine::Active) {
        info() << "Keys pending state, current key number " << mCurrentKeyNumber
               << " contractor keys count " << mContractorKeysCount;
        mStep = KeysSharingTargetNextKey;
        return resultAwakeAsFastAsPossible();
    }
    warning() << "Invalid TL state for this TA state: "
              << mTrustLines->trustLineState(mContractorUUID);
    return resultDone();
}

pair<BytesShared, size_t> PublicKeysSharingTargetTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(KeysCount)
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
        &mContractorKeysCount,
        sizeof(KeysCount));
    dataBytesOffset += sizeof(KeysCount);

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mCurrentKeyNumber,
        sizeof(KeyNumber));

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const string PublicKeysSharingTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeySharingTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}