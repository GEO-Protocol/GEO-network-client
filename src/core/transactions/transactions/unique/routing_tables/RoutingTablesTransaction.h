#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H

#include "../../base/UniqueTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include <stdint.h>
#include <utility>

class RoutingTablesTransaction : public UniqueTransaction {

public:
    enum RoutingTableLevelStepIdentifier {
        FirstLevelRoutingTableStep = 1,
        SecondLevelRoutingTableStep = 2
    };

public:
    const NodeUUID &contractorUUID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    RoutingTablesTransaction(
        TransactionType type,
        NodeUUID &nodeUUID,
        NodeUUID &contractorUUID,
        TransactionsScheduler *scheduler);

    RoutingTablesTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler);

    void increaseRequestsCounter();

    void resetRequestsCounter();

    void progressConnectionTimeout();

    void restoreStandardConnectionTimeout();

    TransactionResult::SharedConst finishTransaction();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToDataBytes();

protected:
    const uint16_t kMaxRequestsCount = 5;
    const uint16_t kConnectionProgression = 2;
    const uint16_t kStandardConnectionTimeout = 20000;

    bool isUniqueWasChecked = false;

    uint16_t mRequestCounter = 0;
    uint16_t mConnectionTimeout = kStandardConnectionTimeout;

    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H
