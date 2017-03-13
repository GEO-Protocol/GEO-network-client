#include "FromContractorToFirstLevelRoutingTablesAcceptTransaction.h"

FromContractorToFirstLevelRoutingTablesAcceptTransaction::FromContractorToFirstLevelRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
    TrustLinesManager *trustLinesManager) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        relationshipsBetweenInitiatorAndContractor->senderUUID()
    ),
    mLinkBetweenInitiatorAndContractor(relationshipsBetweenInitiatorAndContractor),
    mTrustLinesManager(trustLinesManager) {}

FromContractorToFirstLevelRoutingTablesAcceptTransaction::FromContractorToFirstLevelRoutingTablesAcceptTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager)  :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer),
    mTrustLinesManager(trustLinesManager) {}

FirstLevelRoutingTableIncomingMessage::Shared FromContractorToFirstLevelRoutingTablesAcceptTransaction::message() const {

    return mLinkBetweenInitiatorAndContractor;
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablesAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveLinkBetweenInitiatorAndContractor();
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
            throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

void FromContractorToFirstLevelRoutingTablesAcceptTransaction::saveLinkBetweenInitiatorAndContractor() {

    string logLine;
    cout << "Message with relationships between initiator and contractor from contractor received " << endl;
    logLine = "SenderUUID:" + mLinkBetweenInitiatorAndContractor->senderUUID().stringUUID() + "::";
    cout << "Sender UUID -> " << mLinkBetweenInitiatorAndContractor->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;

    for (const auto &nodeAndRecords : mLinkBetweenInitiatorAndContractor->records()) {

        logLine += "ContractorUUID:" + nodeAndRecords.first.stringUUID() + "::";
        cout << "Contractor UUID -> " << nodeAndRecords.first.stringUUID() << endl;

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            logLine += "InitiatorUUID:" + neighborAndDirect.first.stringUUID() + "::";
            cout << "Initiator UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            logLine += "Direction:" + to_string(neighborAndDirect.second);
            cout << "Direction -> " << neighborAndDirect.second << endl;

        }

    }

    //mFileLogger->addLine(logLine.c_str());
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTablesAcceptTransaction::waitingForSecondLevelRoutingTableState() {

    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType},
            mConnectionTimeout
        )
    );
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
            secondLevelMessage
        );

        sendResponseToContractor(
            mContractorUUID,
            kResponseCodeSuccess
        );

        createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction();

        return finishTransaction();

    } else {
        throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void FromContractorToFirstLevelRoutingTablesAcceptTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    string logLine;
    cout << "Second level routing table message received " << endl;
    logLine = "SenderUUID:" + secondLevelMessage->senderUUID().stringUUID() + "::";
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

void FromContractorToFirstLevelRoutingTablesAcceptTransaction::sendResponseToContractor(
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


void FromContractorToFirstLevelRoutingTablesAcceptTransaction::createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction() {

    if (mLinkBetweenInitiatorAndContractor->records().size() > 1) {
        throw ConflictError("FromContractorToFirstLevelRoutingTablesAcceptTransaction::createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction: "
                                "Transaction must contains only one record in first level routing table message.");
    }

    BaseTransaction::Shared transaction = make_shared<FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction>(
        mNodeUUID,
        mContractorUUID,
        mLinkBetweenInitiatorAndContractor,
        mTrustLinesManager
    );

    launchSubsidiaryTransaction(transaction);
}