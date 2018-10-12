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
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mVisualInterface(visualInterface)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runInitializationStage()
{
    info() << "runInitializationStage " << mContractorUUID << " " << mCommand->amount();
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    try {
        if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
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
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mOperationResult = mTrustLines->setOutgoing(
            mContractorUUID,
            mCommand->amount());

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending);

        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();

        switch (mOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                info() << "Outgoing trust line to the node " << mContractorUUID
                       << " successfully initialised with " << mCommand->amount();
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Updated: {
                info() << "Outgoing trust line to the node " << mContractorUUID
                       << " successfully set to " << mCommand->amount();
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
                       << " successfully set to " << mCommand->amount();
                break;
            }
        }

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

    } catch (ValueError &) {
        warning() << "Attempt to set outgoing trust line to the node " << mContractorUUID << " failed. "
                  << "Cannot open TL with zero amount.";
        return resultProtocolError();

    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto serializedAuditData = getOwnSerializedAuditData();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnAuditStage();
        mTrustLinesInfluenceController->testTerminateProcessOnAuditStage();
#endif

    } catch(IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setOutgoing(
            mContractorUUID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        warning() << "Attempt to set outgoing trust line to the node " << mContractorUUID << " failed. "
                  << "Can't sign audit data. IO transaction can't be completed. "
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
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first,
        mCommand->amount());

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runResponseProcessingStage()
{
    info() << "runResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "Contractor don't send response. Transaction will be closed.";
        mTrustLines->setOutgoing(
            mContractorUUID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        return resultDone();
    }

    auto message = popNextMessage<AuditResponseMessage>();
    info() << "contractor " << message->senderUUID << " confirmed modifying TL.";
    if (message->senderUUID != mContractorUUID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }

    if (message->state() == ConfirmationMessage::ReservationsPresentOnTrustLine) {
        info() << "Contractor's TL is not ready for audit yet";
        // message on communicator queue, wait for audit response after reservations committing or cancelling
        // todo add timeout or count failed attempts for running conflict resolver TA
        return resultWaitForMessageTypes(
            {Message::TrustLines_AuditConfirmation},
            kWaitMillisecondsForResponse);
    }
    processConfirmationMessage(message);

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {

        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept changing TL. Response code: " << message->state();
            mTrustLines->setOutgoing(
                mContractorUUID,
                mPreviousOutgoingAmount);
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLProcessingResponseStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLProcessingResponseStage();
#endif

        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                message->signature(),
                message->keyNumber())) {
            warning() << "Contractor didn't sign message correct";
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolver TA
            return resultDone();
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

        mTrustLines->setTrustLineAuditNumberAndMakeActive(
            mContractorUUID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);

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
    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setOutgoing(
            mContractorUUID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        error() << "Attempt to process confirmation from contractor " << mContractorUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    trustLineActionSignal(
        mContractorUUID,
        mEquivalent,
        false);

    return resultDone();
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_AuditConfirmation},
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
        mCommand->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
