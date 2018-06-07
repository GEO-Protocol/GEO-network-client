#include "InitialAuditTargetTransaction.h"

InitialAuditTargetTransaction::InitialAuditTargetTransaction(
    const NodeUUID &nodeUUID,
    AuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::InitialAuditTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeyChain(KeyChain::makeKeyChain(
        manager->trustLineReadOnly(message->senderUUID)->trustLineID(),
        logger))
{}

TransactionResult::SharedConst InitialAuditTargetTransaction::run()
{
    info() << "run";
    auto ioTransaction = mStorageHandler->beginTransaction();
    bool isDataCorrect;
    BytesShared rowAuditData;
    size_t rowAuditDataBytesCount;
    std::tie(isDataCorrect, rowAuditData, rowAuditDataBytesCount) = mKeyChain.checkSignedData(
        ioTransaction,
        mMessage->signedData(),
        mMessage->signedDataSize(),
        mMessage->keyNumber());

    if (!isDataCorrect) {
        warning() << "Contractor didn't sign message correct";
        return resultDone();
    }
    info() << "Sign is correct";
    if (!deserializeAuditDataAndCheck(rowAuditData)) {
        warning() << "Contractor sign wrong data";
        return resultDone();
    }
    info() << "Signed data is correct";
    try {
        // todo mark contractor key as used
        mTrustLines->setTrustLineState(
            ioTransaction,
            mMessage->senderUUID,
            TrustLine::Active);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't save Audit or update TL on storage. Details: " << e.what();
        return resultDone();
    }
    info() << "All data saved. Now TL is ready for using";

    auto serializedAuditData = serializeAuditData();
    BytesShared ownSignedData;
    size_t ownSignedDataSize;
    uint32_t ownKeyNumber;
    std::tie(ownSignedData, ownSignedDataSize, ownKeyNumber) = mKeyChain.signData(
        ioTransaction,
        serializedAuditData.first,
        serializedAuditData.second);

    sendMessage<AuditMessage>(
        mMessage->senderUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        ownKeyNumber,
        ownSignedDataSize,
        ownSignedData);
    info() << "Send audit message signed by key " << ownKeyNumber;
    return resultDone();
}

pair<BytesShared, size_t> InitialAuditTargetTransaction::serializeAuditData()
{
    size_t bytesCount = kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmountDespiteReservations(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmountDespiteReservations(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mTrustLines->balance(mMessage->senderUUID)));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

bool InitialAuditTargetTransaction::deserializeAuditDataAndCheck(
    BytesShared serializedData)
{
    size_t bytesBufferOffset = 0;
    vector<byte> incomingAmountBytes(
        serializedData.get() + bytesBufferOffset,
        serializedData.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    auto incomingAmount = bytesToTrustLineAmount(incomingAmountBytes);
    info() << "deserialized incoming amount: " << incomingAmount;
    if (mTrustLines->outgoingTrustAmountDespiteReservations(mMessage->senderUUID) != incomingAmount) {
        warning() << "Contractor send wrong incoming amount";
        return false;
    }
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBytes(
        serializedData.get() + bytesBufferOffset,
        serializedData.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    auto outgoingAmount = bytesToTrustLineAmount(outgoingAmountBytes);
    info() << "deserialized outgoing amount: " << outgoingAmount;
    if (mTrustLines->incomingTrustAmountDespiteReservations(mMessage->senderUUID) != outgoingAmount) {
        warning() << "Contractor send wrong outgoing amount";
        return false;
    }
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBufferBytes(
        serializedData.get() + bytesBufferOffset,
        serializedData.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);
    TrustLineBalance balance = bytesToTrustLineBalance(balanceBufferBytes);
    info() << "deserialized balance: " << balance;
    if (mTrustLines->balance(mMessage->senderUUID) != balance) {
        warning() << "Contractor send wrong balance";
        return false;
    }
    info() << "all data correct";
    return true;
}

const string InitialAuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[InitialAuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}