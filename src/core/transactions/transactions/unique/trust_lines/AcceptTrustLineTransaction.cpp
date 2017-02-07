#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    NodeUUID &nodeUUID,
    AcceptTrustLineMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mMessage(message),
    mTrustLinesManager(manager) {}

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> AcceptTrustLineTransaction::serializeToBytes() {

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

void AcceptTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    deserializeParentFromBytes(buffer);
    byte *commandBuffer = (byte *) calloc(
        AcceptTrustLineMessage::kRequestedBufferSize(),
        sizeof(byte)
    );
    memcpy(
        commandBuffer,
        buffer.get() + kOffsetToDataBytes(),
        AcceptTrustLineMessage::kRequestedBufferSize()
    );
    BytesShared commandBufferShared(commandBuffer, free);
    AcceptTrustLineMessage *message = new AcceptTrustLineMessage(commandBufferShared.get());
    mMessage = AcceptTrustLineMessage::Shared(message);
}

TransactionResult::Shared AcceptTrustLineTransaction::run() {

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
                    sendResponse(AcceptTrustLineMessage::kResultCodeAccepted);
                    return makeResult(mMessage->resultAccepted());

                } else {
                    sendResponse(AcceptTrustLineMessage::kResultCodeConflict);
                    return makeResult(mMessage->resultConflict());
                }

            } else {
                createTrustLine();
                sendResponse(AcceptTrustLineMessage::kResultCodeAccepted);
                return makeResult(mMessage->resultAccepted());
            }
        }

        default: {
            throw ConflictError("AcceptTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }

}

bool AcceptTrustLineTransaction::checkJournal() {

    // return journal->hasRecordByWeek(mMessage->sender());
    return false;
}

bool AcceptTrustLineTransaction::checkSameTypeTransactions() {

    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->transactionType()) {

            case BaseTransaction::TransactionType::AcceptTrustLineTransactionType: {
                AcceptTrustLineTransaction::Shared acceptTrustLineTransaction = static_pointer_cast<AcceptTrustLineTransaction>(it.first);
                if (mTransactionUUID != it.first->UUID()) {
                    if (mMessage->senderUUID() == acceptTrustLineTransaction->message()->senderUUID()) {
                        return true;
                    }
                }
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
                UpdateTrustLineTransaction::Shared updateTrustLineTransaction = static_pointer_cast<UpdateTrustLineTransaction>(it.first);
                if (mTransactionUUID != it.first->UUID()) {
                    if (mMessage->senderUUID() == updateTrustLineTransaction->message()->senderUUID()) {
                        return true;
                    }
                }
                break;
            }

            case BaseTransaction::TransactionType::RejectTrustLineTransactionType: {
                RejectTrustLineTransaction::Shared rejectTrustLineTransaction = static_pointer_cast<RejectTrustLineTransaction>(it.first);
                if (mTransactionUUID != it.first->UUID()) {
                    if (mMessage->senderUUID() == rejectTrustLineTransaction->message()->senderUUID()) {
                        return true;
                    }
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

bool AcceptTrustLineTransaction::checkTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(
        mMessage->senderUUID(),
        TrustLineDirection::Incoming
    );
}

bool AcceptTrustLineTransaction::checkTrustLineAmount() {


    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID()) == mMessage->amount();
}

void AcceptTrustLineTransaction::createTrustLine() {

    try {
        mTrustLinesManager->accept(
            mMessage->senderUUID(),
            mMessage->amount()
        );

    } catch (std::exception &e) {
        throw Exception(e.what());
    }
}

void AcceptTrustLineTransaction::sendResponse(
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

TransactionResult::Shared AcceptTrustLineTransaction::makeResult(
    MessageResult::SharedConst messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::Shared(transactionResult);
}