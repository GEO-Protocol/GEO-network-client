#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    const NodeUUID &nodeUUID,
    AcceptTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

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

                if (!mTrustLinesManager->checkDirection(
                    mMessage->senderUUID,
                    TrustLineDirection::Both)) {
                    const auto kTransaction = make_shared<TrustLineStatesHandlerTransaction>(
                        currentNodeUUID(),
                        currentNodeUUID(),
                        currentNodeUUID(),
                        mMessage->senderUUID,
                        TrustLineStatesHandlerTransaction::TrustLineState::Created,
                        0,
                        mTrustLinesManager,
                        mStorageHandler,
                        mLog);
                    launchSubsidiaryTransaction(kTransaction);
                }

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

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Accepting,
        mMessage->senderUUID,
        mMessage->amount());

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
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