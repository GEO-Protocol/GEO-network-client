#ifndef GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

class OperationStateMessage: public TransactionMessage {
public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

    typedef byte SerializedOperationState;
    typedef shared_ptr<OperationStateMessage> Shared;
    typedef shared_ptr<const OperationStateMessage> ConstShared;

public:
    OperationStateMessage(
        const OperationState state);

    OperationStateMessage(
        BytesShared buffer);

    const TransactionUUID &transactionUUID() const;

    const OperationState state() const;

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    mutable OperationState mState;
};
#endif //GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H
