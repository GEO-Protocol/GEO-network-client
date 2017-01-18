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
            if (checkTrustLineDirection()) {
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
                                    "Illegal step");
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

        switch (it.first->type()) {

            case BaseTransaction::TransactionType::AcceptTrustLineTransactionType: {
                AcceptTrustLineTransaction::Shared acceptTrustLineTransaction = static_pointer_cast<AcceptTrustLineTransaction>(it.first);
                if (mMessage->senderUUID() == acceptTrustLineTransaction->message()->senderUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
                UpdateTrustLineTransaction::Shared updateTrustLineTransaction = static_pointer_cast<UpdateTrustLineTransaction>(it.first);
                if (mMessage->senderUUID() == updateTrustLineTransaction->command()->contractorUUID()) {
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

bool AcceptTrustLineTransaction::checkTrustLineDirection() {

    return mTrustLinesInterface->isDirectionIncoming(mMessage->senderUUID());
}

bool AcceptTrustLineTransaction::checkTrustLineAmount() {

    return mTrustLinesInterface->checkIncomingAmount(
        mMessage->senderUUID(),
      mMessage->amount()
    );
}

void AcceptTrustLineTransaction::sendResponse(
    uint16_t code) {

    Message *message = new Response(
        mCommunicator->nodeUUID(),
        mMessage->transactionUUID(),
        code
    );

    mCommunicator->sendMessage(
        Message::Shared(message),
        mMessage->senderUUID()
    );
}

void AcceptTrustLineTransaction::createTrustLine() {

    try {
        mTrustLinesInterface->open(
            mMessage->senderUUID(),
            mMessage->amount()
        );

    } catch (std::exception &e) {
        throw Exception(e.what());
    }
}

void AcceptTrustLineTransaction::increaseStepsCounter() {

    mStep += 1;
}

TransactionResult::Shared AcceptTrustLineTransaction::makeResult(
    MessageResult::Shared messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::Shared(transactionResult);
}









