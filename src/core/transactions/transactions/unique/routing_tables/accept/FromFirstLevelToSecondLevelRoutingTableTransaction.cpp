#include "FromFirstLevelToSecondLevelRoutingTableTransaction.h"

FromFirstLevelToSecondLevelRoutingTableTransaction::FromFirstLevelToSecondLevelRoutingTableTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID()
    ),
    mFirstLevelMessage(message) {}

FromFirstLevelToSecondLevelRoutingTableTransaction::FromFirstLevelToSecondLevelRoutingTableTransaction(
    BytesShared buffer) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer) {}

FirstLevelRoutingTableIncomingMessage::Shared FromFirstLevelToSecondLevelRoutingTableTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTableTransaction::run() {

    saveFirstLevelRoutingTable();

    sendResponseToContractor(
        mFirstLevelMessage->senderUUID(),
        kResponseCodeSuccess
    );

    return finishTransaction();
}

void FromFirstLevelToSecondLevelRoutingTableTransaction::saveFirstLevelRoutingTable() {

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

void FromFirstLevelToSecondLevelRoutingTableTransaction::sendResponseToContractor(
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
