#include "FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction.h"

FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message,
    StorageHandler *storageHandler) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID()),
    mFirstLevelMessage(message),
    mStorageHandler(storageHandler) {}

FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
    BytesShared buffer,
    StorageHandler *storageHandler) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer),
    mStorageHandler(storageHandler) {}

FirstLevelRoutingTableIncomingMessage::Shared FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::run() {

    saveFirstLevelRoutingTable();

    sendResponseToContractor(
        mContractorUUID,
        kResponseCodeSuccess);

    return finishTransaction();
}

void FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::saveFirstLevelRoutingTable() {

    for (const auto &nodeAndRecords : mFirstLevelMessage->records()) {

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(
                nodeAndRecords.first,
                neighborAndDirect.first,
                neighborAndDirect.second);

        }
    }

}

void FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::sendResponseToContractor(
    const NodeUUID &contractorUUID,
    const uint16_t code) {

    sendMessage<RoutingTablesResponse>(
        contractorUUID,
        mNodeUUID,
        code);
}
