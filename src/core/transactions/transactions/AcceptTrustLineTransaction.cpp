#include "AcceptTrustLineTransaction.h"

AcceptTrustLineTransaction::AcceptTrustLineTransaction(
    AcceptTrustLineMessage::Shared message,
    Communicator *communicator,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::AcceptTrustLineTransactionType, scheduler),
    mMessage(message),
    mCommunicator(communicator),
    mTrustLinesInterface(interface) {}

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {
    return mMessage;
}

void AcceptTrustLineTransaction::setContext(
    Message::Shared message) {

    BaseTransaction::mContext = message;
}

pair<byte *, size_t> AcceptTrustLineTransaction::serializeContext() {}

TransactionResult::Shared AcceptTrustLineTransaction::run() {

    ////STEP 1
    /*
     if (journal->hasRecordByWeek(mMessage->sender())) {
         sendAcceptTrustLineMessage(code);
     }*/

    ////STEP 2
    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->type()) {

            case BaseTransaction::TransactionType::AcceptTrustLineTransactionType: {
                AcceptTrustLineTransaction::Shared acceptTrustLineTransaction = static_pointer_cast<AcceptTrustLineTransaction>(
                    it.first);
                if (mMessage->sender() == acceptTrustLineTransaction->message()->sender()) {
                    sendConflictResponse();
                    return conflictErrorResult();
                }
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
                UpdateTrustLineTransaction::Shared updateTrustLineTransaction = static_pointer_cast<UpdateTrustLineTransaction>(
                    it.first);
                if (mMessage->sender() == updateTrustLineTransaction->command()->contractorUUID()) {
                    sendConflictResponse();
                    return conflictErrorResult();
                }
                break;
            }

            case BaseTransaction::TransactionType::RejectTrustLineTransactionType: {
            }

            default: {
                break;
            }

        }
    }


}

void AcceptTrustLineTransaction::sendAcceptTrustLineMessage(
    uint16_t) {


}

void AcceptTrustLineTransaction::sendConflictResponse() {

}

TransactionResult::Shared AcceptTrustLineTransaction::conflictErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(MessageResult::Shared(const_cast<MessageResult *> (mMessage->resultConflict())));
    return TransactionResult::Shared(transactionResult);
}







