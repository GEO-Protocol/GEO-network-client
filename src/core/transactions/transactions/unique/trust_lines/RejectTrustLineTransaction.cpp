#include "RejectTrustLineTransaction.h"

RejectTrustLineTransaction::RejectTrustLineTransaction(
    NodeUUID &nodeUUID,
    RejectTrustLineMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
        BaseTransaction::TransactionType::AcceptTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mMessage(message),
    mTrustLinesManager(manager) {}


RejectTrustLineTransaction::RejectTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> RejectTrustLineTransaction::serializeToBytes() {

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

void RejectTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    deserializeParentFromBytes(buffer);
    byte *commandBuffer = (byte *) calloc(
        RejectTrustLineMessage::kRequestedBufferSize(),
        sizeof(byte)
    );
    memcpy(
        commandBuffer,
        buffer.get() + kOffsetToDataBytes(),
        RejectTrustLineMessage::kRequestedBufferSize()
    );
    BytesShared commandBufferShared(commandBuffer, free);
    RejectTrustLineMessage *message = new RejectTrustLineMessage(commandBufferShared);
    mMessage = RejectTrustLineMessage::Shared(message);
}

TransactionResult::Shared RejectTrustLineTransaction::run() {

    switch(mStep) {

        case 1: {
            if (checkSameTypeTransactions()) {
                sendResponse(RejectTrustLineMessage::kResultCodeTransactionConflict);
                return makeResult(mMessage->resultTransactionConflict());
            }
            increaseStepsCounter();
        }

        case 2: {
            if (checkTrustLineDirectionExisting()) {
                if (checkDebt()) {
                    suspendTrustLineFromContractor();
                    sendResponse(RejectTrustLineMessage::kResultCodeRejectDelayed);
                    return makeResult(mMessage->resultRejectDelayed());

                } else {
                    rejectTrustLine();
                    sendResponse(RejectTrustLineMessage::kResultCodeRejected);
                    return makeResult(mMessage->resultRejected());
                }
            }
        }

        default: {
            throw ConflictError("RejectTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

bool RejectTrustLineTransaction::checkSameTypeTransactions() {

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

bool RejectTrustLineTransaction::checkTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Both);
}

void RejectTrustLineTransaction::suspendTrustLineFromContractor() {

    return mTrustLinesManager->suspendDirection(mMessage->contractorUUID(), TrustLineDirection::Incoming);

}

void RejectTrustLineTransaction::rejectTrustLine() {

    mTrustLinesManager->reject(mMessage->contractorUUID());
}

bool RejectTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mMessage->contractorUUID()) == BalanceRange::Negative;
}

void RejectTrustLineTransaction::sendResponse(
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

TransactionResult::Shared RejectTrustLineTransaction::makeResult(
    MessageResult::SharedConst messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::Shared(transactionResult);
}