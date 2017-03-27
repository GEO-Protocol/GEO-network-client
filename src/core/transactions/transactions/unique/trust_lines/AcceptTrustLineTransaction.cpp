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

pair<BytesShared, size_t> AcceptTrustLineTransaction::serializeToBytes() const{

    auto parentBytesAndCount = TrustLineTransaction::serializeToBytes();
    auto messageBytesAndCount = mMessage->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + messageBytesAndCount.second;

    BytesShared dataBytesShared = tryMalloc(
        bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second);
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void AcceptTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(
        buffer);

    BytesShared messageBufferShared = tryMalloc(
        AcceptTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        AcceptTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    mMessage = make_shared<AcceptTrustLineMessage>(
        messageBufferShared);
}

TransactionResult::SharedConst AcceptTrustLineTransaction::run() {

    try {
        switch (mStep) {

            case Stages::CheckJournal: {
                if (checkJournal()) {
                    sendResponseCodeToContractor(
                        400);

                    return transactionResultFromMessage(
                        mMessage->customCodeResult(
                            400));
                }

                mStep = Stages::CheckIncomingDirection;
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

    } catch (exception &e) {
        throw RuntimeError("AcceptTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + string(e.what()));
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

    return mTrustLinesManager->checkDirection(mMessage->senderUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->senderUUID(), TrustLineDirection::Both);
}

bool AcceptTrustLineTransaction::isIncomingTrustLineAlreadyAccepted() {


    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID()) == mMessage->amount();
}

void AcceptTrustLineTransaction::acceptTrustLine() {

    mTrustLinesManager->accept(
        mMessage->senderUUID(),
        mMessage->amount());
}

void AcceptTrustLineTransaction::logAcceptingTrustLineOperation() {

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Accepting,
        mMessage->senderUUID(),
        mMessage->amount());

    mOperationsHistoryStorage->addRecord(
        record);
}

void AcceptTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID(),
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}