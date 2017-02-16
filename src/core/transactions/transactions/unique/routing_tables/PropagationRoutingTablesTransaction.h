#ifndef GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H

#include "RoutingTablesTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../trust_lines/TrustLineUUID.h"

#include "AcceptRoutingTablesTransaction.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../network/messages/outgoing/routing_tables/SecondLevelRoutingTableOutgoingMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <vector>
#include <stdint.h>

using namespace std;

class PropagationRoutingTablesTransaction : public RoutingTablesTransaction {

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

    TransactionResult::SharedConst run();

private:
    pair<bool, const TransactionUUID> isTransactionToContractorUnique();

    bool isContractorsCountEnoughForFirstLevelRoutingTablePropagationFromInitiatorToContractor();

    pair<bool, TransactionResult::SharedConst> checkRoutingTablePropagationFromInitiatorToContractorContext();

    //First level propagation
    TransactionResult::SharedConst propagateFirstLevelRoutingTableFromInitiatorToContractor();

    TransactionResult::SharedConst trySendFirstLevelRoutingTableFromInitiatorToContractor();

    void sendFirstLevelRoutingTableFromInitiatorToContractor();

    //Second level propagation
    TransactionResult::SharedConst propagateSecondLevelRoutingTable();

    TransactionResult::SharedConst trySendSecondLevelRoutingTableFromInitiatorToContractor();

    void sendSecondLevelRoutingTableFromInitiatorToContractor();

    //State for scheduler
    TransactionResult::SharedConst waitingForRoutingTablePropagationResponseFromContractor();

    //Reset transaction instance's state
    void prepareToNextStep();

private:
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
