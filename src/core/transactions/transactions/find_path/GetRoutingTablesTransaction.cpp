#include "GetRoutingTablesTransaction.h"

GetRoutingTablesTransaction::GetRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    RequestRoutingTablesMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger):

    BaseTransaction(
        BaseTransaction::TransactionType::GetRoutingTablesTransactionType,
        nodeUUID,
        logger),

    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler){}

RequestRoutingTablesMessage::Shared GetRoutingTablesTransaction::message() const {

    return  mMessage;
}

TransactionResult::SharedConst GetRoutingTablesTransaction::run() {

    info() << "run\tI am " << mNodeUUID;

    info() << "run\t TrustLinesSize: " << mTrustLinesManager->trustLines().size();

    info() << "run\t" << "RT1 size: " << mTrustLinesManager->rt1().size();
    for (auto const &itRT1 : mTrustLinesManager->rt1()) {
        info() << "run\t\t" << itRT1;
    }
    /*for (auto const &itRT2 : mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsMapDestinationKey()) {
        info() << "run\t\tkey: " << itRT2.first;
        for (auto const &nodeUUID : itRT2.second) {
            info() << "run\t\t\tvalue: " << nodeUUID;
        }
    }*/
    /*for (auto const &itRT3 : mStorageHandler->routingTablesHandler()->routingTable3Level()->routeRecordsMapDestinationKey()) {
        info() << "run\t\tkey: " << itRT3.first;
        for (auto const &nodeUUID : itRT3.second) {
            info() << "run\t\t\tvalue: " << nodeUUID;
        }
    }*/

    sendRoutingTables();

    info() << "message successfully sent to " << mMessage->senderUUID();
    return make_shared<TransactionResult>(TransactionState::exit());
}

void GetRoutingTablesTransaction::sendRoutingTables() {

    info() << "sendRoutingTables\tRT1 size: " << mTrustLinesManager->rt1().size();
    sendMessage<ResultRoutingTable1LevelMessage>(
        mMessage->senderUUID(),
        mNodeUUID,
        mMessage->transactionUUID(),
        mTrustLinesManager->rt1());

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> rt2
        = mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsMapDestinationKey();
    info() << "sendRoutingTables\tRT2 size: " << rt2.size();
    size_t rt2MessageCount = rt2.size() / kCountElementsPerMessage;
    size_t idx = 0;
    auto itRT2 = rt2.begin();
    while (idx < rt2MessageCount) {
        auto itRT2First = itRT2;
        for (size_t jdx = 0; jdx < kCountElementsPerMessage; jdx++) {
            itRT2++;
        }
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> subRT2(itRT2First, itRT2);
        sendMessage<ResultRoutingTable2LevelMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            mMessage->transactionUUID(),
            subRT2);
        idx++;
    }
    if (itRT2 != rt2.end()) {
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> subRT2(itRT2, rt2.end());
        sendMessage<ResultRoutingTable2LevelMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            mMessage->transactionUUID(),
            subRT2);
    }

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> rt3
        = mStorageHandler->routingTablesHandler()->routingTable3Level()->routeRecordsMapDestinationKey();
    info() << "sendRoutingTables\tRT3 size: " << rt3.size();
    size_t rt3MessageCount = rt3.size() / kCountElementsPerMessage;
    idx = 0;
    auto itRT3 = rt3.begin();
    while (idx < rt3MessageCount) {
        auto itRT3First = itRT3;
        for (size_t jdx = 0; jdx < kCountElementsPerMessage; jdx++) {
            itRT3++;
        }
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> subRT3(itRT3First, itRT3);
        sendMessage<ResultRoutingTable3LevelMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            mMessage->transactionUUID(),
            subRT3);
        idx++;
    }
    if (itRT3 != rt3.end()) {
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> subRT3(itRT3, rt3.end());
        sendMessage<ResultRoutingTable3LevelMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            mMessage->transactionUUID(),
            subRT3);
    }
}

const string GetRoutingTablesTransaction::logHeader() const
{
    stringstream s;
    s << "[GetRoutingTablesTA]";

    return s.str();
}
