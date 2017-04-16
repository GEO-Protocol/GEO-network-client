#include "PropagateRoutingTablesUpdatesTransaction.h"

PropagateRoutingTablesUpdatesTransaction::PropagateRoutingTablesUpdatesTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &initiatorUUID,
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction,
    const NodeUUID &recipientUUID,
    const RoutingTableUpdateOutgoingMessage::UpdatingStep updatingStep):

    BaseTransaction(
        BaseTransaction::TransactionType::PropagateRoutingTablesUpdatesTransactionType,
        nodeUUID),
    mInitiatorUUID(initiatorUUID),
    mContractorUUID(contractorUUID),
    mRecipientUUID(recipientUUID) {

    mDirection = direction;
    mUpdatingStep = updatingStep;
}

TransactionResult::SharedConst PropagateRoutingTablesUpdatesTransaction::run() {

    return propagateRoutingTablesUpdates();
}

pair<bool, TransactionResult::SharedConst> PropagateRoutingTablesUpdatesTransaction::checkContext() {

    if (mkExpectationResponsesCount == mContext.size()) {

        Message::Shared responseMessage = *mContext.begin();

        if (responseMessage->typeID() != Message::MessageTypeID::ResponseMessageType) {
            throw ConflictError("PropagateRoutingTablesUpdatesTransaction::checkContext: "
                                    "Illegal message type in context.");
        }

        Response::Shared response = static_pointer_cast<Response>(responseMessage);
        if (response->code() != kResponseCodeSuccess) {
            return make_pair(
                false,
                TransactionResult::Shared(
                    nullptr));
        }

        return make_pair(
            true,
            transactionResultFromMessage(
                make_shared<MessageResult>(
                    response->senderUUID(),
                    response->transactionUUID(),
                    kResponseCodeSuccess)));

    } else {
        throw ConflictError("PropagateRoutingTablesUpdatesTransaction::checkContext: "
                                "Unexpected context size.");
    }
}

TransactionResult::SharedConst PropagateRoutingTablesUpdatesTransaction::propagateRoutingTablesUpdates() {

    if (!mContext.empty()) {
        auto flagAndResult = checkContext();

        if (flagAndResult.first) {
            return flagAndResult.second;

        } else {
            return trySendMessageToRecipient();
        }

    } else {
        return trySendMessageToRecipient();
    }
}

TransactionResult::SharedConst PropagateRoutingTablesUpdatesTransaction::trySendMessageToRecipient() {

    setExpectationResponsesCounter(1);

    if (mRequestCounter < kMaxRequestsCount) {
        sendMessageToRecipient();

        if (mRequestCounter < 0) {
            progressConnectionTimeout();
        }

        increaseRequestsCounter();

    } else {
        return finishTransaction();
    }

    return waitingForResponseFromRecipient();
}

void PropagateRoutingTablesUpdatesTransaction::sendMessageToRecipient() {

    sendMessage<RoutingTableUpdateOutgoingMessage>(
        mRecipientUUID,
        mNodeUUID,
        mTransactionUUID,
        mInitiatorUUID,
        mContractorUUID,
        mDirection,
        mUpdatingStep);
}

TransactionResult::SharedConst PropagateRoutingTablesUpdatesTransaction::waitingForResponseFromRecipient() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::ResponseMessageType},
            mConnectionTimeout));
}

void PropagateRoutingTablesUpdatesTransaction::increaseRequestsCounter() {

     mRequestCounter += 1;
}

void PropagateRoutingTablesUpdatesTransaction::progressConnectionTimeout() {

    mConnectionTimeout *= kConnectionProgression;
}