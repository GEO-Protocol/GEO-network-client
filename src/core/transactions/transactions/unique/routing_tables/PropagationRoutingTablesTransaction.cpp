#include "PropagationRoutingTablesTransaction.h"

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    NodeUUID &contractorUUID,
    TransactionsScheduler *scheduler,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID,
        scheduler
    ),
    mTrustLinesManager(trustLinesManager) {}

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        buffer,
        scheduler
    ),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::run() {

    if (!mContext.empty()) {
        cout << "Waking up from response" << endl;
    } else {
        cout << "Waking up from scheduler" << endl;
    }

    if (!isUniqueWasChecked) {
        auto flagAndTransactionUUID = isTransactionToContractorUnique();
        if (!flagAndTransactionUUID.first) {
            killTransaction(flagAndTransactionUUID.second);
        }
        isUniqueWasChecked = true;
    }

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            auto firstLevelPropagationResult = propagateFirstLevelRoutingTable();

            if (firstLevelPropagationResult->resultType() == TransactionResult::ResultType::TransactionStateType) {
                return firstLevelPropagationResult;

            } else if (firstLevelPropagationResult->resultType() == TransactionResult::ResultType::MessageResultType) {
                prepareToNextStep();
                increaseStepsCounter();
            }
        }

        case RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep: {
            return propagateSecondLevelRoutingTable();
        }

        default: {
            break;
        }

    }
}

pair<bool, const TransactionUUID> PropagationRoutingTablesTransaction::isTransactionToContractorUnique() {

    auto transactions = pendingTransactions();
    for (auto const &transactionsAndState : *transactions) {

        auto transaction = transactionsAndState.first;

        switch (transaction->transactionType()) {

            case BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType: {

                PropagationRoutingTablesTransaction::Shared propagationRoutingTableTransaction = static_pointer_cast<PropagationRoutingTablesTransaction>(transaction);
                if (mTransactionUUID != propagationRoutingTableTransaction->UUID()) {
                    continue;
                }

                if (mContractorUUID == propagationRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                return make_pair(
                    false,
                    transaction->UUID()
                );

            }

            case BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType: {

                AcceptRoutingTablesTransaction::Shared acceptRoutingTableTransaction = static_pointer_cast<AcceptRoutingTablesTransaction>(transaction);
                if (mTransactionUUID != acceptRoutingTableTransaction->UUID()) {
                    continue;
                }

                if (mContractorUUID == acceptRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                return make_pair(
                    false,
                    transaction->UUID()
                );

            }

            default: {
                break;
            }

        }

    }

    return make_pair(
        true,
        TransactionUUID()
    );
}

bool PropagationRoutingTablesTransaction::isContractorsCountEnoughForRoutingTablePropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

pair<bool, TransactionResult::SharedConst> PropagationRoutingTablesTransaction::checkContext() {

    if (mExpectationResponsesCount == mContext.size()) {

        for (const auto& responseMessage : mContext) {
            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("PropagationRoutingTablesTransaction::checkContext: "
                                        "Illegal message type in context.");
            }
            RoutingTablesResponse::Shared response = static_pointer_cast<RoutingTablesResponse>(responseMessage);
            if (response->code() != 200) {
                return make_pair(
                    false,
                    TransactionResult::Shared(nullptr)
                );
            }
        }

        MessageResult::Shared messageResult = MessageResult::Shared(
            new MessageResult(
                mContractorUUID,
                mTransactionUUID,
                200
            )
        );

        return make_pair(
            true,
            transactionResultFromMessage(messageResult)
        );

    } else {
        throw ConflictError("PropagationRoutingTablesTransaction::checkContext: "
                                "Transaction waiting responses count " + to_string(1) +
                                " has " + to_string(mContext.size())
        );
    }
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::propagateFirstLevelRoutingTable() {

    if (!mContext.empty()) {
        auto flagAndResult = checkContext();
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            return trySendFirstLevelRoutingTable();
        }

    } else {
        return trySendFirstLevelRoutingTable();
    }
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::trySendFirstLevelRoutingTable() {

    if (!isContractorsCountEnoughForRoutingTablePropagation()) {
        return finishTransaction();
    }

    setExpectationResponsesCounter(1);

    if (mRequestCounter < kMaxRequestsCount) {
        sendFirstLevelRoutingTable();
        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }
        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }
    return waitingForRoutingTablePropagationResponse();
}

void PropagationRoutingTablesTransaction::sendFirstLevelRoutingTable() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);

    vector<pair<NodeUUID, TrustLineDirection>> neighborsAndDirections;
    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {
        if (mContractorUUID == contractorAndTrustLine.first) {
            continue;
        }

        neighborsAndDirections.push_back(
            make_pair(
                contractorAndTrustLine.first,
                mTrustLinesManager->trustLine(contractorAndTrustLine.first)->direction()
            )
        );
    }

    firstLevelMessage->pushBack(
        mNodeUUID,
        neighborsAndDirections
    );

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessage);
    addMessage(
        message,
        mContractorUUID
    );
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::propagateSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        auto flagAndResult = checkContext();
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            return trySendSecondLevelRoutingTable();
        }

    } else {
        return trySendSecondLevelRoutingTable();
    }
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::trySendSecondLevelRoutingTable() {

    setExpectationResponsesCounter(1);

    if (mRequestCounter < kMaxRequestsCount) {
        sendSecondLevelRoutingTable();
        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }
        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }
    return waitingForRoutingTablePropagationResponse();
}

void PropagationRoutingTablesTransaction::sendSecondLevelRoutingTable() {

    SecondLevelRoutingTableOutgoingMessage::Shared secondLevelMessage = make_shared<SecondLevelRoutingTableOutgoingMessage>(mNodeUUID);

    vector<pair<NodeUUID, TrustLineDirection>> neighborsAndDirections;

#ifdef DEBUG
    for (size_t i = 0; i < 10; ++ i) {
        NodeUUID neighbor;
        TrustLineDirection direction;

        int randomValue = rand() % 2;
        switch (randomValue) {
            case 0: {
                direction = TrustLineDirection::Outgoing;
            }

            case 1: {
                direction = TrustLineDirection::Incoming;
            }

            case 2: {
                direction = TrustLineDirection::Both;
            }

            default: {
                direction = TrustLineDirection::Nowhere;
            }
        }

        neighborsAndDirections.push_back(
            make_pair(
                neighbor,
                direction
            )
        );
    }
#endif

    secondLevelMessage->pushBack(
        mNodeUUID,
        neighborsAndDirections
    );

    Message::Shared message = dynamic_pointer_cast<Message>(secondLevelMessage);
    addMessage(
        message,
        mContractorUUID
    );
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::waitingForRoutingTablePropagationResponse() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(mConnectionTimeout * 1000)
        ),
        Message::MessageTypeID::RoutingTablesResponseMessageType,
        false
    );


    return transactionResultFromState(
        TransactionState::SharedConst(transactionState)
    );
}

void PropagationRoutingTablesTransaction::prepareToNextStep() {

    resetRequestsCounter();
    restoreStandardConnectionTimeout();
    resetExpectationResponsesCounter();
    clearContext();
}
