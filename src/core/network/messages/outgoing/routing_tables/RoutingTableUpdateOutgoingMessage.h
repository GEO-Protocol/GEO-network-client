#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEOUTGOINGMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTableUpdateOutgoingMessage : public TransactionMessage {
public:
    typedef shared_ptr<RoutingTableUpdateOutgoingMessage> Shared;

public:
    enum UpdatingStep {
        FirstLevelNodes = 1,
        SecondLevelNodes
    };
    typedef uint8_t SerializedUpdatingStep;

public:
    RoutingTableUpdateOutgoingMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeUUID &initiatorUUID,
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction,
        const UpdatingStep updatingStep);

private:
    virtual const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mInitiatorUUID;
    NodeUUID mContractorUUID;
    TrustLineDirection mDirection;
    UpdatingStep mUpdatingStep;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEOUTGOINGMESSAGE_H
