#include "FromInitiatorToContractorRoutingTablesAcceptTransaction.h"

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID()
    ),
    mFirstLevelMessage(message),
    mTrustLinesManager(trustLinesManager) {}

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer),
    mTrustLinesManager(trustLinesManager) {}

FirstLevelRoutingTableIncomingMessage::Shared FromInitiatorToContractorRoutingTablesAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveFirstLevelRoutingTable();
            sendResponseToContractor(
                mContractorUUID,
                kResponseCodeSuccess
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

    string logLine;
    cout << "From initiator first level routing table message received " << endl;
    logLine = "SenderUUID:" + mFirstLevelMessage->senderUUID().stringUUID() + "::";
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;

    for (const auto &nodeAndRecords : mFirstLevelMessage->records()) {

        cout << "Initiator UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        logLine += "InitiatorUUID:" + nodeAndRecords.first.stringUUID() + "::";

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            logLine += "NeighborUUID:" + neighborAndDirect.first.stringUUID() + "::";
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            logLine += "Direction:" + to_string(neighborAndDirect.second);
            cout << "Direction -> " << neighborAndDirect.second << endl;

        }
    }

    //mFileLogger->addLine(logLine.c_str());
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::waitingForSecondLevelRoutingTableState() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType},
            mConnectionTimeout
        )
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

        if (mContractorUUID != secondLevelMessage->senderUUID()) {
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Sender UUID from second level routing table message differs from sender UUID from first level routing table message.");
        }

        mContractorUUID = secondLevelMessage->senderUUID();

        saveSecondLevelRoutingTable(
            secondLevelMessage
        );

        sendResponseToContractor(
            mContractorUUID,
            kResponseCodeSuccess
        );

        createFromContractorToFirstLevelRoutingTablesPropagationTransaction(
            secondLevelMessage
        );

        return finishTransaction();

    } else {
        throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    string logLine;
    cout << "From initiator second level routing table message received " << endl;
    logLine = "SenderUUID:" + mFirstLevelMessage->senderUUID().stringUUID() + "::";
    cout << "Sender UUID -> " << secondLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;

    for (const auto &nodeAndRecords : secondLevelMessage->records()) {

        logLine += "NodeUUID:" + nodeAndRecords.first.stringUUID() + "::";
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            logLine += "NeighborUUID:" + neighborAndDirect.first.stringUUID() + "::";
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            logLine += "Direction:" + to_string(neighborAndDirect.second);
            cout << "Direction -> " << neighborAndDirect.second << endl;

        }

    }

    //mFileLogger->addLine(logLine.c_str());
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::sendResponseToContractor(
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

void FromInitiatorToContractorRoutingTablesAcceptTransaction::createFromContractorToFirstLevelRoutingTablesPropagationTransaction(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    const TrustLineDirection direction = TrustLineDirection::Incoming;
    auto relationshipsBetweenInitiatorAndContractor = make_pair(
        mContractorUUID,
        direction
    );

    BaseTransaction::Shared transaction = make_shared<FromContractorToFirstLevelRoutingTablesPropagationTransaction>(
      mNodeUUID,
      relationshipsBetweenInitiatorAndContractor,
      secondLevelMessage,
      mTrustLinesManager
    );

    launchSubsidiaryTransaction(transaction);
}