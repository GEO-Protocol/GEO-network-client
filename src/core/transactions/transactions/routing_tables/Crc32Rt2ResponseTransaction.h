#ifndef GEO_NETWORK_CLIENT_CRC32RT2RESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CRC32RT2RESPONSETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/routing_tables/CRC32Rt2RequestMessage.hpp"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class Crc32Rt2ResponseTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<Crc32Rt2ResponseTransaction> Shared;

public:
    Crc32Rt2ResponseTransaction(
        NodeUUID &nodeUUID,
        CRC32Rt2RequestMessage &message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger)
    noexcept;

    TransactionResult::SharedConst run();

private:
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;

};
#endif //GEO_NETWORK_CLIENT_CRC32RT2RESPONSETRANSACTION_H
