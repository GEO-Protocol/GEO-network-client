#include "FromContractorToFirstLevelRoutingTablePropagationTransaction.h"

FromContractorToFirstLevelRoutingTablePropagationTransaction::FromContractorToFirstLevelRoutingTablePropagationTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const pair<const NodeUUID, const TrustLineDirection> &relationshipsBetweenInitiatorAndContractor,
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelRoutingTableFromInitiator,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID
    ),
    mLinkWithInitiator(relationshipsBetweenInitiatorAndContractor),
    mSecondLevelRoutingTableFromInitiator(secondLevelRoutingTableFromInitiator),
    mTrustLinesManager(trustLinesManager) {}

FromContractorToFirstLevelRoutingTablePropagationTransaction::FromContractorToFirstLevelRoutingTablePropagationTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer
    ),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagationTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {

            auto initiatorAndContractorLinkPropagationResult = propagateRelationshipsBetweenInitiatorAndContractor();

            if (initiatorAndContractorLinkPropagationResult->resultType() == TransactionResult::ResultType::TransactionStateType) {
                return initiatorAndContractorLinkPropagationResult;


            } else {
                prepareToNextStep();
                increaseStepsCounter();
            }
        }

        case RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep: {
            return propagateSecondLevelRoutingTable();
        }

        default: {
            throw ConflictError("FromContractorToFirstLevelRoutingTablePropagationTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

pair<bool, TransactionResult::SharedConst> FromContractorToFirstLevelRoutingTablePropagationTransaction::checkContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        for (const auto& responseMessage : mContext) {

            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("FromContractorToFirstLevelRoutingTablePropagationTransaction::checkContext: "
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
        return make_pair(
            false,
            TransactionResult::Shared(nullptr)
        );
    }
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagationTransaction::propagateRelationshipsBetweenInitiatorAndContractor() {

    if (!isContractorsCountEnoughForRoutingTablesPropagation()) {
        return finishTransaction();
    }

    if (!mContext.empty()) {
        auto flagAndResult = checkContext();
        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            return trySendLinkBetweenInitiatorAndContractor();
        }

    } else {
        return trySendLinkBetweenInitiatorAndContractor();
    }
}

bool FromContractorToFirstLevelRoutingTablePropagationTransaction::isContractorsCountEnoughForRoutingTablesPropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagationTransaction::trySendLinkBetweenInitiatorAndContractor() {

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

void FromContractorToFirstLevelRoutingTablePropagationTransaction::sendLinkBetweenInitiatorAndContractor() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);


    vector<pair<const NodeUUID, const TrustLineDirection>> linkWithInitiator;
    linkWithInitiator.push_back(
        mLinkWithInitiator
    );

    firstLevelMessage->pushBack(
        mNodeUUID,
        linkWithInitiator
    );

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessage);

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

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagationTransaction::propagateSecondLevelRoutingTable() {

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

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagationTransaction::trySendSecondLevelRoutingTable() {

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

void FromContractorToFirstLevelRoutingTablePropagationTransaction::sendSecondLevelRoutingTable() {

    SecondLevelRoutingTableOutgoingMessage::Shared secondLevelMessage = make_shared<SecondLevelRoutingTableOutgoingMessage>(mNodeUUID);

    for (const auto &nodeAndRecord : mSecondLevelRoutingTableFromInitiator->mRecords) {

        vector<pair<const NodeUUID, const TrustLineDirection>> neighborAndDirection;

        for (const auto &neighborAndDirect : nodeAndRecord.second) {

            neighborAndDirection.push_back(
                make_pair(
                    neighborAndDirect.first,
                    neighborAndDirect.second
                )
            );
        }

        secondLevelMessage->pushBack(
            nodeAndRecord.first,
            neighborAndDirection
        );

    }

    Message::Shared message = dynamic_pointer_cast<Message>(secondLevelMessage);

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

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablePropagationTransaction::waitingForRoutingTablePropagationResponse(
    uint32_t connectionTimeout) {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            connectionTimeout
        )
    );
}

void FromContractorToFirstLevelRoutingTablePropagationTransaction::prepareToNextStep() {

    resetRequestsCounter();
    restoreStandardConnectionTimeout();
    resetExpectationResponsesCounter();
    clearContext();
}