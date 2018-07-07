#include "AuditTargetTransaction.h"

AuditTargetTransaction::AuditTargetTransaction(
    const NodeUUID &nodeUUID,
    InitialAuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::AuditTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mAuditNumber(mTrustLines->auditNumber(mMessage->senderUUID) + 1)
{}

TransactionResult::SharedConst AuditTargetTransaction::run()
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
    try {
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

        mTrustLines->setTrustLineAuditNumberAndMakeActive(
            ioTransaction,
            mMessage->senderUUID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mMessage->senderUUID);
        if (mTrustLines->isTrustLineEmpty(
                mMessage->senderUUID)) {
            mTrustLines->setTrustLineState(
                mMessage->senderUUID,
                TrustLine::Archived,
                ioTransaction);
            info() << "TL is Archived";
        } else if (keyChain.ownKeysCriticalCount(ioTransaction)) {
            mTrustLines->setTrustLineState(
                mMessage->senderUUID,
                TrustLine::KeysPending,
                ioTransaction);
            info() << "Start sharing own keys";
            const auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
                mNodeUUID,
                mMessage->senderUUID,
                mEquivalent,
                mTrustLines,
                mStorageHandler,
                mKeysStore,
                mLog);
            launchSubsidiaryTransaction(transaction);
        }
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't update TL on storage. Details: " << e.what();
        return resultDone();
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
            mAuditNumber,
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
        return resultDone();
    }
    info() << "All data saved. Now TL is ready for using";

    sendMessage<AuditMessage>(
        mMessage->senderUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    return resultDone();
}

pair<BytesShared, size_t> AuditTargetTransaction::getOwnSerializedAuditData()
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
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own incoming amount " << mTrustLines->incomingTrustAmount(mMessage->senderUUID);

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own outgoing amount " << mTrustLines->outgoingTrustAmount(mMessage->senderUUID);

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mTrustLines->balance(mMessage->senderUUID)));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    info() << "own balance " << mTrustLines->balance(mMessage->senderUUID);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

pair<BytesShared, size_t> AuditTargetTransaction::getContractorSerializedAuditData()
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
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor outgoing amount " << mTrustLines->outgoingTrustAmount(mMessage->senderUUID);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mMessage->senderUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor incoming amount " << mTrustLines->incomingTrustAmount(mMessage->senderUUID);

    auto contractorBalance = (-1) * mTrustLines->balance(mMessage->senderUUID);
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

const string AuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}