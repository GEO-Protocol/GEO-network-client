#include "FromInitiatorToContractorRoutingTablePropagationTransaction.h"

FromInitiatorToContractorRoutingTablePropagationTransaction::FromInitiatorToContractorRoutingTablePropagationTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID
    ),
    mTrustLinesManager(trustLinesManager) {}

FromInitiatorToContractorRoutingTablePropagationTransaction::FromInitiatorToContractorRoutingTablePropagationTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer
    ),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablePropagationTransaction::run() {

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
            throw ConflictError("FromInitiatorToContractorRoutingTablePropagationTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

pair<bool, TransactionResult::SharedConst> FromInitiatorToContractorRoutingTablePropagationTransaction::checkContext() {

    if (mExpectationResponsesCount == mContext.size()) {

        for (const auto& responseMessage : mContext) {

            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("FromInitiatorToContractorRoutingTablePropagationTransaction::checkContext: "
                                        "Illegal message type in context.");
            }

            RoutingTablesResponse::Shared response = static_pointer_cast<RoutingTablesResponse>(responseMessage);
            if (response->code() != kResponseCodeSuccess) {
                return make_pair(
                    false,
                    TransactionResult::Shared(nullptr)
                );
            }

        }

        return make_pair(
            true,
            transactionResultFromMessage(
                make_shared<MessageResult>(
                    mContractorUUID,
                    mTransactionUUID,
                    kResponseCodeSuccess
                )
            )
        );

    } else {
        throw ConflictError("FromInitiatorToContractorRoutingTablePropagationTransaction::checkContext: "
                                "Transaction waiting responses count " + to_string(1) +
                                " has " + to_string(mContext.size())
        );
    }
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablePropagationTransaction::propagateFirstLevelRoutingTable() {

    if (!isContractorsCountEnoughForRoutingTablesPropagation()) {
        return finishTransaction();
    }

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

bool FromInitiatorToContractorRoutingTablePropagationTransaction::isContractorsCountEnoughForRoutingTablesPropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablePropagationTransaction::trySendFirstLevelRoutingTable() {

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

void FromInitiatorToContractorRoutingTablePropagationTransaction::sendFirstLevelRoutingTable() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);

    vector<pair<const NodeUUID, const TrustLineDirection>> neighborsAndDirections;
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

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablePropagationTransaction::propagateSecondLevelRoutingTable() {

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

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablePropagationTransaction::trySendSecondLevelRoutingTable() {

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

void FromInitiatorToContractorRoutingTablePropagationTransaction::sendSecondLevelRoutingTable() {

    SecondLevelRoutingTableOutgoingMessage::Shared secondLevelMessage = make_shared<SecondLevelRoutingTableOutgoingMessage>(mNodeUUID);

    vector<pair<const NodeUUID, const TrustLineDirection>> neighborsAndDirections;

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

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablePropagationTransaction::waitingForRoutingTablePropagationResponse() {


    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            mConnectionTimeout
        )
    );
}

void FromInitiatorToContractorRoutingTablePropagationTransaction::prepareToNextStep() {

    resetRequestsCounter();
    restoreStandardConnectionTimeout();
    resetExpectationResponsesCounter();
    clearContext();
}
