#include "RejectOutgoingTrustLineTransaction.h"

RejectOutgoingTrustLineTransaction::RejectOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    ConfirmationMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::RejectOutgoingTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst RejectOutgoingTrustLineTransaction::run()
{
    const auto kContractor = mMessage->senderUUID;

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLinesManager->closeOutgoing(
            ioTransaction,
            kContractor);
        auto record = make_shared<TrustLineRecord>(
            currentTransactionUUID(),
            TrustLineRecord::RejectingOutgoing,
            kContractor);

        ioTransaction->historyStorage()->saveTrustLineRecord(record);

        processConfirmationMessage(
            kContractor,
            mMessage);
        info() << "Outgoing TL to the node " << kContractor << " successfully rejected";
        return resultDone();
    } catch (NotFoundError) {
        warning() << "Can't reject outgoing TL because absence";
        processConfirmationMessage(
            kContractor,
            mMessage);
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        info() << "Attempt to close outgoing trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

const string RejectOutgoingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[RejectOutgoingTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}