#include "FromContractorToFirstLevelRoutingTableAcceptTransaction.h"

FromContractorToFirstLevelRoutingTableAcceptTransaction::FromContractorToFirstLevelRoutingTableAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID()
    ),
    mFirstLevelMessage(message) {}

FromContractorToFirstLevelRoutingTableAcceptTransaction::FromContractorToFirstLevelRoutingTableAcceptTransaction(
    BytesShared buffer)  :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer) {}

FirstLevelRoutingTableIncomingMessage::Shared FromContractorToFirstLevelRoutingTableAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTableAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveLinkBetweenInitiatorAndContractor();
            sendResponseToContractor(
                mFirstLevelMessage->senderUUID(),
                kResponseCodeSuccess
            );
            increaseStepsCounter();
            return waitingForSecondLevelRoutingTableState();
        }

        case RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep: {
            return checkIncomingMessageForSecondLevelRoutingTable();
        }

        default: {
            throw ConflictError("FromContractorToFirstLevelRoutingTableAcceptTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

void FromContractorToFirstLevelRoutingTableAcceptTransaction::saveLinkBetweenInitiatorAndContractor() {

    cout << "Message with relationships between initiator and contractor received " << endl;
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : mFirstLevelMessage->mRecords) {
        cout << "Contractor UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Initiator UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTableAcceptTransaction::waitingForSecondLevelRoutingTableState() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType},
            mConnectionTimeout
        )
    );
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTableAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage;

        for (const auto& incomingMessage : mContext) {

            if (incomingMessage->typeID() == Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType) {
                secondLevelMessage = static_pointer_cast<SecondLevelRoutingTableIncomingMessage>(incomingMessage);
            }
        }

        if (secondLevelMessage == nullptr) {
            throw ConflictError("FromContractorToFirstLevelRoutingTableAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Can not cast to SecondLevelRoutingTableIncomingMessage.");
        }

        saveSecondLevelRoutingTable(secondLevelMessage);
        sendResponseToContractor(
            secondLevelMessage->senderUUID(),
            kResponseCodeSuccess
        );

        return finishTransaction();

    } else {
        throw ConflictError("FromContractorToFirstLevelRoutingTableAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void FromContractorToFirstLevelRoutingTableAcceptTransaction::saveSecondLevelRoutingTable(
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

void FromContractorToFirstLevelRoutingTableAcceptTransaction::sendResponseToContractor(
    const NodeUUID &contractorUUID,
    const uint16_t code) {

    Message *message = new RoutingTablesResponse(
        mNodeUUID,
        code
    );

    addMessage(
        Message::Shared(message),
        contractorUUID
    );
}
