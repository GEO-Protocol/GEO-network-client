#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    AcceptTrustLineMessage::Shared message,
    Communicator *communicator,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::AcceptTrustLineTransactionType, scheduler),
    mMessage(message),
    mCommunicator(communicator),
    mTrustLinesInterface(interface) {

    mStep = 1;
}

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {
    return mMessage;
}

void AcceptTrustLineTransaction::setContext(
    Message::Shared message) {

    BaseTransaction::mContext = message;
}

pair<byte *, size_t> AcceptTrustLineTransaction::serializeContext() {}

TransactionResult::Shared AcceptTrustLineTransaction::run() {

    switch (mStep) {

        case 1: {
            if (checkJournal()) {
                sendAcceptTrustLineResponse(400);
                //TODO:: update journal
                break;
            }
        }

        case 2: {
            if (checkSameTypeTransactions()) {
                sendRejectTrustLineResponse();
                break;
            }
        }

        default: {
            break;
        }

    }

}

bool AcceptTrustLineTransaction::checkJournal() {

    // return journal->hasRecordByWeek(mMessage->sender());
    return false;
}

void AcceptTrustLineTransaction::sendAcceptTrustLineResponse(
    uint16_t code) {

    Message *message = new AcceptTrustLineMessage(
        mCommunicator->nodeUUID(),
        mMessage->transactionUUID(),
        code
    );

    mCommunicator->sendMessage(
        Message::Shared(message),
        mMessage->sender()
    );
}

bool AcceptTrustLineTransaction::checkSameTypeTransactions() {

    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->type()) {

            case BaseTransaction::TransactionType::AcceptTrustLineTransactionType: {
                AcceptTrustLineTransaction::Shared acceptTrustLineTransaction = static_pointer_cast<AcceptTrustLineTransaction>(it.first);
                if (mMessage->sender() == acceptTrustLineTransaction->message()->sender()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
                UpdateTrustLineTransaction::Shared updateTrustLineTransaction = static_pointer_cast<UpdateTrustLineTransaction>(it.first);
                if (mMessage->sender() == updateTrustLineTransaction->command()->contractorUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::RejectTrustLineTransactionType: {
                break;
            }

            default: {
                break;
            }

        }

    }

    return false;
}

void AcceptTrustLineTransaction::sendRejectTrustLineResponse() {

}

void AcceptTrustLineTransaction::increaseStepsCounter() {

    mStep += 1;
}

TransactionResult::Shared AcceptTrustLineTransaction::conflictErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(MessageResult::Shared(const_cast<MessageResult *> (mMessage->resultConflict())));
    return TransactionResult::Shared(transactionResult);
}








