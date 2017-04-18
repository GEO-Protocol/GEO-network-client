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
        const NodeUUID &relationshipsBetweenInitiatorAndContractor,
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelRoutingTableFromInitiator,
        TrustLinesManager *trustLinesManager);

    FromContractorToFirstLevelRoutingTablesPropagationTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager,
        Logger *logger = nullptr);

    TransactionResult::SharedConst run();

private:
    pair<bool, TransactionResult::SharedConst> checkContext();

    TransactionResult::SharedConst propagateRelationshipsBetweenInitiatorAndContractor();

    bool isContractorsCountEnoughForRoutingTablesPropagation();

    TransactionResult::SharedConst trySendLinkBetweenInitiatorAndContractor();

    void sendLinkBetweenInitiatorAndContractor();

    TransactionResult::SharedConst propagateSecondLevelRoutingTable();

    TransactionResult::SharedConst trySendSecondLevelRoutingTable();

    void sendSecondLevelRoutingTable();

    TransactionResult::SharedConst waitingForRoutingTablePropagationResponse(
        uint32_t connectionTimeout);

    void prepareToNextStep();

private:
    NodeUUID mLinkWithInitiator;
    SecondLevelRoutingTableIncomingMessage::Shared mSecondLevelRoutingTableFromInitiator;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEPROPAGATION_H
