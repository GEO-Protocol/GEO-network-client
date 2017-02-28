#ifndef GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H
#define GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../propagate/FromFirstLevelToSecondLevelRoutingTablePropagationTransaction.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class FromContractorToFirstLevelRoutingTableAcceptTransaction : public RoutingTablesTransaction  {
public:
    typedef shared_ptr<FromContractorToFirstLevelRoutingTableAcceptTransaction> Shared;

public:
    FromContractorToFirstLevelRoutingTableAcceptTransaction(
        const NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
        TrustLinesManager *trustLinesManager);

    FromContractorToFirstLevelRoutingTableAcceptTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager);

    FirstLevelRoutingTableIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void saveLinkBetweenInitiatorAndContractor();

    TransactionResult::SharedConst waitingForSecondLevelRoutingTableState();

    TransactionResult::SharedConst checkIncomingMessageForSecondLevelRoutingTable();

    void saveSecondLevelRoutingTable(
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage);

    void sendResponseToContractor(
        const NodeUUID &contractorUUID,
        const uint16_t code);

    void createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction();

private:
    FirstLevelRoutingTableIncomingMessage::Shared mLinkBetweenInitiatorAndContractor;

    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H
