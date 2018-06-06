#include "InitialAuditSourceTransaction.h"

InitialAuditSourceTransaction::InitialAuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::InitialAuditSourceTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeyChain(KeyChain::makeKeyChain(manager->trustLineReadOnly(contractorUUID)->trustLineID(), logger))
{}

TransactionResult::SharedConst InitialAuditSourceTransaction::run()
{
    info() << mStep;
    switch (mStep) {
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst InitialAuditSourceTransaction::runInitialisationStage()
{
    auto serializedAuditData = serializeAuditData();
    auto ioTransaction = mStorageHandler->beginTransaction();
    std::tie(ownSignedData, ownSignedDataSize, ownKeyNumber) = mKeyChain.signData(
        ioTransaction,
        serializedAuditData.first,
        serializedAuditData.second);

    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        ownKeyNumber,
        ownSignedDataSize,
        ownSignedData);
    info() << "Send audit message signed by key " << ownKeyNumber;
    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_Audit},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst InitialAuditSourceTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "No audit confirmation message received";
        return resultDone();
    }

    auto message = popNextMessage<AuditMessage>();
    auto ioTransaction = mStorageHandler->beginTransaction();
    bool isDataCorrect;
    BytesShared rowAuditData;
    size_t rowAuditDataBytesCount;
    std::tie(isDataCorrect, rowAuditData, rowAuditDataBytesCount) = mKeyChain.checkSignedData(
        ioTransaction,
        message->signedData(),
        message->signedDataSize(),
        message->keyNumber());

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
        // todo mark key as used
        // todo save audit
        mTrustLines->setTrustLineState(
            ioTransaction,
            mContractorUUID,
            TrustLine::Active);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't save Audit or update TL on storage. Details: " << e.what();
        return resultDone();
    }
    info() << "All data saved. Now TL is ready for using";
    return resultDone();
}

pair<BytesShared, size_t> InitialAuditSourceTransaction::serializeAuditData()
{
    size_t bytesCount = kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmountDespiteReservations(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmountDespiteReservations(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mTrustLines->balance(mContractorUUID)));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

bool InitialAuditSourceTransaction::deserializeAuditDataAndCheck(
    BytesShared serializedData)
{
    size_t bytesBufferOffset = 0;
    vector<byte> incomingAmountBytes(
        serializedData.get() + bytesBufferOffset,
        serializedData.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    auto incomingAmount = bytesToTrustLineAmount(incomingAmountBytes);
    info() << "deserialized incoming amount: " << incomingAmount;
    if (mTrustLines->outgoingTrustAmountDespiteReservations(mContractorUUID) != incomingAmount) {
        warning() << "Contractor send wrong incoming amount";
        return false;
    }
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBytes(
        serializedData.get() + bytesBufferOffset,
        serializedData.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    auto outgoingAmount = bytesToTrustLineAmount(outgoingAmountBytes);
    info() << "deserialized outgoing amount: " << outgoingAmount;
    if (mTrustLines->incomingTrustAmountDespiteReservations(mContractorUUID) != outgoingAmount) {
        warning() << "Contractor send wrong outgoing amount";
        return false;
    }
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBufferBytes(
        serializedData.get() + bytesBufferOffset,
        serializedData.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);
    TrustLineBalance balance = bytesToTrustLineBalance(balanceBufferBytes);
    info() << "deserialized balance: " << balance;
    if (mTrustLines->balance(mContractorUUID) != balance) {
        warning() << "Contractor send wrong balance";
        return false;
    }
    info() << "all data correct";
    return true;
}

const string InitialAuditSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[InitialAuditSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}