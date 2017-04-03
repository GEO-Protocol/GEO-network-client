#include "FromInitiatorToContractorRoutingTablesPropagationTransaction.h"

FromInitiatorToContractorRoutingTablesPropagationTransaction::FromInitiatorToContractorRoutingTablesPropagationTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler){

    mContractorsUUIDs.push_back(
        contractorUUID);
}

FromInitiatorToContractorRoutingTablesPropagationTransaction::FromInitiatorToContractorRoutingTablesPropagationTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        buffer),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler) {}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            auto firstLevelPropagationResult = propagateFirstLevelRoutingTable();

            if (firstLevelPropagationResult->resultType() == TransactionResult::ResultType::TransactionStateType) {
                return firstLevelPropagationResult;

            } else {
                prepareToNextStep();
                mStep = RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep;
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
        throw ConflictError("FromInitiatorToContractorRoutingTablesPropagationTransaction::checkContext: "
                                "Unexpected context size.");
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

        if (*mContractorsUUIDs.begin() == contractorAndTrustLine.first) {
            continue;
        }

        neighborsAndDirections.push_back(
            make_pair(
                contractorAndTrustLine.first,
                mTrustLinesManager->trustLineReadOnly(contractorAndTrustLine.first)->direction()));

    }

    firstLevelMessage->pushBack(
        mNodeUUID,
        neighborsAndDirections);

    sendMessage(
        *mContractorsUUIDs.begin(),
        dynamic_pointer_cast<Message>(firstLevelMessage));
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::propagateSecondLevelRoutingTable() {

    if (mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsWithDirectionsMapSourceKey().empty()) {
        return finishTransaction();
    }

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

    for (auto& nodeAndNeighborsAndDirections : mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsWithDirectionsMapSourceKey()) {

        if (*mContractorsUUIDs.begin() == nodeAndNeighborsAndDirections.first) {
            continue;
        }

        secondLevelMessage->pushBack(
            nodeAndNeighborsAndDirections.first,
            nodeAndNeighborsAndDirections.second);

    }

    sendMessage(
        *mContractorsUUIDs.begin(),
        dynamic_pointer_cast<Message>(secondLevelMessage));
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesPropagationTransaction::waitingForRoutingTablePropagationResponse() {


    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::RoutingTablesResponseMessageType},
            mConnectionTimeout));
}

void FromInitiatorToContractorRoutingTablesPropagationTransaction::prepareToNextStep() {

    resetRequestsCounter();
    restoreStandardConnectionTimeout();
    resetExpectationResponsesCounter();
    clearContext();
}
