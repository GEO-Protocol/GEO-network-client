#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    const NodeUUID &nodeUUID,
    UpdateTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler) {}

TransactionResult::SharedConst UpdateTrustLineTransaction::run()
{
    const auto kContractor = mMessage->senderUUID;

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    // Trust line must be removed (updated) in the trust lines storage,
    // and history record about the operation must be written to the history storage.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->update(
            ioTransaction,
            kContractor,
            mMessage->newAmount());
        updateHistory(ioTransaction);

        info() << "Trust line to the node " << kContractor
               << " successfully updated to " << mTrustLines->incomingTrustAmount(kContractor);
        return resultDone();

    } catch (ValueError &){
        ioTransaction->rollback();
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "Cannot opent trustline with zero amount";
        return resultDone();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        info() << "Attempt to update trust line to the node " << kContractor << " failed. "
               << "There is no incoming trust line to this node is present. "
               << "Details are: " << e.what();
        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        info() << "Attempt to update trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

const string UpdateTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[UpdateTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

void UpdateTrustLineTransaction::updateHistory(
    IOTransaction::Shared ioTransaction)
{
    auto record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::Updating,
        mMessage->senderUUID,
        mMessage->newAmount());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}
