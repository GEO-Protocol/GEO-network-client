#include "PropagationRoutingTablesTransaction.h"

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TrustLineUUID &trustLineUUID,
    TransactionsScheduler *scheduler,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID,
        trustLineUUID,
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

    if (!isUniqueWasChecked) {
        auto flagAndTransactionUUID = isTransactionToContractorUnique();
        if (!flagAndTransactionUUID.first) {
            killTransaction(flagAndTransactionUUID.second);
        }
        isUniqueWasChecked = true;
    }

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            auto firstLevelPropagationResult = propagateFirstLevelRoutingTableFromInitiatorToContractor();

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

                if (mContractorUUID != propagationRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                if (mTrustLineUUID == propagationRoutingTableTransaction->trustLineUUID()) {
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

                if (mContractorUUID != acceptRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                if (mTrustLineUUID == acceptRoutingTableTransaction->trustLineUUID()) {
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

bool PropagationRoutingTablesTransaction::isContractorsCountEnoughForFirstLevelRoutingTablePropagationFromInitiatorToContractor() {

    return mTrustLinesManager->trustLines().size() > 1;
}

pair<bool, TransactionResult::SharedConst> PropagationRoutingTablesTransaction::checkRoutingTablePropagationFromInitiatorToContractorContext() {

    if (mExpectationResponsesCount == mContext.size()) {

        for (const auto& responseMessage : mContext) {
            if (responseMessage->typeID() != Message::MessageTypeID::ResponseMessageType) {
                throw ConflictError("PropagationRoutingTablesTransaction::checkRoutingTablePropagationFromInitiatorToContractorContext: "
                                        "Illegal message type in context.");
            }
            Response::Shared response = static_pointer_cast<Response>(responseMessage);
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
        throw ConflictError("PropagationRoutingTablesTransaction::checkRoutingTablePropagationFromInitiatorToContractorContext: "
                                "Transaction waiting responses count " + to_string(1) +
                                " has " + to_string(mContext.size())
        );
    }
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::propagateFirstLevelRoutingTableFromInitiatorToContractor() {

    if (!mContext.empty()) {
        auto flagAndResult = checkRoutingTablePropagationFromInitiatorToContractorContext();
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            return trySendFirstLevelRoutingTableFromInitiatorToContractor();
        }

    } else {
        return trySendFirstLevelRoutingTableFromInitiatorToContractor();
    }
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::trySendFirstLevelRoutingTableFromInitiatorToContractor() {

    if (!isContractorsCountEnoughForFirstLevelRoutingTablePropagationFromInitiatorToContractor()) {
        return finishTransaction();
    }

    setExpectationResponsesCounter(1);

    if (mRequestCounter < kMaxRequestsCount) {
        sendFirstLevelRoutingTableFromInitiatorToContractor();
        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }
        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }
    return waitingForRoutingTablePropagationResponseFromContractor();
}

void PropagationRoutingTablesTransaction::sendFirstLevelRoutingTableFromInitiatorToContractor() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(
        mNodeUUID,
        mTrustLineUUID
    );

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
        auto flagAndResult = checkRoutingTablePropagationFromInitiatorToContractorContext();
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            return trySendSecondLevelRoutingTableFromInitiatorToContractor();
        }

    } else {
        return trySendSecondLevelRoutingTableFromInitiatorToContractor();
    }
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::trySendSecondLevelRoutingTableFromInitiatorToContractor() {

    setExpectationResponsesCounter(1);

    if (mRequestCounter < kMaxRequestsCount) {
        sendSecondLevelRoutingTableFromInitiatorToContractor();
        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }
        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }
    return waitingForRoutingTablePropagationResponseFromContractor();
}

void PropagationRoutingTablesTransaction::sendSecondLevelRoutingTableFromInitiatorToContractor() {

    SecondLevelRoutingTableOutgoingMessage::Shared secondLevelMessage = make_shared<SecondLevelRoutingTableOutgoingMessage>(
        mNodeUUID,
        mTrustLineUUID
    );

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

TransactionResult::SharedConst PropagationRoutingTablesTransaction::waitingForRoutingTablePropagationResponseFromContractor() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(mConnectionTimeout * 1000)
        ),
        Message::MessageTypeID::ResponseMessageType,
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
