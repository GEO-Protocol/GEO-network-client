#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H

#include "../transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include "../../result/MessageResult.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class TrustLinesMessage : public TransactionMessage {
public:
    const TransactionUUID &transactionUUID() const;

    MessageResult::SharedConst customCodeResult(
        const uint16_t code) const;

protected:
    TrustLinesMessage();

    TrustLinesMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID);

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H