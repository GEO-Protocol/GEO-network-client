#include "RejectTrustLineTransaction.h"

RejectTrustLineTransaction::RejectTrustLineTransaction(
    NodeUUID &nodeUUID,
    RejectTrustLineMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mMessage(message),
    mTrustLinesManager(manager) {}


RejectTrustLineTransaction::RejectTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> RejectTrustLineTransaction::serializeToBytes() const{

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

void RejectTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(RejectTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        RejectTrustLineMessage::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mMessage = RejectTrustLineMessage::Shared(
        new RejectTrustLineMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst RejectTrustLineTransaction::run() {

    try {
        switch(mStep) {

            case 1: {
                if (isTransactionToContractorUnique()) {
                    sendResponseCodeToContractor(RejectTrustLineMessage::kResultCodeTransactionConflict);
                    return transactionResultFromMessage(mMessage->resultTransactionConflict());
                }
                increaseStepsCounter();
            }

            case 2: {
                if (isIncomingTrustLineDirectionExisting()) {
                    if (checkDebt()) {
                        suspendTrustLineDirectionFromContractor();
                        sendResponseCodeToContractor(RejectTrustLineMessage::kResultCodeRejectDelayed);
                        return transactionResultFromMessage(mMessage->resultRejectDelayed());

                    } else {
                        rejectTrustLine();
                        sendResponseCodeToContractor(RejectTrustLineMessage::kResultCodeRejected);
                        return transactionResultFromMessage(mMessage->resultRejected());
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
                               "Message -> " + e.what());
    }
}

bool RejectTrustLineTransaction::isTransactionToContractorUnique() {

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

bool RejectTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Both);
}

void RejectTrustLineTransaction::suspendTrustLineDirectionFromContractor() {

    return mTrustLinesManager->suspendDirection(
        mMessage->contractorUUID(),
        TrustLineDirection::Incoming
    );

}

void RejectTrustLineTransaction::rejectTrustLine() {

    mTrustLinesManager->reject(mMessage->contractorUUID());
}

bool RejectTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mMessage->contractorUUID()) == BalanceRange::Negative;
}

void RejectTrustLineTransaction::sendResponseCodeToContractor(
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