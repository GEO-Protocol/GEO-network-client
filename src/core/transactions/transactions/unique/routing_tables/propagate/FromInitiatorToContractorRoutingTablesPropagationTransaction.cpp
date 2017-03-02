#include "FromInitiatorToContractorRoutingTablesPropagationTransaction.h"

FromInitiatorToContractorRoutingTablesPropagationTransaction::FromInitiatorToContractorRoutingTablesPropagationTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID
    ),
    mTrustLinesManager(trustLinesManager) {

    mContractorsUUIDs.push_back(contractorUUID);
}

FromInitiatorToContractorRoutingTablesPropagationTransaction::FromInitiatorToContractorRoutingTablesPropagationTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer
    ),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::run() {

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
            throw ConflictError("FromInitiatorToContractorRoutingTablesPropagationTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

pair<bool, TransactionResult::SharedConst> FromInitiatorToContractorRoutingTablesPropagationTransaction::checkContext() {

    if (mExpectationResponsesCount == mContext.size()) {

        for (const auto& responseMessage : mContext) {

            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("FromInitiatorToContractorRoutingTablesPropagationTransaction::checkContext: "
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
                    *mContractorsUUIDs.begin(),
                    mTransactionUUID,
                    kResponseCodeSuccess
                )
            )
        );

    } else {
        throw ConflictError("FromInitiatorToContractorRoutingTablesPropagationTransaction::checkContext: "
                                "Transaction waiting responses count " + to_string(1) +
                                " has " + to_string(mContext.size())
        );
    }
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::propagateFirstLevelRoutingTable() {

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

bool FromInitiatorToContractorRoutingTablesPropagationTransaction::isContractorsCountEnoughForRoutingTablesPropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::trySendFirstLevelRoutingTable() {

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

void FromInitiatorToContractorRoutingTablesPropagationTransaction::sendFirstLevelRoutingTable() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);
    firstLevelMessage->setPropagationStep(
        RoutingTablesMessage::PropagationStep::FromInitiatorToContractor);

    vector<pair<const NodeUUID, const TrustLineDirection>> neighborsAndDirections;
    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {

        if (mContractorsUUIDs[0] == contractorAndTrustLine.first) {
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
        mContractorsUUIDs[0]
    );
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::propagateSecondLevelRoutingTable() {

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

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::trySendSecondLevelRoutingTable() {

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

void FromInitiatorToContractorRoutingTablesPropagationTransaction::sendSecondLevelRoutingTable() {

    SecondLevelRoutingTableOutgoingMessage::Shared secondLevelMessage = make_shared<SecondLevelRoutingTableOutgoingMessage>(mNodeUUID);

    vector<pair<const NodeUUID, const TrustLineDirection>> neighborsAndDirections;

#ifdef DEBUG
    for (size_t i = 0; i < 10; ++ i) {
        NodeUUID secondLevelContractor;
        TrustLineDirection direction;

        srand(
            time(NULL));

        int randomValue = rand() % 2;
        switch (randomValue) {
            case 0: {
                direction = TrustLineDirection::Outgoing;
                break;
            }

            case 1: {
                direction = TrustLineDirection::Incoming;
                break;
            }

            case 2: {
                direction = TrustLineDirection::Both;
                break;
            }

            default: {
                direction = TrustLineDirection::Nowhere;
            }
        }

        neighborsAndDirections.push_back(
            make_pair(
                secondLevelContractor,
                direction
            )
        );

    }
#endif

    NodeUUID randomFirstLevelNeighbor;
    secondLevelMessage->pushBack(
        randomFirstLevelNeighbor,
        neighborsAndDirections
    );

    Message::Shared message = dynamic_pointer_cast<Message>(secondLevelMessage);
    addMessage(
        message,
        mContractorsUUIDs[0]
    );
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::waitingForRoutingTablePropagationResponse() {


    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            mConnectionTimeout
        )
    );
}

void FromInitiatorToContractorRoutingTablesPropagationTransaction::prepareToNextStep() {

    resetRequestsCounter();
    restoreStandardConnectionTimeout();
    resetExpectationResponsesCounter();
    clearContext();
}
