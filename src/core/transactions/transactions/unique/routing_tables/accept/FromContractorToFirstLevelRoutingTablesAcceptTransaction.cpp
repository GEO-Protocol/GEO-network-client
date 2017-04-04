#include "FromContractorToFirstLevelRoutingTablesAcceptTransaction.h"

FromContractorToFirstLevelRoutingTablesAcceptTransaction::FromContractorToFirstLevelRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Logger *logger) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        relationshipsBetweenInitiatorAndContractor->senderUUID(),
        logger),
    mLinkBetweenInitiatorAndContractor(relationshipsBetweenInitiatorAndContractor),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler) {}

FromContractorToFirstLevelRoutingTablesAcceptTransaction::FromContractorToFirstLevelRoutingTablesAcceptTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Logger *logger) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer,
        logger),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler) {}

FirstLevelRoutingTableIncomingMessage::Shared FromContractorToFirstLevelRoutingTablesAcceptTransaction::message() const {

    return mLinkBetweenInitiatorAndContractor;
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablesAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveLinkBetweenInitiatorAndContractor();
            sendResponseToContractor(
                mContractorUUID,
                kResponseCodeSuccess);
            mStep = RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep;

            return waitingForSecondLevelRoutingTableState();
        }

        case RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep: {
            return checkIncomingMessageForSecondLevelRoutingTable();
        }

        default: {
            throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

void FromContractorToFirstLevelRoutingTablesAcceptTransaction::saveLinkBetweenInitiatorAndContractor() {

    info() << "Message with relationships between initiator and contractor from contractor received";
    info() << "Sender UUID: " + mLinkBetweenInitiatorAndContractor->senderUUID().stringUUID();
    info() << "Routing table";

    for (const auto &nodeAndRecords : mLinkBetweenInitiatorAndContractor->records()) {

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            info() << "Contractor UUID: " + nodeAndRecords.first.stringUUID();
            info() << "Initiator UUID: " + neighborAndDirect.first.stringUUID();
            info() << "Direction UUID: " + to_string(neighborAndDirect.second);

            mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(
                nodeAndRecords.first,
                neighborAndDirect.first,
                neighborAndDirect.second);

        }

    }

}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablesAcceptTransaction::waitingForSecondLevelRoutingTableState() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType},
            mConnectionTimeout));
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage;

        for (const auto& incomingMessage : mContext) {

            if (incomingMessage->typeID() == Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType) {
                secondLevelMessage = static_pointer_cast<SecondLevelRoutingTableIncomingMessage>(incomingMessage);
            }
        }

        if (secondLevelMessage == nullptr) {
            throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Can not cast to SecondLevelRoutingTableIncomingMessage.");
        }

        if (mContractorUUID != secondLevelMessage->senderUUID()) {
            throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Sender UUID from second level routing table message differs from sender UUID from first level routing table message.");
        }

        saveSecondLevelRoutingTable(
            secondLevelMessage);

        sendResponseToContractor(
            mContractorUUID,
            kResponseCodeSuccess);

        createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction();

        return finishTransaction();

    } else {
        throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void FromContractorToFirstLevelRoutingTablesAcceptTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    info() << "Second level routing table message from contractor received";
    info() << "Sender UUID: " + secondLevelMessage->senderUUID().stringUUID();
    info() << "Routing table";

    for (const auto &nodeAndRecords : secondLevelMessage->records()) {

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            info() << "Node UUID: " + nodeAndRecords.first.stringUUID();
            info() << "Neighbor UUID: " + neighborAndDirect.first.stringUUID();
            info() << "Direction: " + to_string(neighborAndDirect.second);

            mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(
                nodeAndRecords.first,
                neighborAndDirect.first,
                neighborAndDirect.second);

        }

    }

}

void FromContractorToFirstLevelRoutingTablesAcceptTransaction::sendResponseToContractor(
    const NodeUUID &contractorUUID,
    const uint16_t code) {

    sendMessage<RoutingTablesResponse>(
        contractorUUID,
        mNodeUUID,
        code);
}


void FromContractorToFirstLevelRoutingTablesAcceptTransaction::createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction() {

    if (mLinkBetweenInitiatorAndContractor->records().size() > 1) {
        throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction: "
                                "Transaction must contains only one record in first level routing table message.");
    }

    BaseTransaction::Shared transaction = make_shared<FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction>(
        mNodeUUID,
        mContractorUUID,
        mLinkBetweenInitiatorAndContractor,
        mTrustLinesManager);

    launchSubsidiaryTransaction(
        transaction);
}

const string FromContractorToFirstLevelRoutingTablesAcceptTransaction::logHeader() const {

    stringstream s;
    s << "[FromContractorToFirstLevelRoutingTablesAcceptTransaction]";

    return s.str();
}
