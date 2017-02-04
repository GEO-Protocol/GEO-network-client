#include "SendRoutingTablesTransaction.h"

SendRoutingTablesTransaction::SendRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    NodeUUID &contractorUUID,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(
        BaseTransaction::TransactionType::SendRoutingTablesTransactionType,
        nodeUUID,
        scheduler
    ),
    mContractorUUID(contractorUUID){}

pair<BytesShared, size_t> SendRoutingTablesTransaction::serializeToBytes() {

    throw NotImplementedError("SendRoutingTablesTransaction::serializeToBytes: "
                                  "Method not implemeted.");
}

void SendRoutingTablesTransaction::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("SendRoutingTablesTransaction::deserializeFromBytes: "
                                  "Method not implemeted.");
}

const NodeUUID &SendRoutingTablesTransaction::contractorUUID() const {

    return mContractorUUID;
}

TransactionResult::Shared SendRoutingTablesTransaction::run() {

    switch(mStep) {

        case 1: {
            checkSameTypeTransactions();
            increaseStepsCounter();
        }

        case 2: {
            if (mContext.get() != nullptr) {
                if (checkFirstLevelExchangeContext()) {
                    resetRequestsCounter();
                    increaseStepsCounter();

                } else {
                    //TODO:: ? return message result and relaunch t/a from manager.
                }

            } else {
                if (mRequestCounter < kMaxRequestsCount) {
                    sendMessageWithFirstLevelRoutingTable();
                    increaseRequestsCounter();

                } else {
                    return repeatSecondStep();
                }
            }
            return waitingForFirstLevelRoutingTableAcceptedResponse();

        }

        case 3: {
            break;
        }

        default: {
            throw ConflictError("SendRoutingTablesTransaction::run: "
                                    "Illegal step execution.");
        }
    }

}

void SendRoutingTablesTransaction::checkSameTypeTransactions() {

    auto transactions = pendingTransactions();
    for (auto const &transactionAndState : *transactions) {

        switch(transactionAndState.first->transactionType()) {

            case BaseTransaction::TransactionType::SendRoutingTablesTransactionType: {
                SendRoutingTablesTransaction::Shared sendRoutingTablesTransaction = static_pointer_cast<SendRoutingTablesTransaction>(transactionAndState.first);
                if (sendRoutingTablesTransaction->contractorUUID() == mContractorUUID) {
                    killTransaction(sendRoutingTablesTransaction->UUID());
                }
                break;
            }

            case BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType: {
                break;
            }

            default: {
                break;
            }
        }
    }

}

bool SendRoutingTablesTransaction::checkFirstLevelExchangeContext() {

    return true;
}

void SendRoutingTablesTransaction::sendMessageWithFirstLevelRoutingTable() {

    /*FirstLevelRoutingTableOutgoingMessage *firstLevelMessage = new FirstLevelRoutingTableOutgoingMessage(
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID
    );
    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessageShared(firstLevelMessage);

    NodeUUID randomNeighbor;
    firstLevelMessage->pushBack(
        randomNeighbor,
        TrustLineDirection::Outgoing
    );

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessageShared);
    addMessage(
        message,
        mContractorUUID
    );*/

}

TransactionResult::Shared SendRoutingTablesTransaction::repeatSecondStep() {

    resetRequestsCounter();

    TransactionState *transactionState = new TransactionState(
        kSecondStepRepeatDelay,
        true
    );

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared SendRoutingTablesTransaction::waitingForFirstLevelRoutingTableAcceptedResponse() {

    TransactionState *transactionState = new TransactionState(
        kConnectionTimeout,
        Message::MessageTypeID::ResponseMessageType
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}
