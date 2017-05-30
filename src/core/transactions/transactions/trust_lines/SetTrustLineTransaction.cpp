#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLines(manager),
    mStorageHandler(storageHandler) {}

SetTrustLineCommand::Shared SetTrustLineTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst SetTrustLineTransaction::run()
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
        mTrustLines->set(
            ioTransaction,
            kContractor,
            mCommand->newAmount());
        updateHistory(ioTransaction);


        // Notifiyng remote node about trust line state changed.
        // This message is information only. It is OK if it would be lost, and remote node will never receive it
        // (trust lines would be syncronized again in the future, for example, on payment operation).
        sendMessage<SetTrustLineMessage>(
            mCommand->contractorUUID(),
            mNodeUUID,
            mTransactionUUID,
            mCommand->newAmount());

        info() << "Trust line to the node " << kContractor << " updated successfully.";
        return resultOk();

    } catch (ValueError &){
        ioTransaction->rollback();
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "Cannot opent trustline with zero amount";

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        info() << "Attempt to update trust line to the node " << kContractor << " failed. "
               << "There is no outgoing trust line to this node is present. "
               << "Details are: " << e.what();

        return resultTrustlineIsAbsent();

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

TransactionResult::SharedConst SetTrustLineTransaction::resultOk()
{
    return transactionResultFromCommand(
            mCommand->responseCreated());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultTrustlineIsAbsent()
{
    return transactionResultFromCommand(
            mCommand->responseTrustlineIsAbsent());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
            mCommand->responseProtocolError());
}

const string SetTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[SetTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

void SetTrustLineTransaction::updateHistory(
    IOTransaction::Shared ioTransaction)
{
    auto record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::Setting,
        mCommand->contractorUUID(),
        mCommand->newAmount());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}