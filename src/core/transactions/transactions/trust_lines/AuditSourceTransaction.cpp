#include "AuditSourceTransaction.h"

AuditSourceTransaction::AuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::AuditSourceTransactionType,
        nodeUUID,
        equivalent,
        contractorUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCountSendingAttempts(0)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
    mStep = Initialization;
}

AuditSourceTransaction::AuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::AuditSourceTransactionType,
        nodeUUID,
        equivalent,
        contractorUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mCountSendingAttempts(0)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID);
    mStep = NextAttempt;
}

TransactionResult::SharedConst AuditSourceTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::NextAttempt: {
            return runNextAttemptStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst AuditSourceTransaction::runInitializationStage()
{
    info() << "runInitializationStage " << mContractorUUID;
    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    try {
        if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
            return resultDone();
        }

    } catch (NotFoundError &e) {
        warning() << "Attempt to audit not existing TL";
        return resultDone();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorUUID)) {
        warning() << "There are no own keys";
        return resultDone();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorUUID)) {
        warning() << "There are no contractor keys";
        return resultDone();
    }

    mTrustLines->setTrustLineState(
        mContractorUUID,
        TrustLine::AuditPending);

    // note: io transaction would commit automatically on destructor call.
    // there is no need to call commit manually.
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto serializedAuditData = getOwnSerializedAuditData();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
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
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(
                mContractorUUID));

        mTrustLines->setTrustLineAuditNumber(
            mContractorUUID,
            mAuditNumber);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceInitializationStage(
            BaseTransaction::AuditSourceTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnTargetStage(
            BaseTransaction::AuditSourceTransactionType);
#endif

    } catch(IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
        warning() << "Attempt to audit trust line to the node " << mContractorUUID << " failed. "
                  << "Can't sign audit data. IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Notifying remote node about trust line state changed.
    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mTrustLines->incomingTrustAmount(mContractorUUID),
        mTrustLines->outgoingTrustAmount(mContractorUUID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    mCountSendingAttempts++;
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst AuditSourceTransaction::runNextAttemptStage()
{
    info() << "runNextAttemptStage " << mContractorUUID;
    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    try {
        if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::AuditPending) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
            return resultDone();
        }

    } catch (NotFoundError &e) {
        warning() << "Attempt to audit not existing TL";
        return resultDone();
    }

    processPongMessage(mContractorUUID);

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorUUID)) {
        warning() << "There are no own keys";
        return resultDone();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorUUID)) {
        warning() << "There are no contractor keys";
        return resultDone();
    }

    // note: io transaction would commit automatically on destructor call.
    // there is no need to call commit manually.
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(mContractorUUID));
    try {
        mOwnSignatureAndKeyNumber = keyChain.getSignatureAndKeyNumberForPendingAudit(
            ioTransaction,
            mAuditNumber);
        debug() << "signature getting";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceResumingStage(
            BaseTransaction::AuditSourceTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceResumingStage(
            BaseTransaction::AuditSourceTransactionType);
#endif

    } catch(IOError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to audit trust line to the node " << mContractorUUID << " failed. "
                  << "Can't get audit data. IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Notifying remote node about trust line state changed.
    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mTrustLines->incomingTrustAmount(mContractorUUID),
        mTrustLines->outgoingTrustAmount(mContractorUUID),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    mCountSendingAttempts++;
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst AuditSourceTransaction::runResponseProcessingStage()
{
    info() << "runResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<AuditMessage>(
                mContractorUUID,
                mEquivalent,
                mNodeUUID,
                mTransactionUUID,
                mContractorUUID,
                mTrustLines->incomingTrustAmount(mContractorUUID),
                mTrustLines->outgoingTrustAmount(mContractorUUID),
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
            mContractorUUID,
            0,
            mNodeUUID);
        return resultDone();
    }

    auto message = popNextMessage<AuditResponseMessage>();
    info() << "contractor " << message->senderUUID << " confirmed audit.";
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

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {

        // todo process ConfirmationMessage::OwnKeysAbsent and ConfirmationMessage::ContractorKeysAbsent

        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept changing TL. Response code: " << message->state();
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::ConflictResolving,
                ioTransaction);
            // todo run conflict resolving TA
            return resultDone();
        }

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
        info() << "Contractor sign audit correct";

        keyChain.saveContractorAuditPart(
            ioTransaction,
            mAuditNumber,
            message->keyNumber(),
            message->signature());

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);
        info() << "All data saved. Now TL is ready for using";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceProcessingResponseStage(
            BaseTransaction::AuditSourceTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceProcessingResponseStage(
            BaseTransaction::AuditSourceTransactionType);
#endif

    } catch (ValueError &e) {
        ioTransaction->rollback();
        // todo need correct reaction, maybe conflict resolver
        error() << "Attempt to save audit from contractor " << mContractorUUID << " failed. "
                << "Details are: " << e.what();
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        // todo need correct reaction, maybe conflict resolver
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

const string AuditSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}