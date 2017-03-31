#ifndef GEO_NETWORK_CLIENT_FROMFIRSTLEVELTOSECONDLEVELROUTINGTABLEPROPAGATION_H
#define GEO_NETWORK_CLIENT_FROMFIRSTLEVELTOSECONDLEVELROUTINGTABLEPROPAGATION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <vector>
#include <stdint.h>

class FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction : public RoutingTablesTransaction {

public:
    typedef shared_ptr<FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction> Shared;

public:
    FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
        TrustLinesManager *trustLinesManager);

    FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager);

    TransactionResult::SharedConst run();

private:
    pair<bool, TransactionResult::SharedConst> checkContext();

    TransactionResult::SharedConst propagateRelationshipsBetweenInitiatorAndContractor();

    bool isContractorsCountEnoughForRoutingTablesPropagation();

    TransactionResult::SharedConst trySendLinkBetweenInitiatorAndContractor();

    void sendLinkBetweenInitiatorAndContractor();

    TransactionResult::SharedConst waitingForRoutingTablePropagationResponse();

private:
    FirstLevelRoutingTableIncomingMessage::Shared mLinkWithInitiator;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_FROMFIRSTLEVELTOSECONDLEVELROUTINGTABLEPROPAGATION_H
