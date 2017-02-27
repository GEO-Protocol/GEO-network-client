#ifndef GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H
#define GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H

#include "RoutingTablesTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../network/messages/outgoing/routing_tables/SecondLevelRoutingTableOutgoingMessage.h"
#include "../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <vector>
#include <stdint.h>

using namespace std;

class FromContractorToFirstLevelRoutingTablePropagation: public RoutingTablesTransaction {
public:
    typedef shared_ptr<FromContractorToFirstLevelRoutingTablePropagation> Shared;

public:
    FromContractorToFirstLevelRoutingTablePropagation(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        pair<const NodeUUID, const TrustLineDirection> &&relationshipsBetweenInitiatorAndContractor,
        vector<pair<const NodeUUID, const TrustLineDirection>> &&secondLevelRoutingTableFromInitiator,
        TrustLinesManager *trustLinesManager);

    FromContractorToFirstLevelRoutingTablePropagation(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager);

    TransactionResult::SharedConst run();

private:
    pair<bool, TransactionResult::SharedConst> checkContext();

    //Relationships with initiator propagation to B1 level
    TransactionResult::SharedConst propagateRelationshipsBetweenInitiatorAndContractor();

    bool isContractorsCountEnoughForRoutingTablesPropagation();

    TransactionResult::SharedConst trySendLinkBetweenInitiatorAndContractor();

    void sendLinkBetweenInitiatorAndContractor();

    //Propagation second level routing table received from initiator to B1 level
    TransactionResult::SharedConst propagateSecondLevelRoutingTable();

    TransactionResult::SharedConst trySendSecondLevelRoutingTable();

    void sendSecondLevelRoutingTable();

    //State for scheduler
    TransactionResult::SharedConst waitingForRoutingTablePropagationResponse(
        uint32_t connectionTimeout);

    //Reset transaction instance's state
    void prepareToNextStep();

private:
    const uint32_t kReWaitingTimeout = 3000;
    const uint8_t kMaxReWaitingAttemptsCount = 5;

    uint8_t mReWaitingAttemptsCount = 0;

    pair<const NodeUUID, const TrustLineDirection> mLinkWithInitiator;
    vector<pair<const NodeUUID, const TrustLineDirection>> mSecondLevelRoutingTable;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H
