#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class MaxFlowCalculationMessage : public SenderMessage {
public:
    typedef shared_ptr<MaxFlowCalculationMessage> Shared;

public:
    MaxFlowCalculationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const NodeUUID &targetUUID);

    MaxFlowCalculationMessage(
        BytesShared buffer);

    const NodeUUID &targetUUID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const size_t kOffsetToInheritedBytes() const
        noexcept;

protected:
    NodeUUID mTargetUUID;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONMESSAGE_H
