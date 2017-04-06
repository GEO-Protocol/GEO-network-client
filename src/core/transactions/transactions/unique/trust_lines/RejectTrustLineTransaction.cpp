#include "RejectTrustLineTransaction.h"

RejectTrustLineTransaction::RejectTrustLineTransaction(
    const NodeUUID &nodeUUID,
    RejectTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        nodeUUID
    ),
    mMessage(message),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {}


RejectTrustLineTransaction::RejectTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {

    deserializeFromBytes(
        buffer);
}

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> RejectTrustLineTransaction::serializeToBytes() const{

    throw NotImplementedError("RejectTrustLineTransaction::serializeToBytes: "
        "Method not implemented");
}

void RejectTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(
        buffer);

    throw NotImplementedError("RejectTrustLineTransaction::deserializeFromBytes: "
        "Method not implemented");
}

TransactionResult::SharedConst RejectTrustLineTransaction::run() {

    try {
        switch(mStep) {

            case Stages::CheckUnicity: {
                if (!isTransactionToContractorUnique()) {
                    sendResponseCodeToContractor(
                        RejectTrustLineMessage::kResultCodeTransactionConflict);

                    return transactionResultFromMessage(
                        mMessage->resultTransactionConflict());
                }

                mStep = Stages::CheckIncomingDirection;
            }

            case Stages::CheckIncomingDirection: {
                if (isIncomingTrustLineDirectionExisting()) {

                    if (checkDebt()) {
                        suspendTrustLineDirectionFromContractor();
                        sendResponseCodeToContractor(
                            RejectTrustLineMessage::kResultCodeRejectDelayed);

                        return transactionResultFromMessage(
                            mMessage->resultRejectDelayed());

                    } else {
                        rejectTrustLine();
                        logRejectingTrustLineOperation();
                        sendResponseCodeToContractor(
                            RejectTrustLineMessage::kResultCodeRejected);

                        return transactionResultFromMessage(
                            mMessage->resultRejected());
                    }

                }
            }

            default: {
                throw ConflictError("RejectTrustLineTransaction::run: "
                                        "Illegal step execution.");
            }

        }

    } catch (exception &e) {
        throw RuntimeError("RejectTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + string(e.what()));
    }
}

bool RejectTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool RejectTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Both);
}

void RejectTrustLineTransaction::suspendTrustLineDirectionFromContractor() {

    return mTrustLinesManager->suspendDirection(
        mMessage->contractorUUID(),
        TrustLineDirection::Incoming);

}

void RejectTrustLineTransaction::rejectTrustLine() {

    mTrustLinesManager->reject(
        mMessage->contractorUUID());
}

void RejectTrustLineTransaction::logRejectingTrustLineOperation() {

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Rejecting,
        mMessage->senderUUID());

    mOperationsHistoryStorage->addRecord(
        record);
}

bool RejectTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mMessage->contractorUUID()) == BalanceRange::Negative;
}

void RejectTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID(),
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}