#ifndef GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H
#define GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../../network/messages/outgoing/routing_tables/SecondLevelRoutingTableOutgoingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <map>
#include <vector>
#include <stdint.h>

using namespace std;

class FromContractorToFirstLevelRoutingTablesPropagationTransaction: public RoutingTablesTransaction {
public:
    typedef shared_ptr<FromContractorToFirstLevelRoutingTablesPropagationTransaction> Shared;

public:
    FromContractorToFirstLevelRoutingTablesPropagationTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const pair<const NodeUUID, const TrustLineDirection> &relationshipsBetweenInitiatorAndContractor,
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelRoutingTableFromInitiator,
        TrustLinesManager *trustLinesManager);

    FromContractorToFirstLevelRoutingTablesPropagationTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager);

    TransactionResult::SharedConst run();

private:
    pair<bool, TransactionResult::SharedConst> checkContext();

    // Relationships with initiator propagation to B1 level
    TransactionResult::SharedConst propagateRelationshipsBetweenInitiatorAndContractor();

    bool isContractorsCountEnoughForRoutingTablesPropagation();

    TransactionResult::SharedConst trySendLinkBetweenInitiatorAndContractor();

    void sendLinkBetweenInitiatorAndContractor();

    // Propagation second level routing table received from initiator to B1 level
    TransactionResult::SharedConst propagateSecondLevelRoutingTable();

    TransactionResult::SharedConst trySendSecondLevelRoutingTable();

    void sendSecondLevelRoutingTable();

    // State for scheduler
    TransactionResult::SharedConst waitingForRoutingTablePropagationResponse(
        uint32_t connectionTimeout);

    // Reset transaction instance's state
    void prepareToNextStep();

private:
    pair<const NodeUUID, const TrustLineDirection> mLinkWithInitiator;
    SecondLevelRoutingTableIncomingMessage::Shared mSecondLevelRoutingTableFromInitiator;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H
