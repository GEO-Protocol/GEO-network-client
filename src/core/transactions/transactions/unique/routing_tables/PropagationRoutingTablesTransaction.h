#ifndef GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H

#include "../../base/UniqueTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../trust_lines/TrustLineUUID.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include "../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"

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
        NodeUUID &contractorUUID,
        TrustLineUUID &trustLineUUID,
        TransactionsScheduler *scheduler);

    PropagationRoutingTablesTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler);

    const NodeUUID &contractorUUID() const;

    const TrustLineUUID &trustLineUUID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    pair<bool, const TransactionUUID> isTransactionToContractorUnique();

    void sendFirstLevelRoutingTableToContractor();

    vector<pair<NodeUUID, TrustLineDirection>> emulateFirstLevelRoutingTable();

private:
    NodeUUID mContractorUUID;
    TrustLineUUID mTrustLineUUID;
};


#endif //GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
