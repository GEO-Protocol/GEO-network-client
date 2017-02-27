#ifndef GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H
#define GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class FromContractorToFirstLevelRoutingTableAcceptTransaction : public RoutingTablesTransaction  {
public:
    typedef shared_ptr<FromContractorToFirstLevelRoutingTableAcceptTransaction> Shared;

public:
    FromContractorToFirstLevelRoutingTableAcceptTransaction(
        const NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared message);

    FromContractorToFirstLevelRoutingTableAcceptTransaction(
        BytesShared buffer);

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

private:
    FirstLevelRoutingTableIncomingMessage::Shared mFirstLevelMessage;
};

#endif //GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H
