#include "BaseTrustLineTransaction.h"

BaseTrustLineTransaction::BaseTrustLineTransaction(
    const TransactionType type,
    const SerializedEquivalent equivalent,
    ContractorID contractorID,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    FeaturesManager *featuresManager,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log) :

    BaseTransaction(
        type,
        equivalent,
        log),
    mContractorID(contractorID),
    mContractorsManager(contractorsManager),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mFeaturesManager(featuresManager),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

BaseTrustLineTransaction::BaseTrustLineTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const SerializedEquivalent equivalent,
    ContractorID contractorID,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    FeaturesManager *featuresManager,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log) :

    BaseTransaction(
        type,
        transactionUUID,
        equivalent,
        log),
    mContractorID(contractorID),
    mContractorsManager(contractorsManager),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mFeaturesManager(featuresManager),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

TransactionResult::SharedConst BaseTrustLineTransaction::sendAuditErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<AuditResponseMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->idOnContractorSide(mContractorID),
        currentTransactionUUID(),
        errorState);
    return resultDone();
}

pair<BytesShared, size_t> BaseTrustLineTransaction::getOwnSerializedAuditData(
    lamport::KeyHash::Shared ownPublicKeysHash,
    lamport::KeyHash::Shared contractorPublicKeysHash)
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount
                        + lamport::KeyHash::kBytesSize
                        + lamport::KeyHash::kBytesSize
                        + sizeof(EquivalentRegisterAddressLength)
                        + mFeaturesManager->getEquivalentsRegistryAddress().length();
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);
    info() << "own audit " << mAuditNumber;

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mContractorID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own incoming amount " << mTrustLines->incomingTrustAmount(mContractorID);

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mContractorID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own outgoing amount " << mTrustLines->outgoingTrustAmount(mContractorID);

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mTrustLines->balance(mContractorID)));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    dataBytesOffset += kTrustLineBalanceSerializeBytesCount;
    info() << "own balance " << mTrustLines->balance(mContractorID);

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        ownPublicKeysHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;
    info() << "Own keys hash: " << ownPublicKeysHash->toString();

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        contractorPublicKeysHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;
    info() << "Contractor keys hash: " << contractorPublicKeysHash->toString();

    auto equivalentRegistryAddress = mFeaturesManager->getEquivalentsRegistryAddress();
    auto equivalentsRegistryAddressLength = (EquivalentRegisterAddressLength)equivalentRegistryAddress.length();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &equivalentsRegistryAddressLength,
        sizeof(EquivalentRegisterAddressLength));
    dataBytesOffset += sizeof(EquivalentRegisterAddressLength);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        equivalentRegistryAddress.c_str(),
        equivalentRegistryAddress.length());
    info() << "own equivalents registry address: " << equivalentRegistryAddress;

    return make_pair(
        dataBytesShared,
        bytesCount);
}

pair<BytesShared, size_t> BaseTrustLineTransaction::getContractorSerializedAuditData(
    lamport::KeyHash::Shared ownPublicKeysHash,
    lamport::KeyHash::Shared contractorPublicKeysHash)
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount
                        + lamport::KeyHash::kBytesSize
                        + lamport::KeyHash::kBytesSize
                        + sizeof(EquivalentRegisterAddressLength)
                        + mFeaturesManager->getEquivalentsRegistryAddress().length();
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);
    info() << "contractor audit " << mAuditNumber;

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mContractorID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor outgoing amount " << mTrustLines->outgoingTrustAmount(mContractorID);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mContractorID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor incoming amount " << mTrustLines->incomingTrustAmount(mContractorID);

    auto contractorBalance = -1 * mTrustLines->balance(mContractorID);
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(contractorBalance));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    dataBytesOffset += kTrustLineBalanceSerializeBytesCount;
    info() << "contractor balance " << contractorBalance;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        contractorPublicKeysHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;
    info() << "Contractor keys hash: " << contractorPublicKeysHash->toString();

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        ownPublicKeysHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;
    info() << "Own keys hash: " << ownPublicKeysHash->toString();

    auto equivalentRegistryAddress = mFeaturesManager->getEquivalentsRegistryAddress();
    auto equivalentsRegistryAddressLength = (EquivalentRegisterAddressLength)equivalentRegistryAddress.length();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &equivalentsRegistryAddressLength,
        sizeof(EquivalentRegisterAddressLength));
    dataBytesOffset += sizeof(EquivalentRegisterAddressLength);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        equivalentRegistryAddress.c_str(),
        equivalentRegistryAddress.length());
    info() << "contractor equivalents registry address: " << equivalentRegistryAddress;

    return make_pair(
        dataBytesShared,
        bytesCount);
}