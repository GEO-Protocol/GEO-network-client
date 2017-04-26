#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    AcceptTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {

    deserializeFromBytes(
        buffer);
}

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst AcceptTrustLineTransaction::run() {

    switch (mStep) {

        case Stages::CheckJournal: {
            if (checkJournal()) {
                sendResponseCodeToContractor(
                        400);
                mStep = Stages::CheckIncomingDirection;
            }
        }
        case Stages::CheckIncomingDirection: {
            if (isIncomingTrustLineDirectionExisting()) {

                if (isIncomingTrustLineAlreadyAccepted()) {
                    sendResponseCodeToContractor(
                        AcceptTrustLineMessage::kResultCodeAccepted);

                    return transactionResultFromMessage(
                        mMessage->resultAccepted());

                } else {
                    sendResponseCodeToContractor(
                        AcceptTrustLineMessage::kResultCodeConflict);

                    return transactionResultFromMessage(
                        mMessage->resultConflict());
                }

            } else {
                acceptTrustLine();
                logAcceptingTrustLineOperation();
                sendResponseCodeToContractor(
                    AcceptTrustLineMessage::kResultCodeAccepted);

                return transactionResultFromMessage(
                    mMessage->resultAccepted());
            }
        }

        default: {
            throw ConflictError("AcceptTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

bool AcceptTrustLineTransaction::checkJournal() {
//  todo add check journal method
    return false;
}

bool AcceptTrustLineTransaction::isTransactionToContractorUnique() {
//    todo  CheckTransactionOnUnique
    return true;
}

bool AcceptTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->senderUUID, TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->senderUUID, TrustLineDirection::Both);
}

bool AcceptTrustLineTransaction::isIncomingTrustLineAlreadyAccepted() {


    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID) == mMessage->amount();
}

void AcceptTrustLineTransaction::acceptTrustLine() {

    mTrustLinesManager->accept(
        mMessage->senderUUID,
        mMessage->amount());
}

void AcceptTrustLineTransaction::logAcceptingTrustLineOperation() {

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Accepting,
        mMessage->senderUUID,
        mMessage->amount());

    mOperationsHistoryStorage->addRecord(
        record);
}

void AcceptTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID,
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}