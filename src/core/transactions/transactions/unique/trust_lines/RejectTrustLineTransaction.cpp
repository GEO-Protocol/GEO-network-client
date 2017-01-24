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

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const {

    return mMessage;
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
            if (checkTrustLineExisting()) {
                if (checkDebt()) {
                    suspendTrustLineFromContractor();
                    sendResponse(RejectTrustLineMessage::kResultCodeRejectDelayed);
                    return makeResult(mMessage->resultRejectDelayed());

                } else {
                    closeTrustLine();
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

bool RejectTrustLineTransaction::checkTrustLineExisting() {

    cout << "Existing with " << mMessage->contractorUUID().stringUUID() << endl;
    return mTrustLinesManager->checkDirection(
        mMessage->contractorUUID(),
        TrustLineDirection::Incoming
    );
}

void RejectTrustLineTransaction::suspendTrustLineFromContractor() {

    mTrustLinesManager->suspendDirection(
        mMessage->contractorUUID(),
        TrustLineDirection::Incoming
    );
}

void RejectTrustLineTransaction::closeTrustLine() {

    mTrustLinesManager->close(mMessage->contractorUUID());
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
    MessageResult::Shared messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::Shared(transactionResult);
}
