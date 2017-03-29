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

    info() << "run\t";

    info() << "run\t TrustLinesSize: " << mTrustLinesManager->trustLines().size();

    info() << "run\t" << "RT1 size: " << mTrustLinesManager->rt1().size();
    info() << "run\t" << "RT2 size: " << mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsMapDestinationKey().size();
    info() << "run\t" << "RT3 size: " << mStorageHandler->routingTablesHandler()->routingTable3Level()->routeRecordsMapDestinationKey().size();
    info() << "run\t" << "transactionUUID\t" << mMessage->transactionUUID();
    sendMessage<ResultRoutingTablesMessage>(
        mMessage->senderUUID(),
        mNodeUUID,
        mMessage->transactionUUID(),
        mTrustLinesManager->rt1(),
        mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsMapDestinationKey(),
        mStorageHandler->routingTablesHandler()->routingTable3Level()->routeRecordsMapDestinationKey());

    info() << "message successfully sent";
    return make_shared<TransactionResult>(TransactionState::exit());
}

const string GetRoutingTablesTransaction::logHeader() const
{
    stringstream s;
    s << "[GetRoutingTablesTA]";

    return s.str();
}
