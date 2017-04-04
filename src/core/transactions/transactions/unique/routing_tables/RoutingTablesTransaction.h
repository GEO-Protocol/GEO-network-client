#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H

#include "../../base/BaseTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"


#include <stdint.h>
#include <utility>
#include <vector>

using namespace std;

class RoutingTablesTransaction : public BaseTransaction {

public:
    enum RoutingTableLevelStepIdentifier {
        FirstLevelRoutingTableStep = 1,
        SecondLevelRoutingTableStep = 2
    };

public:
    const NodeUUID &contractorUUID() const;

    const vector<NodeUUID> &contractorsUUIDs() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    RoutingTablesTransaction(
        const TransactionType type,
        BytesShared buffer,
        Logger *logger);

    RoutingTablesTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID);

    RoutingTablesTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        Logger *logger);

    void increaseRequestsCounter();

    void resetRequestsCounter();

    void progressConnectionTimeout();

    void restoreStandardConnectionTimeout();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToDataBytes();

protected:
    const uint16_t kResponseCodeSuccess = 200;
    const uint16_t kMaxRequestsCount = 5;
    const uint8_t kConnectionProgression = 2;
    const uint32_t kStandardConnectionTimeout = 20000;

    uint16_t mRequestCounter = 0;
    uint32_t mConnectionTimeout = kStandardConnectionTimeout;

    NodeUUID mContractorUUID;
    vector<NodeUUID> mContractorsUUIDs;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H
