#include "Crc32Rt2ResponseTransaction.h"

Crc32Rt2ResponseTransaction::Crc32Rt2ResponseTransaction(
    NodeUUID &nodeUUID,
    CRC32Rt2RequestMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::RoutingTables_CRC32Rt2Response,
        nodeUUID,
        logger),
    mStorageHandler(storageHandler),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{

}

TransactionResult::SharedConst Crc32Rt2ResponseTransaction::run() {
    debug() << " CRC32Rt2RequestMessage::" <<  mRequestMessage->CRC32Type() << endl;
    if(mRequestMessage->CRC32Type() == CRC32Rt2RequestMessage::FirstAndSecondLevel){
        auto CRC32Rt2Sum = mTrustLinesManager->crc32SumFirstAndSecondLevelForNeighbor(mRequestMessage->senderUUID);
        auto responseMessage = make_shared<CRC32Rt2ResponseMessage>(
            mNodeUUID,
            mRequestMessage->transactionUUID(),
            CRC32Rt2Sum
        );
        sendMessage(
            mRequestMessage->senderUUID,
            responseMessage);
        return resultDone();
    }
    if(mRequestMessage->CRC32Type() == CRC32Rt2RequestMessage::FirstLevel){
        auto CRC32Rt2Sum = mTrustLinesManager->crc32SumAllFirstLevelNeighbors(mRequestMessage->senderUUID);
        auto responseMessage = make_shared<CRC32Rt2ResponseMessage>(
            mNodeUUID,
            mRequestMessage->transactionUUID(),
            CRC32Rt2Sum
        );
        sendMessage(
            mRequestMessage->senderUUID,
            responseMessage);
        return resultDone();
    }
    return resultDone();
}


const string Crc32Rt2ResponseTransaction::logHeader() const
{
    stringstream s;
    s << "[Crc32Rt2ResponseTransaction: " << currentTransactionUUID() << "]";
    return s.str();
}