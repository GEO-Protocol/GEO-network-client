#include "Crc32Rt2ResponseTransaction.h"

Crc32Rt2ResponseTransaction::Crc32Rt2ResponseTransaction(
    NodeUUID &nodeUUID,
    CRC32Rt2RequestMessage &message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger):
    BaseTransaction(
        BaseTransaction::RoutingTables_CRC32Rt2Response,
        nodeUUID,
        logger)
{

}
