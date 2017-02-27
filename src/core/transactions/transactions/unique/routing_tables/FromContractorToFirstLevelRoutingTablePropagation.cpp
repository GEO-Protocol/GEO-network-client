#include "FromContractorToFirstLevelRoutingTablePropagation.h"

FromContractorToFirstLevelRoutingTablePropagation::FromContractorToFirstLevelRoutingTablePropagation(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    pair<const NodeUUID, const TrustLineDirection> &&relationshipsBetweenInitiatorAndContractor,
    vector<pair<const NodeUUID, const TrustLineDirection>> &&secondLevelRoutingTableFromInitiator,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID
    ),
    mLinkWithInitiator(relationshipsBetweenInitiatorAndContractor),
    mSecondLevelRoutingTable(secondLevelRoutingTableFromInitiator),
    mTrustLinesManager(trustLinesManager) {}

FromContractorToFirstLevelRoutingTablePropagation::FromContractorToFirstLevelRoutingTablePropagation(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer
    ),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagation::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            // Try send messages with information about link between nodes A and B (initiator and contractor).
            // If messages were sent to nodes of B1 level, transaction must wait for responses from nodes of B1 level.
            // So returns result with state back to scheduler and he asleep current transaction.
            // After some time current transaction wake up and check if all responses was received.
            // Execution will be resume from step 'FirstLevelRoutingTableStep'.
            // If all responses was received, transaction continue execution from second step 'SecondLevelRoutingTableStep',
            // else transaction will be finished.
            auto initiatorAndContractorLinkPropagationResult = propagateRelationshipsBetweenInitiatorAndContractor();

            if (initiatorAndContractorLinkPropagationResult->resultType() == TransactionResult::ResultType::TransactionStateType) {
                return initiatorAndContractorLinkPropagationResult;


            } else if (initiatorAndContractorLinkPropagationResult->resultType() == TransactionResult::ResultType::MessageResultType) {
                prepareToNextStep();
                increaseStepsCounter();
            }
        }

        case RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep: {
            return propagateSecondLevelRoutingTable();
        }

        default: {
            throw ConflictError("FromContractorToFirstLevelRoutingTablePropagation::run: "
                                    "Illegal step execution.");
        }

    }
}

pair<bool, TransactionResult::SharedConst> FromContractorToFirstLevelRoutingTablePropagation::checkContext() {

    // Check if received response from each node of B1 level
    if (mExpectationResponsesCount == mContext.size()) {
        for (const auto& responseMessage : mContext) {

            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("FromContractorToFirstLevelRoutingTablePropagation::checkContext: "
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
        // If some of the remote nodes are offline, wait for response some more time.
        // But no more than 10 attempts.
        // Between every attempt using less timeout than when try to wait for responses after sending.
        if (mReWaitingAttemptsCount < kMaxReWaitingAttemptsCount) {
            mReWaitingAttemptsCount += 1;
            return make_pair(
                false,
                waitingForRoutingTablePropagationResponse(kReWaitingTimeout)
            );

        } else {
            // If some of the remote nodes are still offline, returns false flag.
            return make_pair(
                false,
                TransactionResult::Shared(nullptr)
            );
        }
    }
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagation::propagateRelationshipsBetweenInitiatorAndContractor() {

    if (!isContractorsCountEnoughForRoutingTablesPropagation()) {
        return finishTransaction();
    }

    if (!mContext.empty()) {
        auto flagAndResult = checkContext();
        // If all responses were received returns 'MessageResult' to run()
        // and continue execution from second step.
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            // If responses were not received completely - maybe need await some more time
            if (flagAndResult.second != nullptr) {
                if (flagAndResult.second->resultType() == TransactionResult::ResultType::TransactionStateType) {
                    return flagAndResult.second;
                }
            }
            // If transaction still can't collect all responses, try send message again.
            return trySendLinkBetweenInitiatorAndContractor();
        }

    } else {
        // If transaction still can't collect all responses, try send message again.
        return trySendLinkBetweenInitiatorAndContractor();
    }
}

bool FromContractorToFirstLevelRoutingTablePropagation::isContractorsCountEnoughForRoutingTablesPropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagation::trySendLinkBetweenInitiatorAndContractor() {

    setExpectationResponsesCounter(uint16_t(mTrustLinesManager->trustLines().size() - 1));

    if (mRequestCounter < kMaxRequestsCount) {
        sendLinkBetweenInitiatorAndContractor();
        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }
        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }
    return waitingForRoutingTablePropagationResponse(mConnectionTimeout);
}

void FromContractorToFirstLevelRoutingTablePropagation::sendLinkBetweenInitiatorAndContractor() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);

    // Node B.
    // Information about relationships with node A.
    vector<pair<const NodeUUID, const TrustLineDirection>> linkWithInitiator;
    linkWithInitiator.push_back(mLinkWithInitiator);
    firstLevelMessage->pushBack(
        mNodeUUID,
        linkWithInitiator
    );

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessage);

    // Sending information about relationships between nodes A and B to B1 level.
    // Node A is also at B1 level for node B. So excludes node A from receivers.
    // Node A is initiator, B is a contractor, so UUID of node A presents in 'mLinkWithInitiator'.
    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {
        if (contractorAndTrustLine.first == mLinkWithInitiator.first) {
            continue;
        }

        addMessage(
            message,
            contractorAndTrustLine.first
        );

    }
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagation::propagateSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        auto flagAndResult = checkContext();
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            if (flagAndResult.second != nullptr) {
                if (flagAndResult.second->resultType() == TransactionResult::ResultType::TransactionStateType) {
                    return flagAndResult.second;
                }
            }
            return trySendSecondLevelRoutingTable();
        }

    } else {
        return trySendSecondLevelRoutingTable();
    }
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagation::trySendSecondLevelRoutingTable() {

    setExpectationResponsesCounter(uint16_t(mTrustLinesManager->trustLines().size() - 1));

    if (mRequestCounter < kMaxRequestsCount) {
        sendSecondLevelRoutingTable();
        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }
        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }
    return waitingForRoutingTablePropagationResponse(mConnectionTimeout);
}

void FromContractorToFirstLevelRoutingTablePropagation::sendSecondLevelRoutingTable() {

    SecondLevelRoutingTableOutgoingMessage::Shared secondLevelMessage = make_shared<SecondLevelRoutingTableOutgoingMessage>(mNodeUUID);

    secondLevelMessage->pushBack(
        mNodeUUID,
        mSecondLevelRoutingTable
    );

    Message::Shared message = dynamic_pointer_cast<Message>(secondLevelMessage);


    // Sending received second level routing table from initiator to nodes of B1 level.
    // Node A is also at B1 level for node B. So excludes node A from receivers.
    // Node A is initiator, B is a contractor, so UUID of node A presents in 'mLinkWithInitiator'.
    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {

        if (contractorAndTrustLine.first == mLinkWithInitiator.first) {
            continue;
        }

        addMessage(
            message,
            contractorAndTrustLine.first
        );

    }
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagation::waitingForRoutingTablePropagationResponse(
    uint32_t connectionTimeout) {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            connectionTimeout
        )
    );
}

void FromContractorToFirstLevelRoutingTablePropagation::prepareToNextStep() {

    resetRequestsCounter();
    restoreStandardConnectionTimeout();
    resetExpectationResponsesCounter();
    clearContext();
}