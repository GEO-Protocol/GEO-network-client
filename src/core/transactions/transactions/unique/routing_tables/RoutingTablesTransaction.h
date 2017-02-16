#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H

#include "../../base/UniqueTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/NodeUUID.h"

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

    const TrustLineUUID &trustLineUUID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    RoutingTablesTransaction(
        TransactionType type,
        NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const TrustLineUUID &trustLineUUID,
        TransactionsScheduler *scheduler);

    RoutingTablesTransaction(
        BaseTransaction::TransactionType type,
        NodeUUID &nodeUUID,
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
    TrustLineUUID mTrustLineUUID;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLETRANSACTION_H
