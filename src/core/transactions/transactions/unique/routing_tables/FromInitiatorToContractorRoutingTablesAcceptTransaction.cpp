#include "FromInitiatorToContractorRoutingTablesAcceptTransaction.h"

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID()
    ),
    mFirstLevelMessage(message) {}

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    BytesShared buffer) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer) {}

FirstLevelRoutingTableIncomingMessage::Shared FromInitiatorToContractorRoutingTablesAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveFirstLevelRoutingTable();
            sendResponseToContractor(
                const_cast<NodeUUID&> (mFirstLevelMessage->senderUUID()),
                200
            );
            increaseStepsCounter();
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

    cout << "First level routing table message received " << endl;
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : mFirstLevelMessage->mRecords) {
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::waitingForSecondLevelRoutingTableState() {

    auto state = TransactionState::waitForMessageTypes(
        {Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType},
        mConnectionTimeout
    );

    cout << "FromInitiatorToContractorRoutingTablesAcceptTransaction return state" << endl;
    cout << "Awakening timestamp in micros -> " << state->awakeningTimestamp() << endl;
    cout << "Waiting for such messages type as" << endl;
    for (const auto &i : state->acceptedMessagesTypes()) {
        cout << "-> " << i << endl;
    }

    return transactionResultFromState(
        state
    );
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
        saveSecondLevelRoutingTable(secondLevelMessage);
        sendResponseToContractor(
            const_cast<NodeUUID&> (secondLevelMessage->senderUUID()),
            200
        );
        return finishTransaction();

    } else {

        throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    cout << "Second level routing table message received " << endl;
    cout << "Sender UUID -> " << secondLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : secondLevelMessage->mRecords) {
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::sendResponseToContractor(
    NodeUUID &contractorUUID,
    uint16_t code) {

    Message *message = new RoutingTablesResponse(
        mNodeUUID,
        code
    );

    addMessage(
        Message::Shared(message),
        contractorUUID
    );
}