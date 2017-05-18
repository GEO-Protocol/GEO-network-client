#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    const NodeUUID &nodeUUID,
    UpdateTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

    deserializeFromBytes(
        buffer);
}

UpdateTrustLineMessage::Shared UpdateTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst UpdateTrustLineTransaction::run() {

    try {
        switch (mStep) {

            case Stages::CheckContractorUUIDValidity: {
                if (!isContractorUUIDValid(mMessage->senderUUID))
                    return transactionResultFromMessage(
                        mMessage->resultRejected());
                mStep = Stages::CheckJournal;
            }

            case Stages::CheckJournal: {
                if (checkJournal()) {
                    sendResponseCodeToContractor(
                        400);

                    return transactionResultFromMessage(
                        make_shared<const MessageResult>(
                            currentNodeUUID(),
                            currentTransactionUUID(),
                            400));
                }

                mStep = Stages::CheckIncomingDirection;
            }

            case Stages::CheckIncomingDirection: {
                if (isIncomingTrustLineDirectionExisting()) {

                    if (isIncomingTrustLineCouldBeModified()) {
                        updateIncomingTrustAmount();
                        logUpdatingTrustLineOperation();
                        sendResponseCodeToContractor(
                            UpdateTrustLineMessage::kResultCodeAccepted);

                        return transactionResultFromMessage(
                            mMessage->resultAccepted());

                    } else {
                        sendResponseCodeToContractor(
                            UpdateTrustLineMessage::kResultCodeRejected);

                        return transactionResultFromMessage(
                            mMessage->resultRejected());
                    }

                } else {
                    sendResponseCodeToContractor(
                        UpdateTrustLineMessage::kResultCodeTrustLineAbsent);

                    return transactionResultFromMessage(
                        mMessage->resultConflict());
                }

            }

            default: {
                throw ConflictError("UpdateTrustLineTransaction::run: "
                                        "Illegal step execution.");
            }

        }

    } catch (exception &e) {
        throw RuntimeError("UpdateTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + string(e.what()));
    }
}

bool UpdateTrustLineTransaction::checkJournal() {
// todo add method to check journal
    return false;
}

bool UpdateTrustLineTransaction::isTransactionToContractorUnique() {
    // todo Add method to check transaction unique
    return true;
}

bool UpdateTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->senderUUID, TrustLineDirection::Incoming) ||
           mTrustLinesManager->checkDirection(mMessage->senderUUID, TrustLineDirection::Both);
}

bool UpdateTrustLineTransaction::isIncomingTrustLineCouldBeModified() {

    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID) <= mMessage->newAmount();
}

void UpdateTrustLineTransaction::updateIncomingTrustAmount() {

    mTrustLinesManager->setIncomingTrustAmount(
        mMessage->senderUUID,
        mMessage->newAmount());
}

void UpdateTrustLineTransaction::logUpdatingTrustLineOperation() {

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Updating,
        mMessage->senderUUID,
        mMessage->newAmount());

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
    ioTransaction->trustLineHandler()->saveTrustLine(
        mTrustLinesManager->trustLines().at(
            mMessage->senderUUID));
}

void UpdateTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID,
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}

const string UpdateTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[UpdateTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}
