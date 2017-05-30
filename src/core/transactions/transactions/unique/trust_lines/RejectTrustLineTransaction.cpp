#include "RejectTrustLineTransaction.h"

RejectTrustLineTransaction::RejectTrustLineTransaction(
    const NodeUUID &nodeUUID,
    RejectTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler) {}

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst RejectTrustLineTransaction::run()
{
    const auto kContractor = mMessage->contractorUUID();

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        return transactionResultFromMessage(mMessage->resultRejected());
    }

    // Trust line must be removed (updated) in the trust lines storage,
    // and history record about the operation must be written to the history storage.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    // -----------------------------------------------------------
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->reject(
            ioTransaction,
            kContractor);
        updateHistory(ioTransaction);

        info() << "Trust line to the node " << kContractor << " closed successfully.";
        return resultDone();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        info() << "Attempt to close trust line to the node " << kContractor << " failed. "
               << "There is no outgoing trust line to this node is present. "
               << "Details are: " << e.what();

        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        info() << "Attempt to close trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

const string RejectTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[RejectTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

void RejectTrustLineTransaction::updateHistory(
    IOTransaction::Shared ioTransaction)
{
    auto record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::Rejecting,
        mMessage->contractorUUID());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}