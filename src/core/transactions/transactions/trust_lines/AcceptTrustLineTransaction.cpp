#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    TrustLineInitialMessage::Shared message,
    ContractorsManager *contractorsManager,
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
        message->equivalent(),
        logger),
    mContractorID(message->idOnReceiverSide),
    mSenderIncomingIP(message->senderIncomingIP()),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(message->isContractorGateway())
{}

TransactionResult::SharedConst AcceptTrustLineTransaction::run()
{
    info() << "sender: " << mContractorID << " sender incoming IP " << mSenderIncomingIP;
    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested ID";
        return resultDone();
    }
    if (!mContractorsManager->channelConfirmed(mContractorID)) {
        warning() << "Channel is not confirmed";
        return resultDone();
    }
    info() << "Contractor main address: " << mContractorsManager->contractorMainAddress(mContractorID)->fullAddress();

    if (mTrustLinesManager->trustLineIsPresent(mContractorID)) {
        if (mTrustLinesManager->isTrustLineEmpty(mContractorID)
            and mTrustLinesManager->auditNumber(mContractorID) == 0) {
            info() << "Send confirmation on init TL again";
            sendMessageWithTemporaryCaching<TrustLineConfirmationMessage>(
                mContractorID,
                Message::TrustLines_Initial,
                kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
                mEquivalent,
                mContractorsManager->idOnContractorSide(mContractorID),
                mTransactionUUID,
                mIAmGateway,
                ConfirmationMessage::OK);
            return resultDone();
        }
        if (mTrustLinesManager->trustLineState(mContractorID) != TrustLine::Archived) {
            warning() << "Trust line already present and not initial.";
            return sendTrustLineErrorConfirmation(
                ConfirmationMessage::TrustLineAlreadyPresent);
        } else {
            info() << "Reopening of archived TL";
        }
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        if (mTrustLinesManager->trustLineIsPresent(mContractorID)) {
            mTrustLinesManager->setTrustLineState(
                mContractorID,
                TrustLine::Active,
                ioTransaction);
            info() << "TrustLine form the node " << mContractorID
                   << " successfully reinitialised.";
        } else {
            // todo : add parameter mSenderIsGateway
            mTrustLinesManager->accept(
                mContractorID,
                ioTransaction);
            mTrustLinesManager->setTrustLineState(
                mContractorID,
                TrustLine::Active,
                ioTransaction);
            info() << "Trust Line from the node " << mContractorID
                   << " has been successfully initialised.";
        }

        populateHistory(ioTransaction, TrustLineRecord::Accepting);
        if (mSenderIsGateway) {
            mTrustLinesManager->setContractorAsGateway(
                ioTransaction,
                mContractorID,
                true);
            info() << "Incoming trust line was opened from gateway";
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTargetStage(
            BaseTransaction::AcceptTrustLineTransaction);
        mTrustLinesInfluenceController->testTerminateProcessOnTargetStage(
            BaseTransaction::AcceptTrustLineTransaction);
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLinesManager->removeTrustLine(
            mContractorID);
        warning() << "Attempt to accept incoming trust line from the node " << mContractorID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Sending confirmation back.
    sendMessageWithTemporaryCaching<TrustLineConfirmationMessage>(
        mContractorID,
        Message::TrustLines_Initial,
        kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
        mEquivalent,
        mContractorsManager->idOnContractorSide(mContractorID),
        mTransactionUUID,
        mIAmGateway,
        ConfirmationMessage::OK);
    info() << "Confirmation was sent";
    return resultDone();
}

TransactionResult::SharedConst AcceptTrustLineTransaction::sendTrustLineErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<TrustLineConfirmationMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->idOnContractorSide(mContractorID),
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
        mContractorsManager->contractor(mContractorID),
        0);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}