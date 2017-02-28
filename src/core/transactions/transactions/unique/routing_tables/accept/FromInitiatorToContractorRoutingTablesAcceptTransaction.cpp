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
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveFirstLevelRoutingTable() {

    cout << "From initiator first level routing table message received " << endl;
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;

    for (const auto &nodeAndRecords : mFirstLevelMessage->mRecords) {

        cout << "Initiator UUID -> " << nodeAndRecords.first.stringUUID() << endl;

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;

        }
    }
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

        saveSecondLevelRoutingTable(
            secondLevelMessage
        );
        sendResponseToContractor(
            secondLevelMessage->senderUUID(),
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
        mFirstLevelMessage->senderUUID(),
        direction
    );

    BaseTransaction::Shared transaction = make_shared<FromContractorToFirstLevelRoutingTablePropagationTransaction>(
      mNodeUUID,
      mContractorUUID,
      relationshipsBetweenInitiatorAndContractor,
      secondLevelMessage,
      mTrustLinesManager
    );

    launchSubsidiaryTransaction(transaction);
}
