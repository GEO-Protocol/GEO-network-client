#include "FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction.h"

FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
    const NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message,
    StorageHandler *storageHandler,
    Logger *logger) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID(),
        logger),
    mFirstLevelMessage(message),
    mStorageHandler(storageHandler) {}

FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
    BytesShared buffer,
    StorageHandler *storageHandler,
    Logger *logger) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        buffer,
        logger),
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

    info() << "Link between initiator and contractor from first level received";
    info() << "Sender UUID: " + mFirstLevelMessage->senderUUID().stringUUID();
    info() << "Routing table";

    for (const auto &nodeAndRecords : mFirstLevelMessage->records()) {

        for (const auto &neighborAndDirect : nodeAndRecords.second) {

            info() << "Contractor UUID: " + nodeAndRecords.first.stringUUID();
            info() << "Initiator UUID: " + neighborAndDirect.first.stringUUID();
            info() << "Direction UUID: " + to_string(neighborAndDirect.second);


            try {
                mStorageHandler->routingTablesHandler()->routingTable3Level()->saveRecord(
                    nodeAndRecords.first,
                    neighborAndDirect.first,
                    neighborAndDirect.second);

            } catch (Exception&) {
                error() << "Except when saving link between initiator and contractor from first level at second level side";
            }

        }
    }
    mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();

}

void FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::sendResponseToContractor(
    const NodeUUID &contractorUUID,
    const uint16_t code) {

    sendMessage<RoutingTablesResponse>(
        contractorUUID,
        mNodeUUID,
        code);
}

const string FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction::logHeader() const {

    stringstream s;
    s << "[FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction]";

    return s.str();
}
