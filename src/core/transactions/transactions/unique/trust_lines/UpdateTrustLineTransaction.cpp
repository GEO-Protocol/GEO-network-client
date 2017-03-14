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

pair<BytesShared, size_t> UpdateTrustLineTransaction::serializeToBytes() const{

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

void UpdateTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(
        buffer);

    BytesShared messageBufferShared = tryMalloc(
        UpdateTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        UpdateTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    mMessage = make_shared<UpdateTrustLineMessage>(
        messageBufferShared);
}

TransactionResult::SharedConst UpdateTrustLineTransaction::run() {

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

                mStep = Stages::CheckUnicity;
            }

            case Stages::CheckUnicity: {
                if (!isTransactionToContractorUnique()) {
                    sendResponseCodeToContractor(
                        UpdateTrustLineMessage::kResultCodeTransactionConflict);

                    return transactionResultFromMessage(
                        mMessage->resultTransactionConflict());
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
                        UpdateTrustLineMessage::kResultCodeConflict);

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
                               "Message -> " + e.what());
    }
}

bool UpdateTrustLineTransaction::checkJournal() {

    return false;
}

bool UpdateTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool UpdateTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->senderUUID(), TrustLineDirection::Incoming) ||
           mTrustLinesManager->checkDirection(mMessage->senderUUID(), TrustLineDirection::Both);
}

bool UpdateTrustLineTransaction::isIncomingTrustLineCouldBeModified() {

    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID()) <= mMessage->newAmount();
}

void UpdateTrustLineTransaction::updateIncomingTrustAmount() {

    mTrustLinesManager->setIncomingTrustAmount(
        mMessage->senderUUID(),
        mMessage->newAmount());
}

void UpdateTrustLineTransaction::logUpdatingTrustLineOperation() {

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Updating,
        mMessage->senderUUID(),
        mMessage->newAmount());

    mOperationsHistoryStorage->addRecord(
        record);
}

void UpdateTrustLineTransaction::sendResponseCodeToContractor(
    uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID(),
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}