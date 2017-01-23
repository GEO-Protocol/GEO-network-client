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

AcceptTrustLineMessage::Shared AcceptTrustLineTransaction::message() const {
    return mMessage;
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
                if (mMessage->senderUUID() == acceptTrustLineTransaction->message()->senderUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransactionType: {
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

    return mTrustLinesManager->checkDirection(
        mMessage->senderUUID(),
        TrustLineDirection::Incoming);
}

bool AcceptTrustLineTransaction::checkTrustLineAmount() {


    return mTrustLinesManager->incomingTrustAmount(mMessage->senderUUID()) == mMessage->amount();
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

TransactionResult::Shared AcceptTrustLineTransaction::makeResult(
    MessageResult::Shared messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::Shared(transactionResult);
}









