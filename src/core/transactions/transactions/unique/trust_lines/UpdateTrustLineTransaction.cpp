#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    const NodeUUID &nodeUUID,
    UpdateTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {}

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {

    deserializeFromBytes(
        buffer);
}

UpdateTrustLineMessage::Shared UpdateTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst UpdateTrustLineTransaction::run() {

    try {
        switch (mStep) {

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

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Updating,
        mMessage->senderUUID,
        mMessage->newAmount());

    mOperationsHistoryStorage->addRecord(
        record);
}

void UpdateTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID,
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}