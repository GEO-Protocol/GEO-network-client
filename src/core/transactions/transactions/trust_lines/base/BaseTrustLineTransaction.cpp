#include "BaseTrustLineTransaction.h"

BaseTrustLineTransaction::BaseTrustLineTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    ContractorID contractorID,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log) :

    BaseTransaction(
        type,
        currentNodeUUID,
        equivalent,
        log),
    mContractorID(contractorID),
    mContractorsManager(contractorsManager),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

BaseTrustLineTransaction::BaseTrustLineTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    ContractorID contractorID,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        equivalent,
        log),
    mContractorID(contractorID),
    mContractorsManager(contractorsManager),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

TransactionResult::SharedConst BaseTrustLineTransaction::sendAuditErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<AuditResponseMessage>(
        mContractorID,
        mEquivalent,
        mNodeUUID,
        mContractorsManager->idOnContractorSide(mContractorID),
        currentTransactionUUID(),
        errorState);
    return resultDone();
}

pair<BytesShared, size_t> BaseTrustLineTransaction::getOwnSerializedAuditData()
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
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
    info() << "own balance " << mTrustLines->balance(mContractorID);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

pair<BytesShared, size_t> BaseTrustLineTransaction::getContractorSerializedAuditData()
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
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
    info() << "contractor balance " << contractorBalance;

    return make_pair(
        dataBytesShared,
        bytesCount);
}