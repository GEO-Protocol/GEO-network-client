#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    TrustLineInitialMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    bool iAmGateway,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::AcceptTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorUUID(message->senderUUID),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(message->isContractorGateway())
{}

TransactionResult::SharedConst AcceptTrustLineTransaction::run()
{
    info() << "sender: " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (mTrustLinesManager->trustLineIsPresent(mContractorUUID)) {
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

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        // todo : add parameter mSenderIsGateway
        mTrustLinesManager->accept(
            mContractorUUID,
            0,
            ioTransaction);
        populateHistory(ioTransaction, TrustLineRecord::Accepting);
        info() << "Trust Line from the node " << mContractorUUID
               << " has been successfully initialised.";

        if (mSenderIsGateway) {
            mTrustLinesManager->setContractorAsGateway(
                ioTransaction,
                mContractorUUID,
                true);
            info() << "Incoming trust line was opened from gateway";
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLinesManager->trustLines().erase(mContractorUUID);
        warning() << "Attempt to accept incoming trust line from the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    sendMessageWithCaching<TrustLineConfirmationMessage>(
        mContractorUUID,
        Message::TrustLines_Initial,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mIAmGateway,
        ConfirmationMessage::OK);

    publicKeysSharingSignal(
        mContractorUUID,
        mEquivalent);
    return resultDone();
}

TransactionResult::SharedConst AcceptTrustLineTransaction::sendTrustLineErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<TrustLineConfirmationMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        false,
        errorState);
    return resultDone();
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
        0);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}