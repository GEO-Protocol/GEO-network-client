#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    NodeUUID &nodeUUID,
    UpdateTrustLineMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
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

    UniqueTransaction(scheduler),
    mTrustLinesManager(manager) {

    deserializeFromBytes(buffer);
}

UpdateTrustLineMessage::Shared UpdateTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> UpdateTrustLineTransaction::serializeToBytes() {

    auto parentBytesAndCount = serializeParentToBytes();
    auto messageBytesAndCount = mMessage->serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +  messageBytesAndCount.second;
    byte *data = (byte *) calloc (
        bytesCount,
        sizeof(byte)
    );
    //-----------------------------------------------------
    memcpy(
        data,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        data + parentBytesAndCount.second,
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        BytesShared(data, free),
        bytesCount
    );
}

void UpdateTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    deserializeParentFromBytes(buffer);
    byte *commandBuffer = (byte *) calloc(
        UpdateTrustLineMessage::kRequestedBufferSize(),
        sizeof(byte)
    );
    memcpy(
        commandBuffer,
        buffer.get() + kOffsetToInheritedBytes(),
        UpdateTrustLineMessage::kRequestedBufferSize()
    );
    BytesShared commandBufferShared(commandBuffer, free);
    UpdateTrustLineMessage *message = new UpdateTrustLineMessage(commandBufferShared);
    mMessage = UpdateTrustLineMessage::Shared(message);
}

TransactionResult::Shared UpdateTrustLineTransaction::run() {

    switch (mStep) {

        case 1: {
            if (checkJournal()) {
                sendResponse(400);
                return makeResult(mMessage->customCodeResult(400));
            }
            increaseStepsCounter();
        }

        case 2: {
            if (checkSameTypeTransactions()) {
                sendResponse(AcceptTrustLineMessage::kResultCodeTransactionConflict);
                return makeResult(mMessage->resultTransactionConflict());
            }
            increaseStepsCounter();
        }

        case 3: {
            if (checkTrustLineDirectionExisting()) {
                if (checkTrustLineAmount()) {
                    updateIncomingTrustAmount();
                    sendResponse(UpdateTrustLineMessage::kResultCodeAccepted);
                    return makeResult(mMessage->resultAccepted());

                } else {
                    sendResponse(UpdateTrustLineMessage::kResultCodeRejected);
                    return makeResult(mMessage->resultRejected());
                }

            } else {
                sendResponse(UpdateTrustLineMessage::kResultCodeConflict);
                return makeResult(mMessage->resultConflict());
            }
        }

        default: {
            throw ConflictError("UpdateTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }
    }
}

bool UpdateTrustLineTransaction::checkJournal() {

    // return journal->hasRecordByWeek(mMessage->sender());
    return false;
}

bool UpdateTrustLineTransaction::checkSameTypeTransactions() {

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

bool UpdateTrustLineTransaction::checkTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(
        mMessage->senderUUID(),
        TrustLineDirection::Incoming
    );
}

bool UpdateTrustLineTransaction::checkTrustLineAmount() {

    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID()) <= mMessage->newAmount();
}

void UpdateTrustLineTransaction::updateIncomingTrustAmount() {

    mTrustLinesManager->setIncomingTrustAmount(
        mMessage->senderUUID(),
        mMessage->newAmount()
    );
}

void UpdateTrustLineTransaction::sendResponse(
    uint16_t code) {

    Message *message = new Response(
        mNodeUUID,
        mMessage->transactionUUID(),
        code
    );

    addMessage(
        Message::Shared(message),
        mMessage->senderUUID()
    );
}

TransactionResult::Shared UpdateTrustLineTransaction::makeResult(
    MessageResult::SharedConst messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::Shared(transactionResult);
}