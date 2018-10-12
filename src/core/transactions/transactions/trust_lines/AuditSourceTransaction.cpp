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
        logger)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

TransactionResult::SharedConst AuditSourceTransaction::run()
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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnAuditStage();
        mTrustLinesInfluenceController->testTerminateProcessOnAuditStage();
#endif

    } catch(IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
        warning() << "Attempt to set outgoing trust line to the node " << mContractorUUID << " failed. "
                  << "Can't sign audit data. IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
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
        warning() << "Contractor don't send response. Transaction will be closed.";
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
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
    processConfirmationMessage(message);

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {

        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept changing TL. Response code: " << message->state();
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

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
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