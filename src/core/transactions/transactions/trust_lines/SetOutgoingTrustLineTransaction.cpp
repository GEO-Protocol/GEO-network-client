#include "SetOutgoingTrustLineTransaction.h"


SetOutgoingTrustLineTransaction::SetOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    VisualInterface *visualInterface,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept :

    BaseTrustLineTransaction(
        BaseTransaction::SetOutgoingTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        command->contractorUUID(),
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCommand(command),
    mAmount(mCommand->amount()),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mVisualInterface(visualInterface)
{
    mPreviousState = TrustLine::Active;
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

SetOutgoingTrustLineTransaction::SetOutgoingTrustLineTransaction(
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
    bytesBufferOffset += kTrustLineAmountBytesCount;

    auto *trustLineState = new (buffer.get()) TrustLine::SerializedTrustLineState;
    mPreviousState = (TrustLine::TrustLineState) *trustLineState;
    bytesBufferOffset += sizeof(TrustLine::SerializedTrustLineState);

    vector<byte> previousAmountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mPreviousOutgoingAmount = bytesToTrustLineAmount(previousAmountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    memcpy(
        &mAuditNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(AuditNumber));

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesBufferOffset += sizeof(AuditNumber);

        // todo signature can be taken from storage on recovery stage
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

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::run()
{
    switch (mStep) {
        case Stages::TrustLineInitialization: {
            return runInitializationStage();
        }
        case Stages::TrustLineResponseProcessing: {
            return runResponseProcessingStage();
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
                    "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runInitializationStage()
{
    info() << "runInitializationStage " << mContractorUUID << " " << mAmount;
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    try {
        if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active and
                mTrustLines->trustLineState(mContractorUUID) != TrustLine::Archived) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
            return resultProtocolError();
        }
        if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Archived and
                mAmount == TrustLine::kZeroAmount()) {
            warning() << "Try close Archived TL";
            return resultProtocolError();
        }
    } catch (NotFoundError &e) {
        warning() << "Attempt to change not existing TL";
        return resultProtocolError();
    }

    mPreviousOutgoingAmount = mTrustLines->outgoingTrustAmount(mContractorUUID);
    mPreviousState = mTrustLines->trustLineState(mContractorUUID);

    // Trust line must be updated in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mOperationResult = mTrustLines->setOutgoing(
            mContractorUUID,
            mAmount);

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Modify,
            ioTransaction);

        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();

        switch (mOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                info() << "Outgoing trust line to the node " << mContractorUUID
                       << " successfully initialised with " << mAmount;
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Updated: {
                info() << "Outgoing trust line to the node " << mContractorUUID
                       << " successfully set to " << mAmount;
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Closed: {
                info() << "Outgoing trust line to the node " << mContractorUUID
                       << " successfully closed.";
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::NoChanges: {
                // Trust line was set to the same value as previously.
                // By the first look, new history record is redundant here,
                // but this transaction might be launched only by the user,
                // so, in case if new amount is the same - then user knows it,
                // and new history record must be written too.
                info() << "Outgoing trust line to the node " << mContractorUUID
                       << " successfully set to " << mAmount;
                break;
            }
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

        if (mSubsystemsController->isWriteVisualResults()) {
            if (mOperationResult == TrustLinesManager::TrustLineOperationResult::Opened) {
                stringstream s;
                s << VisualResult::OutgoingTrustLineOpen << kTokensSeparator
                  << microsecondsSinceUnixEpoch() << kTokensSeparator
                  << currentTransactionUUID() << kTokensSeparator
                  << mContractorUUID << kCommandsSeparator;
                auto message = s.str();

                try {
                    mVisualInterface->writeResult(
                        message.c_str(),
                        message.size());
                } catch (IOError &e) {
                    error() << "SetOutgoingTrustLineTransaction: "
                            "Error occurred when visual result has accepted. Details: " << e.message();
                }
            }
            if (mOperationResult == TrustLinesManager::TrustLineOperationResult::Closed) {
                stringstream s;
                s << VisualResult::OutgoingTrustLineClose << kTokensSeparator
                  << microsecondsSinceUnixEpoch() << kTokensSeparator
                  << currentTransactionUUID() << kTokensSeparator
                  << mContractorUUID << kCommandsSeparator;
                auto message = s.str();

                try {
                    mVisualInterface->writeResult(
                        message.c_str(),
                        message.size());
                } catch (IOError &e) {
                    error() << "SetOutgoingTrustLineTransaction: "
                            "Error occurred when visual result has accepted. Details: " << e.message();
                }
            }
        }

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";

    } catch (ValueError &) {
        warning() << "Attempt to set outgoing trust line to the node " << mContractorUUID << " failed. "
                  << "Cannot open TL with zero amount.";
        return resultProtocolError();

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setOutgoing(
            mContractorUUID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        warning() << "Attempt to set outgoing trust line to the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<SetIncomingTrustLineMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mAmount);

    mStep = TrustLineResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runResponseProcessingStage()
{
    info() << "runResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "Contractor don't send response. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " confirmed opening TL. gateway: " << message->isContractorGateway();
    if (message->senderUUID != mContractorUUID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept changing TL. Response code: " << message->state();
            mTrustLines->setOutgoing(
                mContractorUUID,
                mAmount);
            mTrustLines->setTrustLineState(
                mContractorUUID,
                mPreviousState,
                ioTransaction);
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLProcessingResponseStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLProcessingResponseStage();
#endif

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending,
            ioTransaction);
        switch (mOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                populateHistory(ioTransaction, TrustLineRecord::Opening);
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Updated: {
                populateHistory(ioTransaction, TrustLineRecord::Setting);
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Closed: {
                populateHistory(ioTransaction, TrustLineRecord::Closing);
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::NoChanges: {
                populateHistory(ioTransaction, TrustLineRecord::Setting);
                break;
            }
        }
        processConfirmationMessage(message);
    } catch (IOError &e) {
        ioTransaction->rollback();
        // todo : need return intermediate state of TL
        error() << "Attempt to process confirmation from contractor " << mContractorUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    mStep = AuditInitialization;
    return resultAwakeAsFastAsPossible();
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent " << mContractorUUID;
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Modify) {
        mOperationResult = mTrustLines->setOutgoing(
            mContractorUUID,
            mAmount);
        mStep = TrustLineResponseProcessing;
        return runResponseProcessingStage();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        mOperationResult = mTrustLines->setOutgoing(
            mContractorUUID,
            mAmount);
        mStep = AuditResponseProcessing;
        return runAuditResponseProcessingStage();
    }

    warning() << "Invalid TL state for this TA: "
              << mTrustLines->trustLineState(mContractorUUID);
    return resultDone();
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

pair<BytesShared, size_t> SetOutgoingTrustLineTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + kTrustLineAmountBytesCount
                        + sizeof(TrustLine::SerializedTrustLineState)
                        + kTrustLineAmountBytesCount
                        + sizeof(AuditNumber);
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
        bytesCount += lamport::Signature::signatureSize()
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
    dataBytesOffset += kTrustLineAmountBytesCount;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mPreviousState,
        sizeof(TrustLine::SerializedTrustLineState));
    dataBytesOffset += sizeof(TrustLine::SerializedTrustLineState);

    buffer = trustLineAmountToBytes(mPreviousOutgoingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
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

const string SetOutgoingTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[SetOutgoingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void SetOutgoingTrustLineTransaction::populateHistory(
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
