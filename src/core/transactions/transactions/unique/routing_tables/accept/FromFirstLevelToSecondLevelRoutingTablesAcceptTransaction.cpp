#include "FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction.h"

FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID()
    ),
    mFirstLevelMessage(message) {}

FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
    BytesShared buffer) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer) {}

FirstLevelRoutingTableIncomingMessage::Shared FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::run() {

    saveFirstLevelRoutingTable();

    sendResponseToContractor(
        mContractorUUID,
        kResponseCodeSuccess
    );

    return finishTransaction();
}

void FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::saveFirstLevelRoutingTable() {

    cout << "Link between initiator and contractor received " << endl;
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;

    for (const auto &nodeAndRecords : mFirstLevelMessage->records()) {

        cout << "Contractor UUID -> " << nodeAndRecords.first.stringUUID() << endl;

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            cout << "Initiator UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;

        }
    }
}

void FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::sendResponseToContractor(
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
