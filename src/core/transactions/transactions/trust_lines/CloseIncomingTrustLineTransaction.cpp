#include "CloseIncomingTrustLineTransaction.h"

CloseIncomingTrustLineTransaction::CloseIncomingTrustLineTransaction(
    CloseIncomingTrustLineCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    FeaturesManager *featuresManager,
    EventsInterfaceManager *eventsInterfaceManager,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):

    BaseTrustLineTransaction(
        BaseTransaction::CloseIncomingTrustLineTransactionType,
        command->equivalent(),
        command->contractorID(),
        contractorsManager,
        manager,
        storageHandler,
        keystore,
        featuresManager,
        trustLinesInfluenceController,
        logger),
    mCommand(command),
    mCountSendingAttempts(0),
    mCountPendingAttempts(0),
    mCountContractorPendingAttempts(0),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mEventsInterfaceManager(eventsInterfaceManager),
    mSubsystemsController(subsystemsController)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorID) + 1;
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::Pending: {
            return runAuditPendingStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        case Stages::ContractorPending: {
            return runContractorPendingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runInitializationStage()
{
    info() << "runInitializationStage " << mContractorID;
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

    mTrustLines->setTrustLineState(
        mContractorID,
        TrustLine::AuditPending);

    if (mTrustLines->isReservationsPresentOnTrustLine(mContractorID)) {
        warning() << "There are some reservations on TL. Audit will be suspended";
        mStep = Pending;
        mCountPendingAttempts++;
        return resultAwakeAfterMilliseconds(
            kPendingPeriodInMilliseconds);
    }

    return initializeAudit();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runAuditPendingStage()
{
    info() << "runAuditPendingStage with " << mContractorID << " attempt " << mCountPendingAttempts;
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }

    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested id";
        return resultProtocolError();
    }

    try {
        if (mTrustLines->trustLineState(mContractorID) != TrustLine::AuditPending) {
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

    if (mTrustLines->isReservationsPresentOnTrustLine(mContractorID)) {
        warning() << "There are some reservations on TL. Audit will be suspended";
        mCountPendingAttempts++;
        if (mCountPendingAttempts > kMaxPendingAttempts) {
            warning() << "Max pending attempts. TL will be conflicted";
            auto ioTransaction = mStorageHandler->beginTransaction();
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultProtocolError();
        }

        return resultAwakeAfterMilliseconds(
            kPendingPeriodInMilliseconds);
    }

    return initializeAudit();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";

        // check if audit was cancelled
        auto ioTransaction = mStorageHandler->beginTransaction();
        auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(mContractorID));
        try {
            if (keyChain.isAuditWasCancelled(ioTransaction, mAuditNumber)) {
                info() << "Audit was cancelled by other audit transaction";
                return resultDone();
            }
        } catch (IOError &e) {
            error() << "Attempt to check if audit was cancelled failed. "
                    << "IO transaction can't be completed. Details are: " << e.what();
            throw e;
        }

        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<AuditMessage>(
                mContractorID,
                mEquivalent,
                mContractorsManager->contractor(mContractorID),
                mTransactionUUID,
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
        sendMessage<PingMessage>(
            mContractorID,
            mContractorsManager->idOnContractorSide(mContractorID));
        info() << "Transaction will be closed and send ping";
        return resultDone();
    }
    auto message = popNextMessage<AuditResponseMessage>();
    info() << "contractor " << message->idOnReceiverSide << " confirmed closing incoming TL.";
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

        if (mCountContractorPendingAttempts > kMaxPendingAttempts) {
            warning() << "Max contractor pending attempts. TL will be conflicted";
            auto ioTransaction = mStorageHandler->beginTransaction();
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultDone();
        }
        mStep = ContractorPending;
        mCountContractorPendingAttempts++;
        mCountSendingAttempts = 0;
        return resultAwakeAfterMilliseconds(
            kPendingPeriodInMilliseconds);
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));
    try {

        // todo process ConfirmationMessage::OwnKeysAbsent and ConfirmationMessage::ContractorKeysAbsent

        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept closing incoming TL. Response code: " << message->state();
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceProcessingResponseStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceProcessingResponseStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
#endif

        auto contractorSerializedAuditData = getContractorSerializedAuditData(
            keyChain.ownPublicKeysHash(ioTransaction),
            keyChain.contractorPublicKeysHash(ioTransaction));
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
        if (mTrustLines->isTrustLineEmpty(mContractorID)) {
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::Archived,
                ioTransaction);
            keyChain.removeUnusedOwnKeys(ioTransaction);
            mTrustLines->setIsOwnKeysPresent(mContractorID, false);
            keyChain.removeUnusedContractorKeys(ioTransaction);
            mTrustLines->setIsContractorKeysPresent(mContractorID, false);
            info() << "Trust Line become empty";
            try {
                mEventsInterfaceManager->writeEvent(
                    Event::closeTrustLineEvent(
                        mContractorsManager->selfContractor()->mainAddress(),
                        mContractorsManager->contractorMainAddress(mContractorID),
                        mEquivalent));
            } catch (std::exception &e) {
                warning() << "Can't write close TL event " << e.what();
            }
        } else {
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::Active);
            info() << "All data saved. Now TL is ready for using";
        }

        populateHistory(ioTransaction, TrustLineRecord::ClosingIncoming);

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
        return resultDone();
    }

    mTrustLines->resetAuditRule(mContractorID);
    trustLineActionSignal(
        mContractorID,
        mEquivalent,
        false);

    return resultDone();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runContractorPendingStage()
{
    info() << "runContractorPendingStage with " << mContractorID
           << " attempt " << mCountContractorPendingAttempts;
    // check if audit was cancelled
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(mContractorID));
    try {
        if (keyChain.isAuditWasCancelled(ioTransaction, mAuditNumber)) {
            info() << "Audit was cancelled by other audit transaction";
            return resultDone();
        }
    } catch (IOError &e) {
        error() << "Attempt to check if audit was cancelled failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    sendMessage<AuditMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->contractor(mContractorID),
        mTransactionUUID,
        mAuditNumber,
        mTrustLines->incomingTrustAmount(mContractorID),
        mTrustLines->outgoingTrustAmount(mContractorID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send message " << mCountSendingAttempts << " times";
    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::initializeAudit()
{
    info() << "initializeAudit";
    mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorID);
    mPreviousState = mTrustLines->trustLineState(mContractorID);

    // Trust line must be updated in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    try {
        mTrustLines->closeIncoming(
            mContractorID);
    } catch (ValueError& e) {
        warning() << "Attempt to close incoming trust line to the node " << mContractorID << " failed. "
                  << e.what();
        return resultProtocolError();
    }

    // remove this TL from Topology TrustLines Manager
    mTopologyTrustLinesManager->addTrustLine(
        make_shared<TopologyTrustLine>(
            0,
            mContractorID,
            make_shared<const TrustLineAmount>(0)));
    mTopologyCacheManager->resetInitiatorCache();
    mMaxFlowCacheManager->clearCashes();
    info() << "Incoming trust line from the node " << mContractorID
           << " successfully closed.";

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        auto ownPublicKeysHash = keyChain.ownPublicKeysHash(ioTransaction);
        auto contractorPublicKeysHash = keyChain.contractorPublicKeysHash(ioTransaction);
        auto serializedAuditData = getOwnSerializedAuditData(
            ownPublicKeysHash,
            contractorPublicKeysHash);
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        keyChain.saveOwnAuditPart(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            ownPublicKeysHash,
            contractorPublicKeysHash,
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
            BaseTransaction::CloseIncomingTrustLineTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceInitializationStage(
            BaseTransaction::CloseIncomingTrustLineTransactionType);
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->setIncoming(
            mContractorID,
            mPreviousIncomingAmount);
        mTrustLines->setTrustLineState(
            mContractorID,
            mPreviousState);
        warning() << "Attempt to close incoming TL from the node " << mContractorID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<AuditMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->contractor(mContractorID),
        mTransactionUUID,
        mAuditNumber,
        mTrustLines->incomingTrustAmount(mContractorID),
        mTrustLines->outgoingTrustAmount(mContractorID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    mCountSendingAttempts++;

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultKeysError()
{
    return transactionResultFromCommand(
        mCommand->responseThereAreNoKeys());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string CloseIncomingTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[CloseIncomingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void CloseIncomingTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mContractorsManager->contractor(mContractorID));

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
