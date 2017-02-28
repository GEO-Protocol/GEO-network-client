#include "FromContractorToFirstLevelRoutingTableAcceptTransaction.h"

FromContractorToFirstLevelRoutingTableAcceptTransaction::FromContractorToFirstLevelRoutingTableAcceptTransaction(
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

FromContractorToFirstLevelRoutingTableAcceptTransaction::FromContractorToFirstLevelRoutingTableAcceptTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLinesManager)  :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer),
    mTrustLinesManager(trustLinesManager) {}

FirstLevelRoutingTableIncomingMessage::Shared FromContractorToFirstLevelRoutingTableAcceptTransaction::message() const {

    return mLinkBetweenInitiatorAndContractor;
}

TransactionResult::SharedConst FromContractorToFirstLevelRoutingTableAcceptTransaction::run() {

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveLinkBetweenInitiatorAndContractor();
            sendResponseToContractor(
                mLinkBetweenInitiatorAndContractor->senderUUID(),
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

    cout << "Message with relationships between initiator and contractor from contractor received " << endl;
    cout << "Sender UUID -> " << mLinkBetweenInitiatorAndContractor->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;

    for (const auto &nodeAndRecords : mLinkBetweenInitiatorAndContractor->mRecords) {

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

        saveSecondLevelRoutingTable(
            secondLevelMessage
        );

        sendResponseToContractor(
            secondLevelMessage->senderUUID(),
            kResponseCodeSuccess
        );

        createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction();

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


void FromContractorToFirstLevelRoutingTableAcceptTransaction::createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction() {

    if (mLinkBetweenInitiatorAndContractor->mRecords.size() > 1) {
        throw ConflictError("FromContractorToFirstLevelRoutingTableAcceptTransaction::createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction: "
                                "Transaction must contains only one record in first level routing table message.");
    }

    BaseTransaction::Shared transaction = make_shared<FromFirstLevelToSecondLevelRoutingTablePropagationTransaction>(
        mNodeUUID,
        mContractorUUID,
        mLinkBetweenInitiatorAndContractor,
        mTrustLinesManager
    );

    launchSubsidiaryTransaction(transaction);
}