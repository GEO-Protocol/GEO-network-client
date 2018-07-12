#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::AcceptTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        manager,
        storageHandler,
        keystore,
        logger),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(false)
{
    mContractorUUID = message->senderUUID;
    mAmount = message->amount();
    mAuditNumber = kInitialAuditNumber;
}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineFromGatewayMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::AcceptTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        manager,
        storageHandler,
        keystore,
        logger),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(true)
{
    mContractorUUID = message->senderUUID;
    mAmount = message->amount();
    mAuditNumber = kInitialAuditNumber;
}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
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

    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(amountBytes);

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesBufferOffset += kTrustLineAmountBytesCount;

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
}

TransactionResult::SharedConst AcceptTrustLineTransaction::run()
{
    info() << mStep;
    switch (mStep) {
        case Stages::TrustLineInitialisation: {
            return runInitializationStage();
        }
        case Stages::KeysSharingTargetNextKey: {
            return runReceiveNextKeyStage();
        }
        case Stages::KeysSharingInitialization: {
            return runPublicKeysSharingInitialisationStage();
        }
        case Stages::NextKeyProcessing: {
            return runPublicKeysSendNextKeyStage();
        }
        case Stages::AuditTarget: {
            return runReceiveAuditStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst AcceptTrustLineTransaction::runInitializationStage()
{
    info() << "sender: " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line already present.";
        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);

        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(mContractorUUID)) {
        warning() << "Contractor " << mContractorUUID << " is in black list. Transaction rejected";
        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            false,
            ConfirmationMessage::ContractorBanned);

        return resultDone();
    }

    if (mAmount == 0) {
        warning() << "Can't establish trust line with zero amount.";
        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        // todo : add parameter mSenderIsGateway
        mTrustLines->accept(
            mContractorUUID,
            mAmount,
            ioTransaction);
        populateHistory(ioTransaction, TrustLineRecord::Accepting);
        info() << "Incoming trust line from the node " << mContractorUUID
               << " has been successfully initialised with " << mAmount;

        if (mSenderIsGateway) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                mContractorUUID,
                true);
            info() << "Incoming trust line was opened from gateway";
        }

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";

        sendMessage<TrustLineConfirmationMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            mIAmGateway,
            ConfirmationMessage::OK);

        mStep = KeysSharingTargetNextKey;
        return resultWaitForMessageTypes(
            {Message::TrustLines_PublicKey},
            kWaitMillisecondsForResponse);

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->trustLines().erase(mContractorUUID);
        warning() << "Attempt to accept incoming trust line from the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst AcceptTrustLineTransaction::runReceiveNextKeyStage()
{
    info() << "runReceiveNextKeyStage";
    if (mContext.empty()) {
        warning() << "No next public key message received. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeyMessage>();
    mCurrentPublicKey = message->publicKey();
    mCurrentKeyNumber = message->number();
    return runPublicKeyReceiverStage();
}

TransactionResult::SharedConst AcceptTrustLineTransaction::runReceiveAuditStage()
{
    info() << "runReceiveAuditStage";
    if (mContext.empty()) {
        warning() << "No audit message received. Transaction will be closed, and wait for message";
        return resultDone();
    }
    mAuditMessage = popNextMessage<AuditMessage>();
    return runAuditTargetStage();
}

TransactionResult::SharedConst AcceptTrustLineTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (mTrustLines->trustLineIsPresent(mContractorUUID)) {
        info() << "Trust line already present.";
        if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Init) {
            warning() << "Invalid TL state: " << mTrustLines->trustLineState(mContractorUUID);
            return resultDone();
        }
    }
    mStep = KeysSharingTargetNextKey;
    return resultWaitForMessageTypes(
        {Message::TrustLines_PublicKey},
        kWaitMillisecondsForResponse);
}

pair<BytesShared, size_t> AcceptTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + kTrustLineAmountBytesCount;

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesCount += sizeof(AuditNumber)
                      + lamport::Signature::signatureSize()
                      + sizeof(KeyNumber);
    }

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

    vector<byte> buffer = trustLineAmountToBytes(mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        dataBytesOffset += kTrustLineAmountBytesCount;

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
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const string AcceptTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[AcceptTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void AcceptTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mContractorUUID,
        mAmount);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}