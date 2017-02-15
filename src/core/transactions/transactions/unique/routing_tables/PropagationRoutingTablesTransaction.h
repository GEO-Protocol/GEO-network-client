#ifndef GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H

#include "../../base/UniqueTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../trust_lines/TrustLineUUID.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../network/messages/outgoing/routing_tables/SecondLevelRoutingTableOutgoingMessage.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <vector>
#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class PropagationRoutingTablesTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<PropagationRoutingTablesTransaction> Shared;

public:
    PropagationRoutingTablesTransaction(
        NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const TrustLineUUID &trustLineUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *trustLinesManager);

    PropagationRoutingTablesTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler,
        TrustLinesManager *trustLinesManager);

    const NodeUUID &contractorUUID() const;

    const TrustLineUUID &trustLineUUID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    pair<bool, const TransactionUUID> isTransactionToContractorUnique();

    TransactionResult::SharedConst checkFirstLevelRoutingTablePropagationContext();

    bool isContractorsCountEnoughForRoutingTablePropagation();

    void sendFirstLevelRoutingTableToContractor();

    TransactionResult::SharedConst waitingForFirstLevelRoutingTablePropagationResponse();

    TransactionResult::SharedConst breakTransaction();

    TransactionResult::SharedConst finishTransaction();

    vector<pair<NodeUUID, TrustLineDirection>> emulateSecondLevelRoutingTable();

private:
    uint16_t mConnectionTimeout = 20000;
    uint16_t kConnectionProgression = 2;
    uint16_t mRequestCounter = 0;

    const uint16_t kMaxRequestsCount = 5;

    NodeUUID mContractorUUID;
    TrustLineUUID mTrustLineUUID;

    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
