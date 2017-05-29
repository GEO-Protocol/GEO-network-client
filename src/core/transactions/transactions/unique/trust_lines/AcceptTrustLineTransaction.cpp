#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    AcceptTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler) {}

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst AcceptTrustLineTransaction::run() {

    const auto kContractor = mMessage->senderUUID;

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        sendResponseCodeToContractor(mMessage->kResultCodeRejected);
        return transactionResultFromMessage(
            mMessage->resultRejected());
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLines->accept(
            ioTransaction,
            kContractor,
            mMessage->amount());

        updateHistory(ioTransaction);


        // Launching transaction for routing tables population
        if (mTrustLines->trustLineReadOnly(kContractor)->direction() != TrustLineDirection::Both) {
            const auto kTransaction = make_shared<TrustLineStatesHandlerTransaction>(
                currentNodeUUID(),
                currentNodeUUID(),
                currentNodeUUID(),
                mMessage->senderUUID,
                TrustLineStatesHandlerTransaction::Created,
                0,
                mTrustLines,
                mStorageHandler,
                mLog);

            launchSubsidiaryTransaction(kTransaction);
        }

        info() << "Trust line to the node " << kContractor << " was successfully opened.";
        sendResponseCodeToContractor(mMessage->kResultCodeAccepted);
        return transactionResultFromMessage(mMessage->resultAccepted());

    } catch (ConflictError &) {
        ioTransaction->rollback();
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "It seems that other transaction already opened the trust line during response receiveing.";
        sendResponseCodeToContractor(mMessage->kResultCodeConflict);
        return transactionResultFromMessage(mMessage->resultConflict());

    } catch (IOError &e) {
        ioTransaction->rollback();
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        sendResponseCodeToContractor(mMessage->kResultCodeRejected);
        throw e;
    }
}

void AcceptTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID,
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}

const string AcceptTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[AcceptTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

void AcceptTrustLineTransaction::updateHistory(
    IOTransaction::Shared ioTransaction)
{
    auto record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::Accepting,
        mMessage->senderUUID,
        mMessage->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}