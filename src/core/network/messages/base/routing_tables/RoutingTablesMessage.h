#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <map>
#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTablesMessage : public SenderMessage {
public:
    typedef shared_ptr<RoutingTablesMessage> Shared;
    typedef uint64_t RecordsCount;

public:
    enum PropagationStep {
        WithoutStep = 0,
        FromInitiatorToContractor,
        FromContractorToFirstLevel,
        FromFirstLevelToSecondLevel
    };
    typedef uint8_t SerializedPropagationStep;

public:
    const RoutingTablesMessage::PropagationStep propagationStep() const;

    const map<const NodeUUID, vector<pair<const NodeUUID, const TrustLineDirection>>>& records() const;

protected:
    RoutingTablesMessage();

    RoutingTablesMessage(
        const NodeUUID &senderUUID);

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

private:
    const bool isRoutingTableMessage() const;

protected:
    map<const NodeUUID, vector<pair<const NodeUUID, const TrustLineDirection>>> mRecords;
    RoutingTablesMessage::PropagationStep mPropagationStep = RoutingTablesMessage::PropagationStep::WithoutStep;
};
#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
