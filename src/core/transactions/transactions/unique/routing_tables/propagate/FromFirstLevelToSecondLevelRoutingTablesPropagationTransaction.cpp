#include "FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction.h"

FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
    TrustLinesManager *trustLinesManager,
    Logger *logger)  :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID,
        logger),
    mLinkWithInitiator(relationshipsBetweenInitiatorAndContractor),
    mTrustLinesManager(trustLinesManager) {}

FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager,
    Logger *logger) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer,
        logger),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::run() {

    return propagateRelationshipsBetweenInitiatorAndContractor();
}

pair<bool, TransactionResult::SharedConst> FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::checkContext() {

    if (mkExpectationResponsesCount == mContext.size()) {
        for (const auto& responseMessage : mContext) {

            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::checkContext: "
                                        "Illegal message type in context.");
            }

            RoutingTablesResponse::Shared response = static_pointer_cast<RoutingTablesResponse>(responseMessage);
            if (response->code() != kResponseCodeSuccess) {
                return make_pair(
                    false,
                    TransactionResult::Shared(nullptr));
            }

        }

        return make_pair(
            true,
            transactionResultFromMessage(
                make_shared<MessageResult>(
                    *mContractorsUUIDs.begin(),
                    mTransactionUUID,
                    kResponseCodeSuccess)));

    } else {
        return make_pair(
            false,
            TransactionResult::Shared(nullptr));
    }
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::propagateRelationshipsBetweenInitiatorAndContractor() {

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

bool FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::isContractorsCountEnoughForRoutingTablesPropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::trySendLinkBetweenInitiatorAndContractor() {

    setExpectationResponsesCounter(uint16_t(mTrustLinesManager->trustLines().size() - 1));

    for (const auto &nodeAndTrustLine : mTrustLinesManager->trustLines()) {

        if (mLinkWithInitiator->records().begin()->second.begin()->first == nodeAndTrustLine.first) {
            setExpectationResponsesCounter(uint16_t(mTrustLinesManager->trustLines().size() - 2));
            break;
        }
    }

    if (mRequestCounter < kMaxRequestsCount) {
        sendLinkBetweenInitiatorAndContractor();

        if (mRequestCounter > 0) {
            progressConnectionTimeout();
        }

        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }

    return waitingForRoutingTablePropagationResponse();
}

void FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::sendLinkBetweenInitiatorAndContractor() {

    NodeUUID initiator;
    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);
    firstLevelMessage->setPropagationStep(
        RoutingTablesMessage::PropagationStep::FromFirstLevelToSecondLevel);

    for (const auto& contractorAndRecord : mLinkWithInitiator->records()) {

        for (const auto &initiatorAndDirect : contractorAndRecord.second) {

            initiator = initiatorAndDirect.first;

            vector<pair<const NodeUUID, const TrustLineDirection>> record;
            record.push_back(
                make_pair(
                    initiator,
                    initiatorAndDirect.second));

            firstLevelMessage->pushBack(
                contractorAndRecord.first,
                record);

        }

    }

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessage);

    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {

        if (contractorAndTrustLine.first == mContractorUUID) {
            continue;
        }

        if (contractorAndTrustLine.first == initiator) {
            continue;
        }

        mContractorsUUIDs.push_back(
            contractorAndTrustLine.first);

        sendMessage(
            contractorAndTrustLine.first,
            message);

    }
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction::waitingForRoutingTablePropagationResponse() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            mConnectionTimeout));
}
