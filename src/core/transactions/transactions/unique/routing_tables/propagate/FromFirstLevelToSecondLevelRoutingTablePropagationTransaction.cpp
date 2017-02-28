#include "FromFirstLevelToSecondLevelRoutingTablePropagationTransaction.h"

FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::FromFirstLevelToSecondLevelRoutingTablePropagationTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
    TrustLinesManager *trustLinesManager)  :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        contractorUUID
    ),
    mLinkWithInitiator(relationshipsBetweenInitiatorAndContractor),
    mTrustLinesManager(trustLinesManager) {}

FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::FromFirstLevelToSecondLevelRoutingTablePropagationTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer
    ),
    mTrustLinesManager(trustLinesManager) {}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::run() {

    return propagateRelationshipsBetweenInitiatorAndContractor();
}

pair<bool, TransactionResult::SharedConst> FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::checkContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        for (const auto& responseMessage : mContext) {

            if (responseMessage->typeID() != Message::MessageTypeID::RoutingTablesResponseMessageType) {
                throw ConflictError("FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::checkContext: "
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
        // If some of the remote nodes are still offline, returns false flag and try send message again.
        return make_pair(
            false,
            TransactionResult::Shared(nullptr)
        );
    }
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::propagateRelationshipsBetweenInitiatorAndContractor() {

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

bool FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::isContractorsCountEnoughForRoutingTablesPropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::trySendLinkBetweenInitiatorAndContractor() {

    setExpectationResponsesCounter(
        uint16_t(mTrustLinesManager->trustLines().size() - 1)
    );

    for (const auto &nodeAndTrustLine : mTrustLinesManager->trustLines()) {

        if (mLinkWithInitiator->mRecords.begin()->second.begin()->first == nodeAndTrustLine.first) {
            setExpectationResponsesCounter(
                uint16_t(mTrustLinesManager->trustLines().size() - 2)
            );
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

void FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::sendLinkBetweenInitiatorAndContractor() {

    NodeUUID initiator;
    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(mNodeUUID);

    for (const auto& contractorAndRecord : mLinkWithInitiator->mRecords) {

        for (const auto &initiatorAndDirect : contractorAndRecord.second) {

            initiator = initiatorAndDirect.first;

            vector<pair<const NodeUUID, const TrustLineDirection>> record;
            record.push_back(
                make_pair(
                    initiator,
                    initiatorAndDirect.second
                )
            );

            firstLevelMessage->pushBack(
                contractorAndRecord.first,
                record
            );

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

        addMessage(
            message,
            contractorAndTrustLine.first
        );

    }
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablePropagationTransaction::waitingForRoutingTablePropagationResponse() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            mConnectionTimeout
        )
    );
}
