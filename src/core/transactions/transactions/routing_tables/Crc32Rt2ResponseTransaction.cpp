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

    auto CRC32Rt2Sum = mTrustLinesManager->crc32SumFirstLevel(mRequestMessage->senderUUID);
    auto responseMessage = make_shared<CRC32Rt2ResponseMessage>(
        mNodeUUID,
        currentTransactionUUID(),
        CRC32Rt2Sum
    );
    sendMessage(
        mRequestMessage->senderUUID,
        responseMessage);
    return resultDone();
}
