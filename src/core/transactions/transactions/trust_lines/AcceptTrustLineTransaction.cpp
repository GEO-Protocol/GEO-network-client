#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineInitialMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    bool iAmGateway,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::AcceptTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(message->isContractorGateway())
{
    mAmount = message->amount();
    mAuditNumber = TrustLine::kInitialAuditNumber;
    mContractorKeysCount = 0;
    mCurrentKeyNumber = 0;
}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
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

    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(amountBytes);

    mAuditNumber = TrustLine::kInitialAuditNumber;

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Init or
            mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesBufferOffset += kTrustLineAmountBytesCount;

        auto *contractorKeysCount = new (buffer.get() + bytesBufferOffset) KeysCount;
        mContractorKeysCount = (KeysCount) *contractorKeysCount;
        bytesBufferOffset += sizeof(KeysCount);

        auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
        mCurrentKeyNumber = (KeyNumber) *keyNumber;
    }
}

TransactionResult::SharedConst AcceptTrustLineTransaction::run()
{
    info() << mStep;
    switch (mStep) {
        case Stages::TrustLineInitialization: {
            return runInitializationStage();
        }
        case Stages::KeysSharingTargetInitialization: {
            return runReceiveFirstKeyStage();
        }
        case Stages::KeysSharingTargetNextKey: {
            return runReceiveNextKeyStage();
        }
        case Stages::KeysSharingInitialization: {
            return runPublicKeysSharingInitializationStage();
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
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(mContractorUUID)) {
        warning() << "Contractor " << mContractorUUID << " is in black list. Transaction rejected";
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::ContractorBanned);
    }

    if (mAmount == 0) {
        warning() << "Can't establish trust line with zero amount.";
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

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

    sendMessageWithCaching<TrustLineConfirmationMessage>(
        mContractorUUID,
        Message::TrustLines_SetIncomingInitial,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mIAmGateway,
        ConfirmationMessage::OK);

    mStep = KeysSharingTargetInitialization;
    return resultWaitForMessageTypes(
        {Message::TrustLines_PublicKeysSharingInit},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst AcceptTrustLineTransaction::runReceiveFirstKeyStage()
{
    info() << "runReceiveFirstKeyStage";
    if (mContext.empty()) {
        warning() << "No next public key init message received. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeysSharingInitMessage>();
    mContractorKeysCount = message->keysCount();
    mCurrentPublicKey = message->publicKey();
    mCurrentKeyNumber = message->number();
    mShouldPopPublicKeyMessage = false;
    return runPublicKeyReceiverInitStage();
}

TransactionResult::SharedConst AcceptTrustLineTransaction::runReceiveNextKeyStage()
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
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        info() << "Trust line is absent.";
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Init) {
        info() << "Init stage";
        mTrustLines->setIncoming(
            mContractorUUID,
            mAmount);
        mStep = KeysSharingTargetInitialization;
        return runReceiveFirstKeyStage();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        info() << "Keys pending stage";
        auto keyChain = mKeysStore->keychain(mTrustLines->trustLineID(mContractorUUID));
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            if (!keyChain.ownKeysPresent(ioTransaction)) {
                mStep = KeysSharingTargetNextKey;
            } else {
                mCurrentPublicKey = keyChain.publicKey(
                    ioTransaction,
                    mCurrentKeyNumber);
                if (mCurrentPublicKey == nullptr) {
                    warning() << "Can't get own public key with number " << mCurrentKeyNumber;
                    return resultDone();
                }
                mStep = NextKeyProcessing;
            }
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't get own public key from storage. Details: " << e.what();
            return resultDone();
        }
        mTrustLines->setIncoming(
            mContractorUUID,
            mAmount);
        return resultAwakeAsFastAsPossible();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        info() << "Audit pending stage";
        mTrustLines->setIncoming(
            mContractorUUID,
            mAmount);
        mStep = AuditTarget;
        return runReceiveAuditStage();
    }
    warning() << "Invalid TL state: " << mTrustLines->trustLineState(mContractorUUID);
    return resultDone();
}

pair<BytesShared, size_t> AcceptTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + kTrustLineAmountBytesCount;

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Init or
            mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesCount += sizeof(KeysCount) + sizeof(KeyNumber);
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

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Init or
            mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        dataBytesOffset += kTrustLineAmountBytesCount;

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mContractorKeysCount,
            sizeof(KeysCount));
        dataBytesOffset += sizeof(KeysCount);

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mCurrentKeyNumber,
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