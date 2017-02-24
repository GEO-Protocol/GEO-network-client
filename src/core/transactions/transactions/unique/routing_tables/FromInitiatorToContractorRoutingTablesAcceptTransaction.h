#ifndef GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESTRANSACTION_H

#include "RoutingTablesTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../../../network/messages/response/RoutingTablesResponse.h"

#include "FromInitiatorToContractorRoutingTablePropagationTransaction.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class FromInitiatorToContractorRoutingTablesAcceptTransaction : public RoutingTablesTransaction {

public:
    typedef shared_ptr<FromInitiatorToContractorRoutingTablesAcceptTransaction> Shared;

public:
    FromInitiatorToContractorRoutingTablesAcceptTransaction(
        const NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared message);

    FromInitiatorToContractorRoutingTablesAcceptTransaction(
        BytesShared buffer);

    FirstLevelRoutingTableIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void saveFirstLevelRoutingTable();

    TransactionResult::SharedConst waitingForSecondLevelRoutingTableState();

    TransactionResult::SharedConst checkIncomingMessageForSecondLevelRoutingTable();

    void saveSecondLevelRoutingTable(
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage
    );

    void sendResponseToContractor(
        NodeUUID &contractorUUID,
        uint16_t code);

private:
    FirstLevelRoutingTableIncomingMessage::Shared mFirstLevelMessage;

};


#endif //GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESTRANSACTION_H
