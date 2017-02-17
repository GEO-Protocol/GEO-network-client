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

#include "PropagationRoutingTablesTransaction.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class AcceptRoutingTablesTransaction : public RoutingTablesTransaction {

public:
    typedef shared_ptr<AcceptRoutingTablesTransaction> Shared;

public:
    AcceptRoutingTablesTransaction(
        NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared message,
        TransactionsScheduler *scheduler);

    AcceptRoutingTablesTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler);

    FirstLevelRoutingTableIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    pair<bool, const TransactionUUID> isTransactionToContractorUnique();

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
