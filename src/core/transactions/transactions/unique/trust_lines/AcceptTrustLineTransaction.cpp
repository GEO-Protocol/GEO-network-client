#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    NodeUUID &nodeUUID,
    AcceptTrustLineMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(
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

    TrustLineTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> AcceptTrustLineTransaction::serializeToBytes() const{

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

void AcceptTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(AcceptTrustLineMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        AcceptTrustLineMessage::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mMessage = AcceptTrustLineMessage::Shared(
        new AcceptTrustLineMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst AcceptTrustLineTransaction::run() {

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
                    if (isIncomingTrustLineAlreadyAccepted()) {
                        sendResponseCodeToContractor(AcceptTrustLineMessage::kResultCodeAccepted);
                        return transactionResultFromMessage(mMessage->resultAccepted());

                    } else {
                        sendResponseCodeToContractor(AcceptTrustLineMessage::kResultCodeConflict);
                        return transactionResultFromMessage(mMessage->resultConflict());
                    }

                } else {
                    acceptTrustLine();
                    sendResponseCodeToContractor(AcceptTrustLineMessage::kResultCodeAccepted);
                    return transactionResultFromMessage(mMessage->resultAccepted());
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
                               "Message -> " + e.what());
    }
}

bool AcceptTrustLineTransaction::checkJournal() {

    return false;
}

bool AcceptTrustLineTransaction::isTransactionToContractorUnique() {

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

bool AcceptTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->senderUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->senderUUID(), TrustLineDirection::Both);
}

bool AcceptTrustLineTransaction::isIncomingTrustLineAlreadyAccepted() {


    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID()) == mMessage->amount();
}

void AcceptTrustLineTransaction::acceptTrustLine() {

    try {
        mTrustLinesManager->accept(
            mMessage->senderUUID(),
            mMessage->amount()
        );

    } catch (std::exception &e) {
        throw Exception(e.what());
    }
}

void AcceptTrustLineTransaction::sendResponseCodeToContractor(
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