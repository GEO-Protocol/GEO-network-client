#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
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
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorUUID(message->senderUUID),
    mSenderIncomingIP(message->senderIncomingIP()),
    mContractorAddresses(message->senderAddresses),
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
    info() << "sender: " << mContractorUUID;
    info() << "sender incoming IP " << mSenderIncomingIP;
    for (auto &senderAddress : mContractorAddresses) {
        auto ipv4Address = static_pointer_cast<IPv4WithPortAddress>(senderAddress);
        info() << "contractor address " << ipv4Address->fullAddress();
    }

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (mTrustLinesManager->trustLineIsPresent(mContractorUUID)) {
        if (mTrustLinesManager->isTrustLineEmpty(mContractorUUID)
            and mTrustLinesManager->auditNumber(mContractorUUID) == 0) {
            info() << "Send confirmation on init TL again";
            sendMessageWithTemporaryCaching<TrustLineConfirmationMessage>(
                mContractorUUID,
                Message::TrustLines_Initial,
                kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
                mEquivalent,
                mNodeUUID,
                mTransactionUUID,
                mIAmGateway,
                ConfirmationMessage::OK);
            return resultDone();
        }
        if (mTrustLinesManager->trustLineState(mContractorUUID) != TrustLine::Archived) {
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

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(mContractorUUID)) {
        warning() << "Contractor " << mContractorUUID << " is in black list. Transaction rejected";
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::ContractorBanned);
    }

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        if (mTrustLinesManager->trustLineIsPresent(mContractorUUID)) {
            mTrustLinesManager->setTrustLineState(
                mContractorUUID,
                TrustLine::Active,
                ioTransaction);
            info() << "TrustLine form the node " << mContractorUUID
                   << " successfully reinitialised.";
        } else {
            auto contractorID = mContractorsManager->getContractorID(
                ioTransaction,
                mSenderIncomingIP,
                mContractorUUID);
            // todo : add parameter mSenderIsGateway
            mTrustLinesManager->accept(
                contractorID,
                mContractorUUID,
                ioTransaction);
            mTrustLinesManager->setTrustLineState(
                mContractorUUID,
                TrustLine::Active,
                ioTransaction);
            info() << "Trust Line from the node " << mContractorUUID
                   << " has been successfully initialised.";
        }

        populateHistory(ioTransaction, TrustLineRecord::Accepting);
        if (mSenderIsGateway) {
            mTrustLinesManager->setContractorAsGateway(
                ioTransaction,
                mContractorUUID,
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
        mTrustLinesManager->trustLines().erase(mContractorUUID);
        warning() << "Attempt to accept incoming trust line from the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Sending confirmation back.
    sendMessageWithTemporaryCaching<TrustLineConfirmationMessage>(
        mContractorUUID,
        Message::TrustLines_Initial,
        kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
        mEquivalent,
        mNodeUUID,
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