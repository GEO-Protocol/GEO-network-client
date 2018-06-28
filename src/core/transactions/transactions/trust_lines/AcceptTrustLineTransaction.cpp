#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::AcceptTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(false)
{}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineFromGatewayMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::AcceptTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(true)
{}

TransactionResult::SharedConst AcceptTrustLineTransaction::run()
{
    const auto kContractor = mMessage->senderUUID;
    info() << "sender: " << kContractor << " equivalent: " << mMessage->equivalent();

    if (kContractor == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (mTrustLines->trustLineIsPresent(kContractor)) {
        warning() << "Trust line already present.";
        sendMessage<TrustLineConfirmationMessage>(
            kContractor,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);

        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(kContractor)) {
        warning() << "Contractor " << kContractor << " is in black list. Transaction rejected";
        sendMessage<TrustLineConfirmationMessage>(
            kContractor,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ContractorBanned);

        return resultDone();
    }

    if (mMessage->amount() == 0) {
        warning() << "Can't establish trust line with zero amount.";
        sendMessage<TrustLineConfirmationMessage>(
            kContractor,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        // todo : add parameter mSenderIsGateway
        mTrustLines->accept(
            ioTransaction,
            kContractor,
            mMessage->amount());
        populateHistory(ioTransaction, TrustLineRecord::Accepting);
        info() << "Incoming trust line from the node " << kContractor
               << " has been successfully initialised with " << mMessage->amount();

        if (mSenderIsGateway) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                mMessage->senderUUID,
                true);
            info() << "Incoming trust line was opened from gateway";
        }

        sendMessage<TrustLineConfirmationMessage>(
            kContractor,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            mIAmGateway,
            ConfirmationMessage::OK);

        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->trustLines().erase(kContractor);
        warning() << "Attempt to accept incoming trust line from the node " << kContractor << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
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
        mMessage->senderUUID,
        mMessage->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}