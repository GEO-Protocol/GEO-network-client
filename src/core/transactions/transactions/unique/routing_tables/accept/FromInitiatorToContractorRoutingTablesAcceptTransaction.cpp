#include "FromInitiatorToContractorRoutingTablesAcceptTransaction.h"

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Logger *logger) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID(),
        logger),
    mFirstLevelMessage(message),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler) {}

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
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

FirstLevelRoutingTableIncomingMessage::Shared FromInitiatorToContractorRoutingTablesAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveFirstLevelRoutingTable();
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
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveFirstLevelRoutingTable() {

    info() << "From initiator first level routing table message received";
    info() << "Sender UUID: " + mFirstLevelMessage->senderUUID().stringUUID();
    info() << "Routing table";

    for (const auto &nodeAndRecords : mFirstLevelMessage->records()) {

        info() << "Initiator UUID: " + nodeAndRecords.first.stringUUID();

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            info() << "Neighbor UUID: " + neighborAndDirect.first.stringUUID();
            info() << "Direction: " + to_string(neighborAndDirect.second);

            //try {
                mStorageHandler->routingTablesHandler()->routingTable2Level()->saveRecord(
                    mFirstLevelMessage->senderUUID(),
                    neighborAndDirect.first,
                    neighborAndDirect.second);

            /*} catch (Exception&) {
                error() << "Except when saving first level routing table from initiator at contractor's side";
            }*/

        }
    }
    mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::waitingForSecondLevelRoutingTableState() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType},
            mConnectionTimeout));
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage;

        for (const auto& incomingMessage : mContext) {

            if (incomingMessage->typeID() == Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType) {
                secondLevelMessage = static_pointer_cast<SecondLevelRoutingTableIncomingMessage>(incomingMessage);
            }

        }

        if (secondLevelMessage == nullptr) {
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Can not cast to SecondLevelRoutingTableIncomingMessage.");
        }

        if (mContractorUUID != secondLevelMessage->senderUUID()) {
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Sender UUID from second level routing table message differs from sender UUID from first level routing table message.");
        }

        mContractorUUID = secondLevelMessage->senderUUID();

        saveSecondLevelRoutingTable(
            secondLevelMessage);

        sendResponseToContractor(
            mContractorUUID,
            kResponseCodeSuccess);

        createFromContractorToFirstLevelRoutingTablesPropagationTransaction(
            secondLevelMessage);

        return finishTransaction();

    } else {
        throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    info() << "From initiator second level routing table message received";
    info() << "Sender UUID: " + mFirstLevelMessage->senderUUID().stringUUID();
    info() << "Routing table";

    for (const auto &nodeAndRecords : secondLevelMessage->records()) {

        info() << "Node UUID: " + nodeAndRecords.first.stringUUID();

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            info() << "Neighbor UUID: " + neighborAndDirect.first.stringUUID();
            info() << "Direction: " + to_string(neighborAndDirect.second);

            try {
                mStorageHandler->routingTablesHandler()->routingTable3Level()->saveRecord(
                    nodeAndRecords.first,
                    neighborAndDirect.first,
                    neighborAndDirect.second);

            } catch (Exception& e) {
                error() << "Except when saving second level routing table from initiator at contractor's side";
            }

        }

    }
    mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();

}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::sendResponseToContractor(
    const NodeUUID &contractorUUID,
    const uint16_t code) {

    sendMessage<RoutingTablesResponse>(
        contractorUUID,
        mNodeUUID,
        code);
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::createFromContractorToFirstLevelRoutingTablesPropagationTransaction(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    auto direction = mTrustLinesManager->trustLineReadOnly(mContractorUUID)->direction();
    auto relationshipsBetweenInitiatorAndContractor = make_pair(
        mContractorUUID,
        direction);

    BaseTransaction::Shared transaction = make_shared<FromContractorToFirstLevelRoutingTablesPropagationTransaction>(
      mNodeUUID,
      relationshipsBetweenInitiatorAndContractor,
      secondLevelMessage,
      mTrustLinesManager);

    launchSubsidiaryTransaction(
        transaction);
}

const string FromInitiatorToContractorRoutingTablesAcceptTransaction::logHeader() const {

    stringstream s;
    s << "[FromInitiatorToContractorRoutingTablesAcceptTransaction]";

    return s.str();
}
