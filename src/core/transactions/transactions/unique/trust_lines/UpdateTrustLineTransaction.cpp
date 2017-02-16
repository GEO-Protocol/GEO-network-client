#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    NodeUUID &nodeUUID,
    UpdateTrustLineMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mMessage(message),
    mTrustLinesManager(manager) {}

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(scheduler),
    mTrustLinesManager(manager) {

    deserializeFromBytes(buffer);
}

UpdateTrustLineMessage::Shared UpdateTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> UpdateTrustLineTransaction::serializeToBytes() const{

    auto parentBytesAndCount = TrustLineTransaction::serializeToBytes();
    auto messageBytesAndCount = mMessage->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +  messageBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void UpdateTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(UpdateTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        UpdateTrustLineMessage::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mMessage = UpdateTrustLineMessage::Shared(
        new UpdateTrustLineMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst UpdateTrustLineTransaction::run() {

    try {
        switch (mStep) {

            case 1: {
                if (checkJournal()) {
                    sendResponseCodeToContractor(400);
                    return transactionResultFromMessage(mMessage->customCodeResult(400));
                }
                increaseStepsCounter();
            }

            case 2: {
                if (isTransactionToContractorUnique()) {
                    sendResponseCodeToContractor(AcceptTrustLineMessage::kResultCodeTransactionConflict);
                    return transactionResultFromMessage(mMessage->resultTransactionConflict());
                }
                increaseStepsCounter();
            }

            case 3: {
                if (isIncomingTrustLineDirectionExisting()) {
                    if (isIncomingTrustLineCouldBeModified()) {
                        updateIncomingTrustAmount();
                        sendResponseCodeToContractor(UpdateTrustLineMessage::kResultCodeAccepted);
                        return transactionResultFromMessage(mMessage->resultAccepted());

                    } else {
                        sendResponseCodeToContractor(UpdateTrustLineMessage::kResultCodeRejected);
                        return transactionResultFromMessage(mMessage->resultRejected());
                    }

                } else {
                    sendResponseCodeToContractor(UpdateTrustLineMessage::kResultCodeConflict);
                    return transactionResultFromMessage(mMessage->resultConflict());
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

    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->transactionType()) {

            case BaseTransaction::TransactionType::AcceptTrustLineTransactionType: {
                AcceptTrustLineTransaction::Shared acceptTrustLineTransaction = static_pointer_cast<AcceptTrustLineTransaction>(it.first);
                if (mMessage->senderUUID() == acceptTrustLineTransaction->message()->senderUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
                UpdateTrustLineTransaction::Shared updateTrustLineTransaction = static_pointer_cast<UpdateTrustLineTransaction>(it.first);
                if (mMessage->senderUUID() == updateTrustLineTransaction->message()->senderUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::RejectTrustLineTransactionType: {
                RejectTrustLineTransaction::Shared rejectTrustLineTransaction = static_pointer_cast<RejectTrustLineTransaction>(it.first);
                if (mMessage->senderUUID() == rejectTrustLineTransaction->message()->senderUUID()) {
                    return true;
                }
                break;
            }

            default: {
                break;
            }

        }

    }

    return false;
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
        mMessage->newAmount()
    );
}

void UpdateTrustLineTransaction::sendResponseCodeToContractor(
    uint16_t code) {

    Message *message = new Response(
        mNodeUUID,
        const_cast<TransactionUUID&> (mMessage->transactionUUID()),
        code
    );

    addMessage(
        Message::Shared(message),
        mMessage->senderUUID()
    );
}