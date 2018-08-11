#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    bool iAmGateway,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept :

    BaseTrustLineTransaction(
        BaseTransaction::OpenTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        command->contractorUUID(),
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCommand(command),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway)
{
    mAmount = command->amount();
    mAuditNumber = TrustLine::kInitialAuditNumber;
    mCurrentKeyNumber = 0;
}

OpenTrustLineTransaction::OpenTrustLineTransaction(
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

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesBufferOffset += kTrustLineAmountBytesCount;

        auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
        mCurrentKeyNumber = (KeyNumber) *keyNumber;
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesBufferOffset += kTrustLineAmountBytesCount;

        // todo signature can be taken from storage
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

TransactionResult::SharedConst OpenTrustLineTransaction::run()
{
    info() << "step " << mStep;
    switch (mStep) {
        case Stages::TrustLineInitialization: {
            return runInitializationStage();
        }
        case Stages::TrustLineResponseProcessing: {
            return runResponseProcessingStage();
        }
        case Stages::KeysSharingInitialization: {
            return runPublicKeysSharingInitializationStage();
        }
        case Stages::NextKeyProcessing: {
            return runPublicKeysSendNextKeyStage();
        }
        case Stages::KeysSharingTargetNextKey: {
            return runReceiveNextKeyStage();
        }
        case Stages::AuditInitialization: {
            return runAuditInitializationStage();
        }
        case Stages::AuditResponseProcessing: {
            return runAuditResponseProcessingStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep");
    }
}

TransactionResult::SharedConst OpenTrustLineTransaction::runInitializationStage()
{
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }
    info() << "Try open TL to " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    if (mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line already present.";
        return resultProtocolError();
    }
    if (mAmount == 0) {
        warning() << "Can't establish trust line with zero amount.";
        return resultProtocolError();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    mTrustLines->open(
        mContractorUUID,
        mAmount,
        ioTransaction);
    info() << "Outgoing trust line to the node " << mContractorUUID
           << " successfully initialised with " << mAmount;

    try {
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
        mTrustLines->removeTrustLine(
            mContractorUUID);
        error() << "Error during saving TA. Details: " << e.what();
        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<SetIncomingTrustLineInitialMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mAmount,
        mIAmGateway);

    mStep = TrustLineResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " send response on opening TL. gateway: " << message->isContractorGateway();
    if (message->senderUUID != mContractorUUID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        error() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept opening TL. Response code: " << message->state();
        mTrustLines->removeTrustLine(
            mContractorUUID,
            ioTransaction);
        // delete this transaction from storage
        ioTransaction->transactionHandler()->deleteRecord(
            currentTransactionUUID());
        processConfirmationMessage(message);
        return resultDone();
    }

#ifdef TESTS
    mTrustLinesInfluenceController->testThrowExceptionOnTLProcessingResponseStage();
    mTrustLinesInfluenceController->testTerminateProcessOnTLProcessingResponseStage();
#endif

    try {
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::KeysPending,
            ioTransaction);
        if (message->isContractorGateway()) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                mContractorUUID,
                true);
        }
        populateHistory(
            ioTransaction,
            TrustLineRecord::Opening);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Attempt to process confirmation from contractor " << mContractorUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    processConfirmationMessage(message);
    mStep = KeysSharingInitialization;
    return resultAwakeAsFastAsPossible();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent";
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Init) {
        info() << "Init state";
        mTrustLines->setOutgoing(
            mContractorUUID,
            mAmount);
        mStep = TrustLineResponseProcessing;
        return runResponseProcessingStage();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        info() << "Keys pending state, current key number " << mCurrentKeyNumber;
        auto keyChain = mKeysStore->keychain(mTrustLines->trustLineID(mContractorUUID));
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            if (keyChain.contractorKeysPresent(ioTransaction)) {
                mStep = KeysSharingTargetNextKey;
            } else if (mCurrentKeyNumber >= TrustLineKeychain::kDefaultKeysSetSize) {
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
        mTrustLines->setOutgoing(
            mContractorUUID,
            mAmount);
        return resultAwakeAsFastAsPossible();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        info() << "Audit state";
        mTrustLines->setOutgoing(
            mContractorUUID,
            mAmount);
        mStep = AuditResponseProcessing;
        return runAuditResponseProcessingStage();
    }
    warning() << "Invalid TL state: " << mTrustLines->trustLineState(mContractorUUID);
    return resultDone();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runReceiveNextKeyStage()
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

TransactionResult::SharedConst OpenTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

pair<BytesShared, size_t> OpenTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + kTrustLineAmountBytesCount;

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        bytesCount += sizeof(KeyNumber);
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesCount += lamport::PublicKey::keySize()
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

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        dataBytesOffset += kTrustLineAmountBytesCount;

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mCurrentKeyNumber,
            sizeof(KeyNumber));
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        dataBytesOffset += kTrustLineAmountBytesCount;

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

const string OpenTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[OpenTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void OpenTrustLineTransaction::populateHistory(
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