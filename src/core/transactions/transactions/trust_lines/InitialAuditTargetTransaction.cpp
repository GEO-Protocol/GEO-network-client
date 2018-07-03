#include "InitialAuditTargetTransaction.h"

InitialAuditTargetTransaction::InitialAuditTargetTransaction(
    const NodeUUID &nodeUUID,
    InitialAuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
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
    mKeysStore(keystore)
{}

TransactionResult::SharedConst InitialAuditTargetTransaction::run()
{
    info() << "Audit initialized by " << mMessage->senderUUID;
    if (!mTrustLines->trustLineIsPresent(mMessage->senderUUID)) {
        warning() << "Trust Line is absent";
        sendMessage<InitialAuditMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    if (mTrustLines->trustLineState(mMessage->senderUUID) != TrustLine::AuditPending) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mMessage->senderUUID);
        // todo set TL state into conflict
        sendMessage<InitialAuditMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mMessage->senderUUID));

    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    if (!keyChain.checkSign(
            ioTransaction,
            contractorSerializedAuditData.first,
            contractorSerializedAuditData.second,
            mMessage->signature(),
            mMessage->keyNumber())) {
        warning() << "Contractor didn't sign message correct";
        // todo set TL state into conflict
        sendMessage<InitialAuditMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }
    info() << "Signature is correct";
    try {
        mTrustLines->setTrustLineState(
            mMessage->senderUUID,
            TrustLine::Active,
            ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't save Audit or update TL on storage. Details: " << e.what();
        throw e;
    }
    info() << "TL state updated";

    auto serializedAuditData = getOwnSerializedAuditData();
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        keyChain.saveAudit(
            ioTransaction,
            kInitialAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            mMessage->keyNumber(),
            mMessage->signature(),
            mTrustLines->incomingTrustAmount(
                mMessage->senderUUID),
            mTrustLines->outgoingTrustAmount(
                mMessage->senderUUID),
            mTrustLines->balance(mMessage->senderUUID));
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't sign audit data. Details: " << e.what();
        throw e;
    }
    info() << "All data saved. Now TL is ready for using";

    sendMessage<InitialAuditMessage>(
        mMessage->senderUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    return resultDone();
}

pair<BytesShared, size_t> InitialAuditTargetTransaction::getOwnSerializedAuditData()
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kInitialAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
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

pair<BytesShared, size_t> InitialAuditTargetTransaction::getContractorSerializedAuditData()
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kInitialAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    auto contractorBalance = (-1) * mTrustLines->balance(mMessage->senderUUID);
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(contractorBalance));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const string InitialAuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[InitialAuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}