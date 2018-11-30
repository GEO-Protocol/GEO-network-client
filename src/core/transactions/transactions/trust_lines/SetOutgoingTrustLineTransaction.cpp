#include "SetOutgoingTrustLineTransaction.h"


SetOutgoingTrustLineTransaction::SetOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept :

    BaseTrustLineTransaction(
        BaseTransaction::SetOutgoingTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        NodeUUID::empty(),
        command->contractorID(),
        contractorsManager,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCommand(command),
    mCountSendingAttempts(0),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorID) + 1;
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
    info() << "runInitializationStage " << mContractorID << " " << mCommand->amount();
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }

    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested id";
        return resultProtocolError();
    }

    try {
        if (mTrustLines->trustLineState(mContractorID) != TrustLine::Active) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorID);
            return resultProtocolError();
        }

    } catch (NotFoundError &e) {
        warning() << "Attempt to change not existing TL";
        return resultProtocolError();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorID)) {
        warning() << "There are no own keys";
        return resultKeysError();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorID)) {
        warning() << "There are no contractor keys";
        return resultKeysError();
    }

    mPreviousOutgoingAmount = mTrustLines->outgoingTrustAmount(mContractorID);
    mPreviousState = mTrustLines->trustLineState(mContractorID);

    // Trust line must be updated in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mOperationResult = mTrustLines->setOutgoing(
            mContractorID,
            mCommand->amount());

        mTrustLines->setTrustLineState(
            mContractorID,
            TrustLine::AuditPending);

        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();

        switch (mOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                info() << "Outgoing trust line to the node " << mContractorID
                       << " successfully initialised with " << mCommand->amount();
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Updated: {
                info() << "Outgoing trust line to the node " << mContractorID
                       << " successfully set to " << mCommand->amount();
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Closed: {
                info() << "Outgoing trust line to the node " << mContractorID
                       << " successfully closed.";
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::NoChanges: {
                // Trust line was set to the same value as previously.
                // By the first look, new history record is redundant here,
                // but this transaction might be launched only by the user,
                // so, in case if new amount is the same - then user knows it,
                // and new history record must be written too.
                info() << "Outgoing trust line to the node " << mContractorID
                       << " successfully set to " << mCommand->amount();
                break;
            }
        }
    } catch (ValueError &) {
        warning() << "Attempt to set outgoing trust line to the node " << mContractorID << " failed. "
                  << "Cannot open TL with zero amount.";
        return resultProtocolError();

    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto serializedAuditData = getOwnSerializedAuditData();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        keyChain.saveOwnAuditPart(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            mTrustLines->incomingTrustAmount(
                mContractorID),
            mTrustLines->outgoingTrustAmount(
                mContractorID),
            mTrustLines->balance(
                mContractorID));

        mTrustLines->setTrustLineAuditNumber(
            mContractorID,
            mAuditNumber);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceInitializationStage(
            BaseTransaction::SetOutgoingTrustLineTransaction);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceInitializationStage(
            BaseTransaction::SetOutgoingTrustLineTransaction);
#endif

    } catch(IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setOutgoing(
            mContractorID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorID,
            mPreviousState);
        warning() << "Attempt to set outgoing trust line to the node " << mContractorID << " failed. "
                  << "Can't sign audit data. IO transaction can't be completed. "
                  << "Details are: " << e.what();

        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<AuditMessage>(
        mContractorID,
        mEquivalent,
        mNodeUUID,
        mContractorsManager->idOnContractorSide(mContractorID),
        mTransactionUUID,
        mContractorID,
        mAuditNumber,
        mTrustLines->incomingTrustAmount(mContractorID),
        mTrustLines->outgoingTrustAmount(mContractorID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    mCountSendingAttempts++;
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runResponseProcessingStage()
{
    info() << "runResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<AuditMessage>(
                mContractorID,
                mEquivalent,
                mNodeUUID,
                mContractorsManager->idOnContractorSide(mContractorID),
                mTransactionUUID,
                mContractorID,
                mAuditNumber,
                mTrustLines->incomingTrustAmount(mContractorID),
                mTrustLines->outgoingTrustAmount(mContractorID),
                mOwnSignatureAndKeyNumber.second,
                mOwnSignatureAndKeyNumber.first);
            mCountSendingAttempts++;
            info() << "Send message " << mCountSendingAttempts << " times";
            return resultWaitForMessageTypes(
                {Message::TrustLines_AuditConfirmation},
                kWaitMillisecondsForResponse);
        }
        info() << "Transaction will be closed and send ping";
        sendMessage<PingMessage>(
            mContractorID,
            0,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(
                mContractorID));
        return resultDone();
    }

    auto message = popNextMessage<AuditResponseMessage>();
    info() << "contractor " << message->idOnReceiverSide << " confirmed modifying TL.";
    if (message->idOnReceiverSide != mContractorID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorID)) {
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

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));
    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {

        // todo process ConfirmationMessage::OwnKeysAbsent and ConfirmationMessage::ContractorKeysAbsent

        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept changing TL. Response code: " << message->state();
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceProcessingResponseStage(
            BaseTransaction::SetOutgoingTrustLineTransaction);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceProcessingResponseStage(
            BaseTransaction::SetOutgoingTrustLineTransaction);
#endif

        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                message->signature(),
                message->keyNumber())) {
            warning() << "Contractor didn't sign message correct by key number " << message->keyNumber();
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolver TA
            return resultDone();
        }

        keyChain.saveContractorAuditPart(
            ioTransaction,
            mAuditNumber,
            message->keyNumber(),
            message->signature());

        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorID);
        if (mTrustLines->isTrustLineEmpty(mContractorID) and
                mAuditNumber > TrustLine::kInitialAuditNumber + 1) {
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::Archived,
                ioTransaction);
            keyChain.removeUnusedOwnKeys(ioTransaction);
            mTrustLines->setIsOwnKeysPresent(mContractorID, false);
            keyChain.removeUnusedContractorKeys(ioTransaction);
            mTrustLines->setIsContractorKeysPresent(mContractorID, false);
            info() << "Trust Line become empty";
        } else {
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::Active);
            info() << "All data saved. Now TL is ready for using";
        }

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
    } catch (ValueError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorID,
            TrustLine::ConflictResolving,
            ioTransaction);
        // todo need correct reaction
        error() << "Attempt to save audit from contractor " << mContractorID << " failed. "
                << "Details are: " << e.what();
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorID,
            TrustLine::ConflictResolving);
        // todo need correct reaction
        error() << "Attempt to process confirmation from contractor " << mContractorID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    trustLineActionSignal(
        mContractorID,
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

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultKeysError()
{
    return transactionResultFromCommand(
        mCommand->responseThereAreNoKeys());
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
