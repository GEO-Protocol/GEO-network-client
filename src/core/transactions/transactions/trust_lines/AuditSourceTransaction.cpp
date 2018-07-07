#include "AuditSourceTransaction.h"

AuditSourceTransaction::AuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::AuditSourceTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mAuditNumber(mTrustLines->auditNumber(mContractorUUID) + 1)
{}

AuditSourceTransaction::AuditSourceTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :

    BaseTransaction(
        buffer,
        nodeUUID,
        logger),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
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
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst AuditSourceTransaction::runInitialisationStage()
{
    info() << "Contractor " << mContractorUUID;
    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::AuditPending) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        // todo implement actual reaction
        return resultDone();
    }

    auto serializedAuditData = getOwnSerializedAuditData();
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";
    } catch(IOError &e) {
        ioTransaction->rollback();
        error() << "Can't sign audit data. Details: " << e.what();
        return resultDone();
    }

    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_Audit},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst AuditSourceTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "No audit confirmation message received";
        return resultDone();
    }

    auto message = popNextMessage<AuditMessage>();
    info() << "Contractor send audit message";
    if (message->senderUUID != mContractorUUID) {
        warning() << "Receive message from different sender: " << message->senderUUID;
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Something wrong, because TL must be present";
        // todo : need correct reaction
        return resultDone();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));

    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {
        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept audit. Response code: " << message->state();
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());
            processConfirmationMessage(message);
            // todo run conflict resolver TA
            return resultDone();
        }

        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                message->signature(),
                message->keyNumber())) {
            warning() << "Contractor didn't sign message correct";
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());
            processConfirmationMessage(message);
            // todo run conflict resolver TA
            return resultDone();
        }
        info() << "Signature is correct";

        mTrustLines->setTrustLineAuditNumberAndMakeActive(
            ioTransaction,
            mContractorUUID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);
        // if TL is empty: incomingAmount, outgoingAmount and balance is 0
        // we marked it as Archived
        if (mTrustLines->isTrustLineEmpty(
            mContractorUUID)) {
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::Archived,
                ioTransaction);
            info() << "TL is Archived";
        } else if (keyChain.ownKeysCriticalCount(ioTransaction)) {
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::KeysPending,
                ioTransaction);
            info() << "Start sharing own keys";
            const auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
                mNodeUUID,
                mContractorUUID,
                mEquivalent,
                mTrustLines,
                mStorageHandler,
                mKeysStore,
                mLog);
            launchSubsidiaryTransaction(transaction);
        }

        keyChain.saveAudit(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            message->keyNumber(),
            message->signature(),
            mTrustLines->incomingTrustAmount(
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(mContractorUUID));
        // delete this transaction from storage
        ioTransaction->transactionHandler()->deleteRecord(
            currentTransactionUUID());
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't check signature, update TL on storage or save Audit. Details: " << e.what();
        return resultDone();
    }
    processConfirmationMessage(message);
    info() << "All data saved. Now TL is ready for using";
    sendMessage<ConfirmationMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID);
    return resultDone();
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
    mStep = ResponseProcessing;
    return runResponseProcessingStage();
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

pair<BytesShared, size_t> AuditSourceTransaction::getOwnSerializedAuditData()
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
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own incoming amount " << mTrustLines->incomingTrustAmount(mContractorUUID);

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own outgoing amount " << mTrustLines->outgoingTrustAmount(mContractorUUID);

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mTrustLines->balance(mContractorUUID)));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    info() << "own balance " << mTrustLines->balance(mContractorUUID);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

pair<BytesShared, size_t> AuditSourceTransaction::getContractorSerializedAuditData()
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
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor outgoing amount " << mTrustLines->outgoingTrustAmount(mContractorUUID);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor incoming amount " << mTrustLines->incomingTrustAmount(mContractorUUID);

    auto contractorBalance = -1 * mTrustLines->balance(mContractorUUID);
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

const string AuditSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}