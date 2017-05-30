#include "CloseTrustLineTransaction.h"


CloseTrustLineTransaction::CloseTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::CloseTrustLineTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst CloseTrustLineTransaction::run()
{
    const auto kContractor = mCommand->contractorUUID();

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    // Trust line must be removed (updated) in the trust lines storage,
    // and history record about the operation must be written to the history storage.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLinesManager->close(
            ioTransaction,
            kContractor);
        updateHistory(ioTransaction);


        // Notifiyng remote node about trust line state changed.
        // This message is information only. It is OK if it would be lost, and remote node will never receive it
        // (trust lines would be syncronized again in the future, for example, on payment operation).
        sendMessage<CloseTrustLineMessage>(
            kContractor,
            mNodeUUID,
            mTransactionUUID,
            mNodeUUID);

        info() << "Trust line to the node " << kContractor << " closed successfully.";
        return resultOK();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        info() << "Attempt to close trust line to the node " << kContractor << " failed. "
               << "There is no outgoing trust line to this node is present."
               << "Details are: " << e.what();

        return resultTrustLineIsAbsent();

    } catch (ConflictError &) {
        ioTransaction->rollback();
        info() << "Attempt to close trust line to the node " << kContractor << "failed. "
               << "It seems that trust line is already closed, but there are some reservations on it.";
        return resultPostponedByReservations();

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

void CloseTrustLineTransaction::updateHistory(
    IOTransaction::Shared ioTransaction)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::Closing,
        mCommand->contractorUUID());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
#endif
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultPostponedByReservations() const
{
    return transactionResultFromCommand(
        mCommand->responsePostponedbyreservations());
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultTrustLineIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustlineIsAbsent());
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string CloseTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[CloseTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}
