#ifndef GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESTRANSACTION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../propagate/FromContractorToFirstLevelRoutingTablesPropagationTransaction.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class FromInitiatorToContractorRoutingTablesAcceptTransaction : public RoutingTablesTransaction {
public:
    typedef shared_ptr<FromInitiatorToContractorRoutingTablesAcceptTransaction> Shared;

public:
    FromInitiatorToContractorRoutingTablesAcceptTransaction(
        const NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared message,
        TrustLinesManager *trustLinesManager);

    FromInitiatorToContractorRoutingTablesAcceptTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager);

    FirstLevelRoutingTableIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void saveFirstLevelRoutingTable();

    TransactionResult::SharedConst waitingForSecondLevelRoutingTableState();

    TransactionResult::SharedConst checkIncomingMessageForSecondLevelRoutingTable();

    void saveSecondLevelRoutingTable(
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage);

    void sendResponseToContractor(
        const NodeUUID &contractorUUID,
        const uint16_t code);

    void createFromContractorToFirstLevelRoutingTablesPropagationTransaction(
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage);

private:
    FirstLevelRoutingTableIncomingMessage::Shared mFirstLevelMessage;

    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESTRANSACTION_H
